#ifndef UARSERVICE_H
#define UARSERVICE_H

#include "UAR.h"
#include <QString>

// Struktura przechowująca dane pojedynczego kroku symulacji dla wykresów
struct SimulationData {
    double x, y, setpoint, error, u, uP, uI, uD;
};

class UARService {
public:
    UARService();

    // Konfiguracja regulatora z nowym parametrem trybu (stała przed/pod całką)
    void configurePID(double k, double Ti, double Td, int trybIdx);

    // Konfiguracja generatora z 5 parametrami (w tym Okres)
    void configureGenerator(int tryb, double okres, double amplituda, double skladowaStala, double wypelnienie);

    // Konfiguracja modelu ARX na podstawie ciągów znaków z UI
    void configureARX(const QString &aStr, const QString &bStr, int k);

    // Obliczenie kolejnego kroku symulacji
    SimulationData nextStep(int currentStep);

    // Resetowanie stanu układu
    void reset();

private:
    ProstyUAR m_uar;
};

#endif
