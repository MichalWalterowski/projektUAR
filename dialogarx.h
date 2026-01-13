#ifndef DIALOGARX_H
#define DIALOGARX_H
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>

namespace Ui { class DialogARX; }

class DialogARX : public QDialog {
    Q_OBJECT
public:
    explicit DialogARX(QWidget *parent = nullptr);
    ~DialogARX();
    void setData(QString a, QString b, int k);
    QString getA() const;
    QString getB() const;
    int getK() const;
private:
    Ui::DialogARX *ui;
};
#endif
