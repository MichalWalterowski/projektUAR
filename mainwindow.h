#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>

#include "UARService.h"
#include "qcustomplot.h"
#include "dialogarx.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // --- Sloty sterujące (przyciski) ---
    void startSimulation();
    void stopSimulation();
    void resetSimulation();
    void simulateStep();       // Główna pętla timera

    // --- Sloty konfiguracji ---
    void openARXDialog();      // Otwiera DialogARX
    void updateParameters();   // Pobiera dane z GUI głównego (PID/Generator)

    // --- Sloty zapisu/odczytu ---
    void saveConfig();
    void loadConfig();

private:
    Ui::MainWindow *ui;
    QTimer *simTimer;
    int m_step = 0; // Licznik kroków (oś czasu)

    // Logika biznesowa (Service)
    UARService m_service;

    // --- PARAMETRY ARX (Cache) ---
    // Musimy je pamiętać w MainWindow, aby przekazywać je do Dialogu i Serwisu
    QString m_curA = "0.0";
    QString m_curB = "0.5";
    int m_curK = 1;

    // Parametry nasycenia i szumu [cite: 30-32]
    double m_curMinU = -10.0;
    double m_curMaxU = 10.0;
    double m_curMinY = -10.0;
    double m_curMaxY = 10.0;
    double m_curNoise = 0.0;
    bool m_curLimitsOn = true;

    // --- WYKRESY (Widgety - kontenery) ---
    // Wskaźniki do widgetów na UI (przypiszemy je w konstruktorze)
    QCustomPlot *m_plotY;       // Wykres 1: Wartość zadana i regulowana
    QCustomPlot *m_plotError;   // Wykres 2: Uchyb
    QCustomPlot *m_plotU;       // Wykres 3: Sterowanie
    QCustomPlot *m_plotUComp;   // Wykres 4: Składowe PID

    // --- SERIE DANYCH (Linie na wykresach) ---
    QCPGraph *m_graphY_zadana;
    QCPGraph *m_graphY_regulowana;

    QCPGraph *m_graphError;

    QCPGraph *m_graphU;

    QCPGraph *m_graphU_P;
    QCPGraph *m_graphU_I;
    QCPGraph *m_graphU_D;

    // --- METODY POMOCNICZE ---
    void setupPlots();             // Konfiguracja wyglądu (osie, kolory)
    void setupConnections();       // Podpięcie sygnałów
    void pushARXParamsToService(); // Wysłanie parametrów ARX do backendu
};

#endif // MAINWINDOW_H
