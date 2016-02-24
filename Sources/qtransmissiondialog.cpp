#include "qtransmissiondialog.h"
#include "ui_qtransmissiondialog.h"

#include "qserialprocessor.h"

QTransmissionDialog::QTransmissionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QTransmissionDialog)
{
    ui->setupUi(this);
}

QTransmissionDialog::~QTransmissionDialog()
{
    delete ui;
}

void QTransmissionDialog::on_RBlittleendian_clicked()
{
    ui->RBlittleendian->setChecked(true);
    ui->RBbigendian->setChecked(false);
}

void QTransmissionDialog::on_RBbigendian_clicked()
{
    ui->RBlittleendian->setChecked(false);
    ui->RBbigendian->setChecked(true);
}

void QTransmissionDialog::on_pushButton_clicked()
{
    quint8 bits = ui->Ebits->text().toUInt();
    if(bits < 16)
    {
        bits++;
        if(bits > 8)
        {
            ui->RBbigendian->setEnabled(true);
            ui->RBlittleendian->setEnabled(true);
        }
    }
    ui->Ebits->setText(QString::number(bits));
}

void QTransmissionDialog::on_pushButton_2_clicked()
{
    quint8 bits = ui->Ebits->text().toUInt();
    if(bits > 1)
    {
        bits--;
        if(bits < 9)
        {
            ui->RBbigendian->setEnabled(false);
            ui->RBlittleendian->setEnabled(false);
        }
    }
    ui->Ebits->setText(QString::number(bits));
}

void QTransmissionDialog::on_Ddiscretization_valueChanged(int value)
{
    ui->Ediscretization->setText( QString::number(value * 0.1, 'f', 2) );
}

QSerialProcessor::BitsOrder QTransmissionDialog::getBitsOrder() const
{
    if( ui->RBlittleendian->isChecked() )
        return QSerialProcessor::LittleEndian;
    else
        return QSerialProcessor::BigEndian;
}

qreal QTransmissionDialog::getDiscretizationPeriod() const
{
    return ui->Ediscretization->text().toDouble();
}

qreal QTransmissionDialog::getReferenceVoltage() const
{
    return ui->Evoltage->text().toDouble();
}

uint QTransmissionDialog::getBitsNumber() const
{
    return ui->Ebits->text().toUInt();
}

void QTransmissionDialog::on_Bdefault_clicked()
{
    ui->RBlittleendian->setChecked(false);
    ui->RBbigendian->setChecked(true);
    ui->Ddiscretization->setValue(10);
    ui->Dvoltage->setValue(50);
    ui->Ebits->setText("10");
    ui->RBbigendian->setEnabled(true);
    ui->RBlittleendian->setEnabled(true);
}

void QTransmissionDialog::on_Dvoltage_valueChanged(int value)
{
    ui->Evoltage->setText(QString::number(value * 0.1, 'f', 1));
}
