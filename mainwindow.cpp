#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , simTimer(new QTimer(this))
{
    ui->setupUi(this);

    // 1. Konfiguracja mapowania wskaźników do widgetów z UI
    // Zakładam, że w Designerze nazwałeś widgety QCustomPlot odpowiednio:
    m_plotY = new QCustomPlot(); //ui->plotY;
    m_plotError = new QCustomPlot(); //ui->plotError;
    m_plotU = new QCustomPlot(); //ui->plotU;
    m_plotUComp = new QCustomPlot(); //ui->plotUComp;

    ui->gridLayoutCharts->addWidget(m_plotY, 0, 0);
    ui->gridLayoutCharts->addWidget(m_plotError, 0, 1);
    ui->gridLayoutCharts->addWidget(m_plotU, 1, 0);
    ui->gridLayoutCharts->addWidget(m_plotUComp, 1, 1);

    // 2. Konfiguracja wykresów i połączeń
    setupPlots();
    setupConnections();

    // 3. Inicjalizacja parametrów początkowych
    // Wysyłamy domyślne ustawienia z GUI do Serwisu
    updateParameters();
    pushARXParamsToService();
}

MainWindow::~MainWindow() {
    delete ui;
}

// --- KONFIGURACJA WYKRESÓW ---
void MainWindow::setupPlots() {
    // === Wykres 1: Wartość Zadana (Setpoint) i Regulowana (Y) ===
    m_plotY->addGraph(); // Index 0: Wartość regulowana
    m_graphY_regulowana = m_plotY->graph(0);
    m_graphY_regulowana->setPen(QPen(Qt::blue));
    m_graphY_regulowana->setName("y (Wyjście)");

    m_plotY->addGraph(); // Index 1: Wartość zadana
    m_graphY_zadana = m_plotY->graph(1);
    m_graphY_zadana->setPen(QPen(Qt::red)); // Zadana na czerwono
    // Linia przerywana dla wartości zadanej
    QPen setpointPen(Qt::red);
    setpointPen.setStyle(Qt::DashLine);
    m_graphY_zadana->setPen(setpointPen);
    m_graphY_zadana->setName("w (Zadana)");

    m_plotY->legend->setVisible(true);
    m_plotY->setInteraction(QCP::iRangeDrag, true);
    m_plotY->setInteraction(QCP::iRangeZoom, true);
    m_plotY->yAxis->setLabel("Wartość");

    //m_plotY->setAttribute(Qt::WA_StyledBackground, true);
    //m_plotY->setStyleSheet("border: 3px solid #222222;");

    // === Wykres 2: Uchyb (Error) ===
    m_plotError->addGraph();
    m_graphError = m_plotError->graph(0);
    m_graphError->setPen(QPen(Qt::darkRed));
    m_plotError->yAxis->setLabel("Uchyb (e)");

    // === Wykres 3: Sterowanie (U) ===
    m_plotU->addGraph();
    m_graphU = m_plotU->graph(0);
    m_graphU->setPen(QPen(Qt::darkGreen));
    m_plotU->yAxis->setLabel("Sterowanie (u)");

    // === Wykres 4: Składowe PID ===
    m_plotUComp->addGraph(); // P
    m_graphU_P = m_plotUComp->graph(0);
    m_graphU_P->setPen(QPen(Qt::red));
    m_graphU_P->setName("P");

    m_plotUComp->addGraph(); // I
    m_graphU_I = m_plotUComp->graph(1);
    m_graphU_I->setPen(QPen(Qt::blue));
    m_graphU_I->setName("I");

    m_plotUComp->addGraph(); // D
    m_graphU_D = m_plotUComp->graph(2);
    m_graphU_D->setPen(QPen(Qt::green));
    m_graphU_D->setName("D");

    m_plotUComp->legend->setVisible(true);
    m_plotUComp->yAxis->setLabel("Składowe PID");
}

// --- POŁĄCZENIA SYGNAŁÓW I SLOTÓW ---
void MainWindow::setupConnections() {
    // Timer symulacji
    connect(simTimer, &QTimer::timeout, this, &MainWindow::simulateStep);

    // Przyciski sterujące
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::startSimulation);
    connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::stopSimulation);
    connect(ui->btnReset, &QPushButton::clicked, this, &MainWindow::resetSimulation);

    // Przycisk konfiguracji ARX
    connect(ui->btnARX, &QPushButton::clicked, this, &MainWindow::openARXDialog);

    // Zapis i odczyt
    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::saveConfig);
    connect(ui->btnLoad, &QPushButton::clicked, this, &MainWindow::loadConfig);

    // Automatyczna aktualizacja parametrów po zmianie w GUI (PID / Generator)
    // Łączymy sygnał valueChanged/currentIndexChanged każdego spinboxa z updateParameters
    QList<QDoubleSpinBox*> spins = this->findChildren<QDoubleSpinBox*>();
    for(auto spin : spins) {
        connect(spin, &QDoubleSpinBox::editingFinished, this, &MainWindow::updateParameters);
    }
    connect(ui->comboPIDMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateParameters);
    connect(ui->comboGenType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateParameters);
}

