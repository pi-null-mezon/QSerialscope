#include "qtransmissiondialog.h"
#include "ui_qtransmissiondialog.h"

#define TIME_STEP 0.001 // in ms
#define VOLTAGE_STEP 0.1 // in V

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
    ui->Ediscretization->setText( QString::number(value * TIME_STEP,'f' ,3) );
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
    ui->RBlittleendian->setChecked(true);
    ui->RBbigendian->setChecked(false);
    ui->Ediscretization->setText("1.0");
    ui->Ebits->setText("8");
    ui->RBbigendian->setEnabled(false);
    ui->RBlittleendian->setEnabled(false);
}

void QTransmissionDialog::on_Dvoltage_valueChanged(int value)
{
    ui->Evoltage->setText(QString::number(value * VOLTAGE_STEP));
}
