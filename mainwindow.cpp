#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    simTimer = new QTimer(this);

    plotY = new QCustomPlot();
    plotError = new QCustomPlot();
    plotU = new QCustomPlot();
    plotUComponents = new QCustomPlot();

    ui->gridLayoutCharts->addWidget(plotY, 0, 0);
    ui->gridLayoutCharts->addWidget(plotError, 0, 1);
    ui->gridLayoutCharts->addWidget(plotU, 1, 0);
    ui->gridLayoutCharts->addWidget(plotUComponents, 1, 1);

    setupPlots();
    setupConnections();

    // Wartości startowe dla ARX, jeśli plik nie zostanie wczytany
    m_curA = "1.0, -0.8";
    m_curB = "0.2";
    m_curK = 1;
    updateARXFromStrings();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupPlots() {
    // Konfiguracja wykresów (podpisy osi i legendy)
    plotY->plotLayout()->insertRow(0);
    plotY->plotLayout()->addElement(0, 0, new QCPTextElement(plotY, "Regulacja: wyjście - zadana", QFont("sans", 10, QFont::Bold)));
    graphY_zadana = plotY->addGraph(); graphY_zadana->setPen(QPen(Qt::red, 2)); graphY_zadana->setName("Zadana (w)");
    graphY_regulowana = plotY->addGraph(); graphY_regulowana->setPen(QPen(Qt::blue, 2)); graphY_regulowana->setName("Wyjście (y)");
    plotY->xAxis->setLabel("Czas [s]"); plotY->yAxis->setLabel("Amplituda sygnałów");
    plotY->legend->setVisible(true);

    plotError->plotLayout()->insertRow(0);
    plotError->plotLayout()->addElement(0, 0, new QCPTextElement(plotError, "Błąd regulacji e(t)", QFont("sans", 10, QFont::Bold)));
    graphError = plotError->addGraph(); graphError->setPen(QPen(Qt::darkCyan, 2));
    plotError->xAxis->setLabel("Czas [s]"); plotError->yAxis->setLabel("Wartość błędu");

    plotU->plotLayout()->insertRow(0);
    plotU->plotLayout()->addElement(0, 0, new QCPTextElement(plotU, "Sygnał sterujący u(t)", QFont("sans", 10, QFont::Bold)));
    graphU = plotU->addGraph(); graphU->setPen(QPen(Qt::black, 2));
    plotU->xAxis->setLabel("Czas [s]"); plotU->yAxis->setLabel("Wartość sterowania");

    plotUComponents->plotLayout()->insertRow(0);
    plotUComponents->plotLayout()->addElement(0, 0, new QCPTextElement(plotUComponents, "Składowe regulatora PID", QFont("sans", 10, QFont::Bold)));
    graphU_P = plotUComponents->addGraph(); graphU_P->setPen(QPen(Qt::red)); graphU_P->setName("P");
    graphU_I = plotUComponents->addGraph(); graphU_I->setPen(QPen(Qt::green)); graphU_I->setName("I");
    graphU_D = plotUComponents->addGraph(); graphU_D->setPen(QPen(Qt::blue)); graphU_D->setName("D");
    plotUComponents->xAxis->setLabel("Czas [s]"); plotUComponents->yAxis->setLabel("Wartości składowych");
    plotUComponents->legend->setVisible(true);

    for(QCustomPlot* p : {plotY, plotError, plotU, plotUComponents}) {
        p->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        p->xAxis->setRange(0, 100);
        p->yAxis->setRange(-2, 2);
    }
}

void MainWindow::setupConnections() {
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::startSimulation);
    connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::stopSimulation);
    connect(ui->btnReset, &QPushButton::clicked, this, &MainWindow::resetSimulation);
    connect(ui->btnOpenARX, &QPushButton::clicked, this, &MainWindow::openARXDialog);
    connect(ui->btnSaveConfig, &QPushButton::clicked, this, &MainWindow::saveConfig);
    connect(ui->btnLoadConfig, &QPushButton::clicked, this, &MainWindow::loadConfig);
    connect(simTimer, &QTimer::timeout, this, &MainWindow::simulateStep);
}

void MainWindow::updateUARFromUI() {
    m_service.configurePID(ui->spinK->value(), ui->spinTi->value(), ui->spinTd->value(), ui->comboTrybPID->currentIndex());
    m_service.configureGenerator(ui->comboTryb->currentIndex(), ui->spinOkres->value(), ui->spinAmplituda->value(), ui->spinSkladowaStala->value(), ui->spinWypelnienie->value());
}

