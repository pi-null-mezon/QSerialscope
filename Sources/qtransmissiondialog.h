#ifndef QTRANSMISSIONDIALOG_H
#define QTRANSMISSIONDIALOG_H

#include <QDialog>
#include "qserialprocessor.h"

namespace Ui {
class QTransmissionDialog;
}

class QTransmissionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QTransmissionDialog(QWidget *parent = 0);
    ~QTransmissionDialog();

public slots:
    QSerialProcessor::BitsOrder getBitsOrder() const;
    uint getBitsNumber() const;
    qreal getDiscretizationPeriod() const;
    qreal getReferenceVoltage() const;

private slots:
    void on_RBlittleendian_clicked();

    void on_RBbigendian_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_Ddiscretization_valueChanged(int value);

    void on_Bdefault_clicked();

    void on_Dvoltage_valueChanged(int value);

private:
    Ui::QTransmissionDialog *ui;
};

#endif // QTRANSMISSIONDIALOG_H
