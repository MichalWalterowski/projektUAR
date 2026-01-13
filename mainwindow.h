#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include "UAR.h"
#include "qcustomplot.h"
#include "dialogarx.h"

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
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
    ProstyUAR m_uar;

    QString m_curA = "0.1, 0.2";
    QString m_curB = "1.0, 0.5";
    int m_curK = 1;

    QCustomPlot *plotY, *plotError, *plotU, *plotUComponents;
    QCPGraph *graphY_zadana, *graphY_regulowana, *graphError, *graphU, *graphU_P, *graphU_I, *graphU_D;

    void setupPlots();
    void setupConnections();
    void updateUARFromUI();
    void updateARXFromStrings(); // Inicjalizacja ARX
};

#endif
