#ifndef UAR_H
#define UAR_H

#include <vector>

// Typy wyliczeniowe dla konfiguracji
enum TrybGen { Prostokatny = 0, Sinusoidalny = 1 };
enum TrybPID { PrzedCalka = 0, PodCalka = 1 };

// Klasa Generatora sygnału zadanego
class GeneratorWartosci {
public:
    GeneratorWartosci();
    void setParams(TrybGen tryb, double okres, double amplituda, double skladowaStala, double wypelnienie);
    double generuj(int krok);
    double getVal() const { return m_curr; }
    void reset() { m_curr = 0; }

private:
    TrybGen m_tryb;
    double m_okres;
    double m_amplituda;
    double m_skladowaStala;
    double m_wypelnienie;
    double m_curr;
};

// Klasa Regulatora PID z wyborem algorytmu
class RegulatorPID {
public:
    RegulatorPID();
    void setParams(double k, double ti, double td, TrybPID tryb);
    double oblicz(double e);
    void reset();

    // Gettery dla składowych (do wykresów)
    double getUP() const { return m_up; }
    double getUI() const { return m_ui; }
    double getUD() const { return m_ud; }

private:
    double m_k, m_ti, m_td;
    TrybPID m_tryb;
    double m_suma_e, m_ost_e;
    double m_up, m_ui, m_ud;
};

// Klasa Modelu ARX
class ModelARX {
public:
    ModelARX();
    void setParams(const std::vector<double> &a, const std::vector<double> &b, int k);
    double symuluj(double u);
    void reset();

private:
    std::vector<double> m_a, m_b;
    int m_k;
    std::vector<double> m_u, m_y;
};

// Klasa spinająca cały układ automatyki
class ProstyUAR {
public:
    ProstyUAR();
    double symuluj();
    void reset();

    GeneratorWartosci& getGen() { return m_gen; }
    RegulatorPID& getPID() { return m_pid; }
    ModelARX& getARX() { return m_arx; }

    double getE() const { return m_e; }
    double getU() const { return m_u; }
    double getY() const { return m_y; }

private:
    GeneratorWartosci m_gen;
    RegulatorPID m_pid;
    ModelARX m_arx;

    double m_e, m_u, m_y;
    int m_krok_licznik;
};

#endif