// --- GŁÓWNA PĘTLA SYMULACJI ---
void MainWindow::simulateStep() {
    // 1. Wykonaj krok symulacji w warstwie usług
    SimulationData data = m_service.nextStep(m_step);

    // 2. Dodaj dane do wykresów
    double t = data.x * (ui->spinInterval->value() / 1000.0); // Czas w sekundach do wyświetlania

    m_graphY_regulowana->addData(t, data.y);
    m_graphY_zadana->addData(t, data.setpoint);
    m_graphError->addData(t, data.error);
    m_graphU->addData(t, data.u);
    m_graphU_P->addData(t, data.uP);
    m_graphU_I->addData(t, data.uI);
    m_graphU_D->addData(t, data.uD);

    // 3. Przesuwanie okna ("Sliding Window")
    // Domyślne okno obserwacji np. 10 sekund
    double windowSize = 10.0;
    double currentTime = t;
    double startTime = currentTime - windowSize;
    if (startTime < 0) startTime = 0;

    // Ustawienie zakresów Osi X (Czas)
    m_plotY->xAxis->setRange(startTime, currentTime + 0.5); // +0.5s marginesu z prawej
    m_plotError->xAxis->setRange(startTime, currentTime + 0.5);
    m_plotU->xAxis->setRange(startTime, currentTime + 0.5);
    m_plotUComp->xAxis->setRange(startTime, currentTime + 0.5);

    // Usuwanie starych danych z pamięci (dla optymalizacji)
    // Usuwamy wszystko co jest starsze niż (startTime - 2s zapasu)
    double deleteThreshold = startTime - 2.0;
    if (deleteThreshold > 0) {
        m_graphY_regulowana->data()->removeBefore(deleteThreshold);
        m_graphY_zadana->data()->removeBefore(deleteThreshold);
        m_graphError->data()->removeBefore(deleteThreshold);
        m_graphU->data()->removeBefore(deleteThreshold);
        // ... (reszta grafów też powinna być czyszczona)
    }

    // 4. Autoskalowanie Osi Y
    // Ważne: Rescale tylko dla widocznego zakresu
    m_plotY->yAxis->rescale(true);
    m_plotError->yAxis->rescale(true);
    m_plotU->yAxis->rescale(true);
    m_plotUComp->yAxis->rescale(true);

    // 5. Odświeżenie widoku
    m_plotY->replot();
    m_plotError->replot();
    m_plotU->replot();
    m_plotUComp->replot();

    m_step++;
}

// --- STEROWANIE ---
void MainWindow::startSimulation() {
    int interval = ui->spinInterval->value();
    if(interval < 10) interval = 10;

    // Zaktualizuj parametry przed startem (na wszelki wypadek)
    updateParameters();

    simTimer->setInterval(interval);
    simTimer->start();
}

void MainWindow::stopSimulation() {
    simTimer->stop();
}

void MainWindow::resetSimulation() {
    stopSimulation();
    m_service.reset();
    m_step = 0;

    // Czyszczenie wykresów
    m_graphY_regulowana->data()->clear();
    m_graphY_zadana->data()->clear();
    m_graphError->data()->clear();
    m_graphU->data()->clear();
    m_graphU_P->data()->clear();
    m_graphU_I->data()->clear();
    m_graphU_D->data()->clear();

    // Replot pustych wykresów
    m_plotY->replot();
    m_plotError->replot();
    m_plotU->replot();
    m_plotUComp->replot();
}

// --- KONFIGURACJA W LOCIE ---
void MainWindow::updateParameters() {
    // 1. PID
    m_service.configurePID(
        ui->spinK->value(),
        ui->spinTi->value(),
        ui->spinTd->value(),
        ui->comboPIDMethod->currentIndex()
        );

    // 2. Generator
    // Ważne: Przekazujemy interwał timera, aby poprawnie liczyć czas
    m_service.configureGenerator(
        ui->comboGenType->currentIndex(),
        ui->spinOkres->value(),
        ui->spinAmp->value(),
        ui->spinOffset->value(),
        ui->spinFill->value(),
        ui->spinInterval->value()
        );

    // Jeśli zmieniono interwał w trakcie pracy, timer trzeba zaktualizować
    if (simTimer->isActive()) {
        simTimer->setInterval(ui->spinInterval->value());
    }
}

