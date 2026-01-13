#include "dialogarx.h"
#include "ui_dialogarx.h"

DialogARX::DialogARX(QWidget *parent) : QDialog(parent), ui(new Ui::DialogARX) {
    ui->setupUi(this);
}

DialogARX::~DialogARX() { delete ui; }

void DialogARX::setData(QString a, QString b, int k) {
    ui->editA->setText(a); ui->editB->setText(b); ui->spinK->setValue(k);
}

QString DialogARX::getA() const { return ui->editA->text(); }
QString DialogARX::getB() const { return ui->editB->text(); }
int DialogARX::getK() const { return ui->spinK->value(); }
