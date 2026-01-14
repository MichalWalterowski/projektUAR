#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include "UARService.h" // Nowa warstwa usług
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
    void simulateStep();
    void startSimulation();
    void stopSimulation();
    void resetSimulation();
    void saveConfig();
    void loadConfig();
    void openARXDialog();

private:
    Ui::MainWindow *ui;
    QTimer *simTimer;
    int m_step = 0;

    // Warstwa usług zarządzająca logiką
    UARService m_service;

    // Domyślne parametry ARX zapobiegające niestabilności
    QString m_curA = "0.0";
    QString m_curB = "0.5";
    int m_curK = 1;

    // Wskaźniki do wykresów i serii danych
    QCustomPlot *plotY, *plotError, *plotU, *plotUComponents;
    QCPGraph *graphY_zadana, *graphY_regulowana, *graphError, *graphU, *graphU_P, *graphU_I, *graphU_D;

    void setupPlots();
    void setupConnections();
    void updateUARFromUI();
    void updateARXFromStrings();
};

#endif // MAINWINDOW_H
