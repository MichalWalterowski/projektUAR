#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <vector>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
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

    if (ui->verticalLayout) {
        ui->verticalLayout->setStretch(0, 0);
        ui->verticalLayout->setStretch(1, 1);
        ui->verticalLayout->setStretch(2, 0);
    }

    setupPlots();
    setupConnections();
    updateARXFromStrings();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupPlots()
{
    // --- 1. WYKRES: SYGNAŁ WYJŚCIOWY I WARTOŚĆ ZADANA ---
    plotY->addLayer("title", plotY->layer("main"), QCustomPlot::limAbove);
    graphY_zadana = plotY->addGraph();
    graphY_zadana->setPen(QPen(Qt::red, 2));
    graphY_zadana->setName("Wartość zadana (w)");

    graphY_regulowana = plotY->addGraph();
    graphY_regulowana->setPen(QPen(Qt::blue, 2));
    graphY_regulowana->setName("Wyjście obiektu (y)");

    plotY->xAxis->setLabel("Czas / Krok symulacji");
    plotY->yAxis->setLabel("Amplituda sygnałów");
    plotY->plotLayout()->insertRow(0);
    plotY->plotLayout()->addElement(0, 0, new QCPTextElement(plotY, "Regulacja: Wyjście - Zadana", QFont("sans", 12, QFont::Bold)));
    plotY->legend->setVisible(true);
    plotY->legend->setFont(QFont("sans", 9));
    plotY->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // --- 2. WYKRES: BŁĄD REGULACJI ---
    graphError = plotError->addGraph();
    graphError->setPen(QPen(Qt::darkCyan, 2));
    graphError->setName("Błąd (e)");

    plotError->xAxis->setLabel("Czas / Krok symulacji");
    plotError->yAxis->setLabel("Wartość błędu");
    plotError->plotLayout()->insertRow(0);
    plotError->plotLayout()->addElement(0, 0, new QCPTextElement(plotError, "Błąd regulacji e(t)", QFont("sans", 11, QFont::Bold)));
    plotError->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // --- 3. WYKRES: SYGNAŁ STERUJĄCY ---
    graphU = plotU->addGraph();
    graphU->setPen(QPen(Qt::black, 1.5));
    graphU->setName("Sterowanie (u)");

    plotU->xAxis->setLabel("Czas / Krok symulacji");
    plotU->yAxis->setLabel("Wartość sterowania");
    plotU->plotLayout()->insertRow(0);
    plotU->plotLayout()->addElement(0, 0, new QCPTextElement(plotU, "Sygnał sterujący u(t)", QFont("sans", 11, QFont::Bold)));
    plotU->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // --- 4. WYKRES: SKŁADOWE PID ---
    graphU_P = plotUComponents->addGraph();
    graphU_P->setPen(QPen(Qt::red));
    graphU_P->setName("Człon P");

    graphU_I = plotUComponents->addGraph();
    graphU_I->setPen(QPen(Qt::green));
    graphU_I->setName("Człon I");

    graphU_D = plotUComponents->addGraph();
    graphU_D->setPen(QPen(Qt::blue));
    graphU_D->setName("Człon D");

    plotUComponents->xAxis->setLabel("Czas / Krok symulacji");
    plotUComponents->yAxis->setLabel("Wartość składowych");
    plotUComponents->plotLayout()->insertRow(0);
    plotUComponents->plotLayout()->addElement(0, 0, new QCPTextElement(plotUComponents, "Składowe regulatora PID", QFont("sans", 11, QFont::Bold)));
    plotUComponents->legend->setVisible(true);
    plotUComponents->legend->setFont(QFont("sans", 8));
    plotUComponents->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // --- WSPÓLNE USTAWIENIA DLA WSZYSTKICH WYKRESÓW ---
    QList<QCustomPlot*> plots = {plotY, plotError, plotU, plotUComponents};
    for (QCustomPlot* p : plots) {
        p->xAxis->grid()->setSubGridVisible(true);
        p->yAxis->grid()->setSubGridVisible(true);
        p->setAntialiasedElements(QCP::aeAll);
        p->xAxis->setRange(0, 100);
        p->yAxis->setRange(-1.5, 1.5);
    }
}