void MainWindow::simulateStep() {
    updateUARFromUI();
    SimulationData d = m_service.nextStep(m_step);
    graphY_zadana->addData(d.x, d.setpoint); graphY_regulowana->addData(d.x, d.y);
    graphError->addData(d.x, d.error); graphU->addData(d.x, d.u);
    graphU_P->addData(d.x, d.uP); graphU_I->addData(d.x, d.uI); graphU_D->addData(d.x, d.uD);

    for (QCustomPlot* p : {plotY, plotError, plotU, plotUComponents}) {
        p->xAxis->setRange(m_step > 100 ? m_step - 100 : 0, m_step + 1);
        p->rescaleAxes(true); p->replot();
    }
    m_step++;
}

void MainWindow::startSimulation() { simTimer->start(ui->spinInterval->value()); }
void MainWindow::stopSimulation() { simTimer->stop(); }

void MainWindow::resetSimulation() {
    simTimer->stop(); m_service.reset(); m_step = 0;
    for(auto g : {graphY_zadana, graphY_regulowana, graphError, graphU, graphU_P, graphU_I, graphU_D}) g->data()->clear();
    for(auto p : {plotY, plotError, plotU, plotUComponents}) { p->xAxis->setRange(0, 100); p->replot(); }
}

void MainWindow::openARXDialog() {
    DialogARX diag(this); diag.setData(m_curA, m_curB, m_curK);
    if (diag.exec() == QDialog::Accepted) {
        m_curA = diag.getA(); m_curB = diag.getB(); m_curK = diag.getK();
        updateARXFromStrings();
    }
}

void MainWindow::updateARXFromStrings() { m_service.configureARX(m_curA, m_curB, m_curK); }

// --- NOWY, PEŁNY ZAPIS ---
void MainWindow::saveConfig() {
    QJsonObject obj;

    // 1. PID
    obj["pid_k"] = ui->spinK->value();
    obj["pid_ti"] = ui->spinTi->value();
    obj["pid_td"] = ui->spinTd->value();
    obj["pid_tryb"] = ui->comboTrybPID->currentIndex();

    // 2. Generator
    obj["gen_tryb"] = ui->comboTryb->currentIndex();
    obj["gen_okres"] = ui->spinOkres->value();
    obj["gen_amplituda"] = ui->spinAmplituda->value();
    obj["gen_stala"] = ui->spinSkladowaStala->value();
    obj["gen_wypelnienie"] = ui->spinWypelnienie->value();

    // 3. ARX
    obj["arx_a"] = m_curA;
    obj["arx_b"] = m_curB;
    obj["arx_k"] = m_curK;

    // 4. System
    obj["sys_interval"] = ui->spinInterval->value();

    QString path = QFileDialog::getSaveFileName(this, "Zapisz konfigurację", "", "JSON (*.json)");
    if (!path.isEmpty()) {
        QFile f(path);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(QJsonDocument(obj).toJson());
            f.close();
        }
    }
}

// --- NOWY, PEŁNY ODCZYT ---
void MainWindow::loadConfig() {
    QString path = QFileDialog::getOpenFileName(this, "Wczytaj konfigurację", "", "JSON (*.json)");
    if (path.isEmpty()) return;

    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
        QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();

        // Blokowanie sygnałów na chwilę zapobiega błędom przy masowej aktualizacji
        ui->spinK->setValue(obj["pid_k"].toDouble());
        ui->spinTi->setValue(obj["pid_ti"].toDouble());
        ui->spinTd->setValue(obj["pid_td"].toDouble());
        ui->comboTrybPID->setCurrentIndex(obj["pid_tryb"].toInt());

        ui->comboTryb->setCurrentIndex(obj["gen_tryb"].toInt());
        ui->spinOkres->setValue(obj["gen_okres"].toDouble());
        ui->spinAmplituda->setValue(obj["gen_amplituda"].toDouble());
        ui->spinSkladowaStala->setValue(obj["gen_stala"].toDouble());
        ui->spinWypelnienie->setValue(obj["gen_wypelnienie"].toDouble());

        m_curA = obj["arx_a"].toString();
        m_curB = obj["arx_b"].toString();
        m_curK = obj["arx_k"].toInt();

        ui->spinInterval->setValue(obj["sys_interval"].toInt());

        updateARXFromStrings();
        f.close();
        QMessageBox::information(this, "Sukces", "Konfiguracja wczytana pomyślnie.");
    }
}
