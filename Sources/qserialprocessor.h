#ifndef QSERIALPROCESSOR_H
#define QSERIALPROCESSOR_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QByteArray>
#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

class QSerialProcessor : public QObject
{
    Q_OBJECT
public:
    explicit QSerialProcessor(QObject *parent = 0);
    ~QSerialProcessor();

signals:
    void dataUpdated(const QByteArray &array);

public slots:
    bool open();
    bool close();
    bool showPortSelectDialog();
    void handleErrors(QSerialPort::SerialPortError code);
    void readData();

private:
    QSerialPort m_serialPort;
    QSerialPortInfo m_serialInfo;
    QByteArray m_data;


};

#endif // QSERIALPROCESSOR_H
