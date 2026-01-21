#include "UARService.h"
#include <QStringList>

UARService::UARService() {
    // Konstruktor domyślny
}

void UARService::configurePID(double k, double Ti, double Td, int trybIdx) {
    // Mapowanie indeksu z ComboBox (int) na typ wyliczeniowy (Enum) z UAR.h
    // Zakładamy kolejność w GUI:
    // Indeks 0: "W sumie (Wew)" -> LiczCalk::Wew (Zalecane)
        // Indeks 1: "Przed sumą (Zew)" -> LiczCalk::Zew
        LiczCalk tryb = (trybIdx == 0) ? LiczCalk::Wew : LiczCalk::Zew;

    // Ustawienie parametrów w regulatorze (przez referencję z ProstyUAR)
    m_uar.getPID().setNastawy(k, Ti, Td, tryb);
}

void UARService::configureGenerator(int trybIdx, double okres, double amplituda, double skladowaStala, double wypelnienie, int interwal_ms) {
    // Mapowanie indeksu z ComboBox na Enum
    // Indeks 0: Sinus, Indeks 1: Prostokąt
    TrybGen tryb = (trybIdx == 0) ? TrybGen::Sin : TrybGen::Pros;

    // Przekazanie parametrów do generatora.
    // Ważne: Przekazujemy 'interwal_ms', aby generator mógł przeliczyć czas rzeczywisty na próbki [cite: 377]
        m_uar.getGen().setParams(tryb, okres, amplituda, skladowaStala, wypelnienie, interwal_ms);
}

void UARService::configureARX(const QString &aStr, const QString &bStr, int k,
                              double uMin, double uMax, double yMin, double yMax, double szumStd, bool limityOn)
{
    std::vector<double> va, vb;

    // 1. Parsowanie stringów z GUI (np. "1.0, -0.5, 0.2") na wektory liczb
    QStringList aList = aStr.split(",", Qt::SkipEmptyParts);
    QStringList bList = bStr.split(",", Qt::SkipEmptyParts);

    for(const QString &s : aList) va.push_back(s.trimmed().toDouble());
    for(const QString &s : bList) vb.push_back(s.trimmed().toDouble());

    // Zabezpieczenie: Jeśli pola są puste, ustawiamy wartości domyślne,
    // aby symulacja nie "wybuchła" (dzielenie przez zero, puste bufory itp.)
    if(va.empty()) va = {0.0};
    if(vb.empty()) vb = {1.0};

    // 2. Konfiguracja wielomianów i opóźnienia
    m_uar.getARX().setParams(va, vb, k);

    // 3. Konfiguracja ograniczeń (Nasycenia) - wymagane przez PDF [cite: 30-33]
        m_uar.getARX().setLimity(uMin, uMax, yMin, yMax, limityOn);

    // 4. Konfiguracja szumu (Zakłócenia) - wymagane przez PDF [cite: 30]
        m_uar.getARX().setSzum(szumStd);
}

SimulationData UARService::nextStep(int currentStep) {
    SimulationData data;

    // 1. Wykonanie kroku symulacji (logika biznesowa)
    // Metoda ta wywołuje kolejno: Generator -> PID -> ModelARX
    double y = m_uar.symuluj();

    // 2. Pakowanie danych do struktury (transfer do GUI)
    data.x = static_cast<double>(currentStep);
    data.y = y;
    data.setpoint = m_uar.getGen().getVal(); // Aktualna wartość zadana
    data.error = m_uar.getE();               // Aktualny uchyb
    data.u = m_uar.getU();                   // Aktualne sterowanie

    // 3. Pobranie składowych PID (do wizualizacji pracy regulatora)
    data.uP = m_uar.getPID().getUP();
    data.uI = m_uar.getPID().getUI();
    data.uD = m_uar.getPID().getUD();

    return data;
}

void UARService::reset() {
    // Pełny reset pamięci symulacji
    m_uar.reset();
}
