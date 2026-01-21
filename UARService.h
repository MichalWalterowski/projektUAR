#ifndef UARSERVICE_H
#define UARSERVICE_H

#include "UAR.h"
#include <QString>

// Struktura do przesyłania kompletu danych do wykresów
struct SimulationData {
    double x;        // Oś czasu (nr kroku)
    double y;        // Wartość regulowana
    double setpoint; // Wartość zadana
    double error;    // Uchyb
    double u;        // Sterowanie
    double uP;
    double uI;
    double uD;       // Składowe PID
};

class UARService {
private:
    ProstyUAR m_uar;

public:
    UARService();

    // Konfiguracja PID
    void configurePID(double k, double Ti, double Td, int trybIdx);

    // Konfiguracja Generatora
    void configureGenerator(int trybIdx, double okres, double amplituda, double skladowaStala, double wypelnienie, int interwal_ms);

    // Konfiguracja ARX (Wektory jako stringi, plus limity i szum)
    void configureARX(const QString &aStr, const QString &bStr, int k,
                      double uMin, double uMax, double yMin, double yMax, double szumStd, bool limityOn);

    // Krok symulacji
    SimulationData nextStep(int currentStep);

    // Reset
    void reset();

};

#endif
