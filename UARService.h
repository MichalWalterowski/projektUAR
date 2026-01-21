#ifndef UARSERVICE_H
#define UARSERVICE_H

#include <QObject> // Niezbędne dla Timera i Sygnałów
#include <QTimer>
#include "UAR.h"
#include <QString>

// Struktura danych (bez zmian)
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

class UARService : public QObject { // <--- Dziedziczenie po QObject
    Q_OBJECT

public:
    explicit UARService(QObject *parent = nullptr);

    // --- Metody sterujące symulacją (Nowość) ---
    void startSimulation(int intervalMs);
    void stopSimulation();
    void resetSimulation();
    bool isRunning() const;
    void setInterval(int intervalMs); // Zmiana prędkości w locie

    // --- Konfiguracja (bez zmian) ---
    void configurePID(double k, double Ti, double Td, int trybIdx);
    void configureGenerator(int trybIdx, double okres, double amplituda, double skladowaStala, double wypelnienie, int interwal_ms);

    // Pamiętamy o bool limityOn z poprzedniego kroku
    void configureARX(const QString &aStr, const QString &bStr, int k,
                      double uMin, double uMax, double yMin, double yMax, double szumStd, bool limityOn);

signals:
    // SYGNAŁ: Wysyłany do MainWindow, gdy są nowe dane do narysowania
    void simulationUpdated(SimulationData data);

private slots:
    // Slot wewnętrzny, który "tyka" razem z timerem
    void performStep();

private:
    ProstyUAR m_uar;
    QTimer *m_timer; // Timer jest teraz tutaj (własność serwisu)
    int m_step = 0;  // Licznik kroków też jest tutaj
};

#endif // UARSERVICE_H
