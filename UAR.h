#ifndef UAR_H
#define UAR_H

#include <vector>

enum TrybGen { Pros = 0, Sin = 1 };

class ModelARX {
    std::vector<double> m_A, m_B;
    unsigned int m_k;
    std::vector<double> m_hist_u, m_hist_y;
public:
    ModelARX(std::vector<double> a = {0.0}, std::vector<double> b = {1.0}, unsigned int k = 1);
    void setParams(std::vector<double> a, std::vector<double> b, unsigned int k);
    double symuluj(double u);
    void reset();
};

class RegulatorPID {
    double m_k = 1.0, m_Ti = 10.0, m_Td = 0.0;
    double m_prev_e = 0.0, m_suma_e = 0.0;
    double m_uP = 0.0, m_uI = 0.0, m_uD = 0.0;
public:
    void setParams(double k, double Ti, double Td);
    double symuluj(double e);
    void reset();
    double getUP() const { return m_uP; }
    double getUI() const { return m_uI; }
    double getUD() const { return m_uD; }
};

class GeneratorWartosci {
    TrybGen m_tryb = Pros;
    int m_TT = 10; double m_TRZ = 1.0, m_A = 1.0, m_S = 0.0, m_p = 0.5;
    int m_i = 0; double m_curr = 0.0;
public:
    void setParams(TrybGen t, int tt, double trz, double a, double s, double p);
    double generuj();
    void reset();
    double getVal() const { return m_curr; }
};

class ProstyUAR {
    ModelARX m_ARX;
    RegulatorPID m_PID;
    GeneratorWartosci m_Gen;
    double m_y = 0.0, m_e = 0.0, m_u = 0.0;
public:
    ProstyUAR();
    double symuluj();
    void reset();
    ModelARX& getARX() { return m_ARX; }
    RegulatorPID& getPID() { return m_PID; }
    GeneratorWartosci& getGen() { return m_Gen; }
    double getY() const { return m_y; }
    double getE() const { return m_e; }
    double getU() const { return m_u; }
};

#endif
