#include "UAR.h"
#include <cmath>
#include <algorithm>

// Definicja PI dla uniknięcia błędów M_PI
#ifndef PI
#define PI 3.14159265358979323846
#endif

// --- IMPLEMENTACJA GENERATORA ---
GeneratorWartosci::GeneratorWartosci()
    : m_tryb(Prostokatny), m_okres(20.0), m_amplituda(1.0), m_skladowaStala(0.0), m_wypelnienie(0.5), m_curr(0.0) {}

void GeneratorWartosci::setParams(TrybGen tryb, double okres, double amplituda, double skladowaStala, double wypelnienie) {
    m_tryb = tryb;
    m_okres = (okres > 0.1) ? okres : 1.0;
    m_amplituda = amplituda;
    m_skladowaStala = skladowaStala;
    // Zabezpieczenie wypełnienia 0-1
    m_wypelnienie = (wypelnienie < 0.0) ? 0.0 : (wypelnienie > 1.0 ? 1.0 : wypelnienie);
}

double GeneratorWartosci::generuj(int krok) {
    double faza = fmod(static_cast<double>(krok), m_okres) / m_okres;

    if (m_tryb == Prostokatny) {
        m_curr = (faza < m_wypelnienie) ? m_amplituda : 0.0;
    } else {
        m_curr = m_amplituda * sin(2.0 * PI * faza);
    }

    m_curr += m_skladowaStala;
    return m_curr;
}

// --- IMPLEMENTACJA REGULATORA PID ---
RegulatorPID::RegulatorPID()
    : m_k(1.0), m_ti(10.0), m_td(0.0), m_tryb(PrzedCalka), m_suma_e(0), m_ost_e(0), m_up(0), m_ui(0), m_ud(0) {}

void RegulatorPID::setParams(double k, double ti, double td, TrybPID tryb) {
    m_k = k; m_ti = ti; m_td = td; m_tryb = tryb;
}

double RegulatorPID::oblicz(double e) {
    m_suma_e += e;
    double roznica = e - m_ost_e;
    m_ost_e = e;

    if (m_tryb == PrzedCalka) {
        // Postać idealna: K * (e + sum_e/Ti + Td*diff)
        m_up = m_k * e;
        m_ui = (m_ti > 0) ? (m_k / m_ti) * m_suma_e : 0;
        m_ud = m_k * m_td * roznica;
    } else {
        // Postać równoległa: K*e + sum_e/Ti + Td*diff
        m_up = m_k * e;
        m_ui = (m_ti > 0) ? (1.0 / m_ti) * m_suma_e : 0;
        m_ud = m_td * roznica;
    }

    return m_up + m_ui + m_ud;
}

void RegulatorPID::reset() { m_suma_e = 0; m_ost_e = 0; m_up = 0; m_ui = 0; m_ud = 0; }

// --- IMPLEMENTACJA MODELU ARX ---
ModelARX::ModelARX() : m_k(1) {
    m_a = {0.0}; m_b = {0.5};
    m_u.assign(100, 0); m_y.assign(100, 0);
}

void ModelARX::setParams(const std::vector<double> &a, const std::vector<double> &b, int k) {
    m_a = a; m_b = b; m_k = k;
    reset();
}

double ModelARX::symuluj(double u) {
    m_u.insert(m_u.begin(), u);
    double y = 0;

    // Suma b_i * u(k-d-i)
    for (size_t i = 0; i < m_b.size(); ++i) {
        int idx = i + m_k;
        if (idx < (int)m_u.size()) y += m_b[i] * m_u[idx];
    }

    // Suma a_i * y(k-i)
    for (size_t i = 0; i < m_a.size(); ++i) {
        if (i < m_y.size()) y -= m_a[i] * m_y[i];
    }

    m_y.insert(m_y.begin(), y);

    if (m_u.size() > 100) m_u.pop_back();
    if (m_y.size() > 100) m_y.pop_back();

    return y;
}

void ModelARX::reset() {
    std::fill(m_u.begin(), m_u.end(), 0.0);
    std::fill(m_y.begin(), m_y.end(), 0.0);
}

// --- IMPLEMENTACJA UAR ---
ProstyUAR::ProstyUAR() : m_e(0), m_u(0), m_y(0), m_krok_licznik(0) {}

double ProstyUAR::symuluj() {
    double w = m_gen.generuj(m_krok_licznik);
    m_e = w - m_y;
    m_u = m_pid.oblicz(m_e);
    m_y = m_arx.symuluj(m_u);
    m_krok_licznik++;
    return m_y;
}

void ProstyUAR::reset() {
    m_gen.reset();
    m_pid.reset();
    m_arx.reset();
    m_e = 0; m_u = 0; m_y = 0; m_krok_licznik = 0;
}
