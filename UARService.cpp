#include "UARService.h"
#include <QStringList> // Niezbędne do poprawnej kompilacji metod split()

UARService::UARService() {}

void UARService::configurePID(double k, double Ti, double Td, int trybIdx) {
    // Mapowanie indeksu z QComboBox na typ wyliczeniowy TrybPID
    TrybPID t = static_cast<TrybPID>(trybIdx);
    m_uar.getPID().setParams(k, Ti, Td, t);
}

void UARService::configureGenerator(int tryb, double okres, double amplituda, double skladowaStala, double wypelnienie) {
    // Mapowanie indeksu z QComboBox na typ wyliczeniowy TrybGen
    TrybGen t = static_cast<TrybGen>(tryb);
    m_uar.getGen().setParams(t, okres, amplituda, skladowaStala, wypelnienie);
}

void UARService::configureARX(const QString &aStr, const QString &bStr, int k) {
    std::vector<double> va, vb;

    // Rozbicie ciągów znaków (np. "1, -0.5") na wektory współczynników
    QStringList aList = aStr.split(",", Qt::SkipEmptyParts);
    QStringList bList = bStr.split(",", Qt::SkipEmptyParts);

    for(const QString &s : aList) va.push_back(s.trimmed().toDouble());
    for(const QString &s : bList) vb.push_back(s.trimmed().toDouble());

    // Domyślne wartości w razie pustych pól, aby uniknąć błędów symulacji
    if(va.empty()) va = {0.0};
    if(vb.empty()) vb = {0.5};

    m_uar.getARX().setParams(va, vb, k);
}

SimulationData UARService::nextStep(int currentStep) {
    SimulationData data;

    // Wykonanie kroku obliczeniowego w modelu matematycznym
    data.y = m_uar.symuluj();

    // Pakowanie wyników do struktury SimulationData
    data.x = static_cast<double>(currentStep);
    data.setpoint = m_uar.getGen().getVal();
    data.error = m_uar.getE();
    data.u = m_uar.getU();

    // Pobranie składowych PID do dedykowanego wykresu
    data.uP = m_uar.getPID().getUP();
    data.uI = m_uar.getPID().getUI();
    data.uD = m_uar.getPID().getUD();

    return data;
}

void UARService::reset() {
    m_uar.reset();
}