void MainWindow::setupConnections() {
    connect(ui->btnStart, SIGNAL(clicked()), this, SLOT(startSimulation()));
    connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(stopSimulation()));
    connect(ui->btnReset, SIGNAL(clicked()), this, SLOT(resetSimulation()));
    connect(ui->btnOpenARX, SIGNAL(clicked()), this, SLOT(openARXDialog()));
    connect(ui->btnSaveConfig, SIGNAL(clicked()), this, SLOT(saveConfig()));
    connect(ui->btnLoadConfig, SIGNAL(clicked()), this, SLOT(loadConfig()));
    connect(simTimer, SIGNAL(timeout()), this, SLOT(simulateStep()));
}

void MainWindow::updateARXFromStrings() {
    std::vector<double> va, vb;
    QStringList aList = m_curA.split(",", Qt::SkipEmptyParts);
    QStringList bList = m_curB.split(",", Qt::SkipEmptyParts);
    for(const QString &s : aList) va.push_back(s.trimmed().toDouble());
    for(const QString &s : bList) vb.push_back(s.trimmed().toDouble());

    if(va.empty()) va.push_back(0.0);
    if(vb.empty()) vb.push_back(1.0);
    m_uar.getARX().setParams(va, vb, m_curK);
}

void MainWindow::updateUARFromUI() {
    m_uar.getPID().setParams(ui->spinK->value(), ui->spinTi->value(), ui->spinTd->value());
    TrybGen t = (ui->comboTryb->currentIndex() == 0) ? Pros : Sin;
    m_uar.getGen().setParams(t, ui->spinTT->value(), ui->spinTRZ->value(),
                             ui->spinAgen->value(), ui->spinSgen->value(), ui->spinPgen->value());
}

void MainWindow::simulateStep() {
    updateUARFromUI();
    double y = m_uar.symuluj();
    double x = m_step;

    graphY_zadana->addData(x, m_uar.getGen().getVal());
    graphY_regulowana->addData(x, y);
    graphError->addData(x, m_uar.getE());
    graphU->addData(x, m_uar.getU());
    graphU_P->addData(x, m_uar.getPID().getUP());
    graphU_I->addData(x, m_uar.getPID().getUI());
    graphU_D->addData(x, m_uar.getPID().getUD());

    QList<QCustomPlot*> plots = {plotY, plotError, plotU, plotUComponents};
    for (QCustomPlot* p : plots) {
        p->xAxis->setRange(m_step > 100 ? m_step - 100 : 0, m_step + 1);
        p->rescaleAxes();
        p->replot();
    }
    m_step++;
}

void MainWindow::openARXDialog() {
    DialogARX diag(this);
    diag.setData(m_curA, m_curB, m_curK);
    if (diag.exec() == QDialog::Accepted) {
        m_curA = diag.getA(); m_curB = diag.getB(); m_curK = diag.getK();
        updateARXFromStrings();
    }
}

void MainWindow::startSimulation() { simTimer->start(ui->spinInterval->value()); }
void MainWindow::stopSimulation() { simTimer->stop(); }
void MainWindow::resetSimulation() {
    simTimer->stop(); m_uar.reset(); m_step = 0;
    for(auto g : {graphY_zadana, graphY_regulowana, graphError, graphU, graphU_P, graphU_I, graphU_D}) g->data()->clear();
    for(auto p : {plotY, plotError, plotU, plotUComponents}) p->replot();
}

void MainWindow::saveConfig() {
    QJsonObject obj;
    obj["K"] = ui->spinK->value(); obj["Ti"] = ui->spinTi->value(); obj["Td"] = ui->spinTd->value();
    obj["curA"] = m_curA; obj["curB"] = m_curB; obj["curK"] = m_curK;
    obj["Tryb"] = ui->comboTryb->currentIndex();
    QFile f(QFileDialog::getSaveFileName(this, "Zapisz", "", "*.json"));
    if (f.open(QIODevice::WriteOnly)) { f.write(QJsonDocument(obj).toJson()); f.close(); }
}

void MainWindow::loadConfig() {
    QFile f(QFileDialog::getOpenFileName(this, "Otwórz", "", "*.json"));
    if (f.open(QIODevice::ReadOnly)) {
        QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
        ui->spinK->setValue(obj["K"].toDouble()); ui->spinTi->setValue(obj["Ti"].toDouble()); ui->spinTd->setValue(obj["Td"].toDouble());
        m_curA = obj["curA"].toString(); m_curB = obj["curB"].toString(); m_curK = obj["curK"].toInt();
        ui->comboTryb->setCurrentIndex(obj["Tryb"].toInt());
        updateARXFromStrings(); f.close();
    }
}