// --- OBSŁUGA OKNA DIALOGOWEGO ARX ---
void MainWindow::openARXDialog() {
    DialogARX dlg(this);

    // 1. Wczytanie aktualnych ustawień do okna (z pamięci podręcznej MainWindow)
    dlg.setData(m_curA, m_curB, m_curK,
                m_curMinU, m_curMaxU, m_curMinY, m_curMaxY, m_curNoise,
                m_curLimitsOn);

    // 2. Otwarcie okna i oczekiwanie na wynik
    if (dlg.exec() == QDialog::Accepted) {
        // 3. Pobranie nowych danych po zatwierdzeniu
        m_curA = dlg.getA();
        m_curB = dlg.getB();
        m_curK = dlg.getK();

        m_curMinU = dlg.getMinU();
        m_curMaxU = dlg.getMaxU();
        m_curMinY = dlg.getMinY();
        m_curMaxY = dlg.getMaxY();
        m_curNoise = dlg.getNoise();
        m_curLimitsOn = dlg.getLimityWlaczone();

        // 4. Wysłanie danych do serwisu
        pushARXParamsToService();
    }
}

void MainWindow::pushARXParamsToService() {
    // Metoda pomocnicza wysyłająca cache do warstwy logiki
    m_service.configureARX(m_curA, m_curB, m_curK,
                           m_curMinU, m_curMaxU, m_curMinY, m_curMaxY, m_curNoise,
                           m_curLimitsOn);
}

// --- ZAPIS I ODCZYT (JSON) ---
void MainWindow::saveConfig() {
    QString fileName = QFileDialog::getSaveFileName(this, "Zapisz konfigurację", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;

    QJsonObject root;

    // Zapisujemy ARX
    QJsonObject arxObj;
    arxObj["A"] = m_curA;
    arxObj["B"] = m_curB;
    arxObj["k"] = m_curK;
    arxObj["minU"] = m_curMinU;
    arxObj["maxU"] = m_curMaxU;
    arxObj["minY"] = m_curMinY;
    arxObj["maxY"] = m_curMaxY;
    arxObj["noise"] = m_curNoise;
    arxObj["limitsOn"] = m_curLimitsOn;
    root["ARX"] = arxObj;

    // Zapisujemy PID
    QJsonObject pidObj;
    pidObj["k"] = ui->spinK->value();
    pidObj["Ti"] = ui->spinTi->value();
    pidObj["Td"] = ui->spinTd->value();
    pidObj["method"] = ui->comboPIDMethod->currentIndex();
    root["PID"] = pidObj;

    // Zapisujemy Generator
    QJsonObject genObj;
    genObj["type"] = ui->comboGenType->currentIndex();
    genObj["period"] = ui->spinOkres->value();
    genObj["amp"] = ui->spinAmp->value();
    genObj["offset"] = ui->spinOffset->value();
    genObj["fill"] = ui->spinFill->value();
    genObj["interval"] = ui->spinInterval->value();
    root["Gen"] = genObj;

    QJsonDocument doc(root);
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void MainWindow::loadConfig() {
    QString fileName = QFileDialog::getOpenFileName(this, "Wczytaj konfigurację", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    // Wczytanie ARX
    if (root.contains("ARX")) {
        QJsonObject arxObj = root["ARX"].toObject();
        m_curA = arxObj["A"].toString();
        m_curB = arxObj["B"].toString();
        m_curK = arxObj["k"].toInt();
        m_curMinU = arxObj["minU"].toDouble();
        m_curMaxU = arxObj["maxU"].toDouble();
        m_curMinY = arxObj["minY"].toDouble();
        m_curMaxY = arxObj["maxY"].toDouble();
        m_curNoise = arxObj["noise"].toDouble();
        m_curLimitsOn = arxObj["limitsOn"].toBool();

        pushARXParamsToService(); // Aktualizacja backendu
    }

    // Wczytanie PID
    if (root.contains("PID")) {
        QJsonObject pidObj = root["PID"].toObject();
        ui->spinK->setValue(pidObj["k"].toDouble());
        ui->spinTi->setValue(pidObj["Ti"].toDouble());
        ui->spinTd->setValue(pidObj["Td"].toDouble());
        ui->comboPIDMethod->setCurrentIndex(pidObj["method"].toInt());
    }

    // Wczytanie Generatora
    if (root.contains("Gen")) {
        QJsonObject genObj = root["Gen"].toObject();
        ui->comboGenType->setCurrentIndex(genObj["type"].toInt());
        ui->spinOkres->setValue(genObj["period"].toDouble());
        ui->spinAmp->setValue(genObj["amp"].toDouble());
        ui->spinOffset->setValue(genObj["offset"].toDouble());
        ui->spinFill->setValue(genObj["fill"].toDouble());
        ui->spinInterval->setValue(genObj["interval"].toInt());
    }

    // Wymuszenie aktualizacji wszystkich parametrów
    updateParameters();
}
