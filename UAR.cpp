#include "UAR.h"
#include <cmath>
#include <algorithm>

ModelARX::ModelARX(std::vector<double> a, std::vector<double> b, unsigned int k) { setParams(a, b, k); }

void ModelARX::setParams(std::vector<double> a, std::vector<double> b, unsigned int k) {
    m_A = a; m_B = b; m_k = k;
    m_hist_u.assign(m_k + m_B.size() + 1, 0.0);
    m_hist_y.assign(m_A.size() + 1, 0.0);
}

void ModelARX::reset() {
    std::fill(m_hist_u.begin(), m_hist_u.end(), 0.0);
    std::fill(m_hist_y.begin(), m_hist_y.end(), 0.0);
}

double ModelARX::symuluj(double u) {
    for (int i = m_hist_u.size() - 1; i > 0; --i) m_hist_u[i] = m_hist_u[i - 1];
    m_hist_u[0] = u;

    double sB = 0;
    for (size_t j = 0; j < m_B.size(); ++j) {
        if (m_k + j < m_hist_u.size()) sB += m_B[j] * m_hist_u[m_k + j];
    }

    double sA = 0;
    for (size_t i = 0; i < m_A.size(); ++i) {
        if (i < m_hist_y.size()) sA += m_A[i] * m_hist_y[i];
    }

    double y = sB - sA;
    for (int i = m_hist_y.size() - 1; i > 0; --i) m_hist_y[i] = m_hist_y[i - 1];
    if(!m_hist_y.empty()) m_hist_y[0] = y;
    return y;
}

void RegulatorPID::setParams(double k, double Ti, double Td) { m_k = k; m_Ti = Ti; m_Td = Td; }
double RegulatorPID::symuluj(double e) {
    m_uP = m_k * e;
    if (m_Ti != 0) m_suma_e += e / m_Ti;
    m_uI = m_suma_e;
    m_uD = m_Td * (e - m_prev_e);
    m_prev_e = e;
    return m_uP + m_uI + m_uD;
}
void RegulatorPID::reset() { m_prev_e = 0; m_suma_e = 0; }

void GeneratorWartosci::setParams(TrybGen t, int tt, double trz, double a, double s, double p) {
    m_tryb = t; m_TT = tt; m_TRZ = trz; m_A = a; m_S = s; m_p = p;
}
double GeneratorWartosci::generuj() {
    int T = (m_TRZ * m_TT); if (T <= 0) T = 1;
    if (m_tryb == Pros) m_curr = ((m_i % T) < (m_p * T)) ? (m_A + m_S) : m_S;
    else m_curr = m_A * std::sin((double(m_i % T) / T) * 2 * 3.14159) + m_S;
    m_i++; return m_curr;
}
void GeneratorWartosci::reset() { m_i = 0; }

ProstyUAR::ProstyUAR() : m_ARX({0.1}, {1.0}, 1) {}
double ProstyUAR::symuluj() {
    m_e = m_Gen.generuj() - m_y;
    m_u = m_PID.symuluj(m_e);
    m_y = m_ARX.symuluj(m_u);
    return m_y;
}
void ProstyUAR::reset() { m_ARX.reset(); m_PID.reset(); m_Gen.reset(); m_y = m_e = m_u = 0; }
