#include "dialogarx.h"
#include "ui_dialogarx.h"

DialogARX::DialogARX(QWidget *parent) : QDialog(parent), ui(new Ui::DialogARX) {
    ui->setupUi(this);

    connect(ui->checkLimity, &QCheckBox::toggled, this, [=](bool checked){
        ui->spinMinU->setEnabled(checked);
        ui->spinMaxU->setEnabled(checked);
        ui->spinMinY->setEnabled(checked);
        ui->spinMaxY->setEnabled(checked);
    });
}

DialogARX::~DialogARX() { delete ui; }

void DialogARX::setData(QString a, QString b, int k, double minU, double maxU, double minY, double maxY, double noise, bool limityWlaczone) {
    ui->editA->setText(a);
    ui->editB->setText(b);
    ui->spinK->setValue(k);

    ui->spinMinU->setValue(minU);
    ui->spinMaxU->setValue(maxU);
    ui->spinMinY->setValue(minY);
    ui->spinMaxY->setValue(maxY);
    ui->spinSzum->setValue(noise);

    ui->checkLimity->setChecked(limityWlaczone);

    ui->spinMinU->setEnabled(limityWlaczone);
    ui->spinMaxU->setEnabled(limityWlaczone);
    ui->spinMinY->setEnabled(limityWlaczone);
    ui->spinMaxY->setEnabled(limityWlaczone);
}

QString DialogARX::getA() const { return ui->editA->text(); }
QString DialogARX::getB() const { return ui->editB->text(); }

int DialogARX::getK() const { return ui->spinK->value(); }

double DialogARX::getMinU() const { return ui->spinMinU->value(); }
double DialogARX::getMaxU() const { return ui->spinMaxU->value(); }
double DialogARX::getMinY() const { return ui->spinMinY->value(); }
double DialogARX::getMaxY() const { return ui->spinMaxY->value(); }
double DialogARX::getNoise() const { return ui->spinSzum->value(); }

bool DialogARX::getLimityWlaczone() const { return ui->checkLimity->isChecked(); }
