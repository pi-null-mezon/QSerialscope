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
    enum BytesPerValue {One, Two};
    enum BitsOrder {LittleEndian, BigEndian};

signals:
    void dataUpdated(const quint16 *pt, quint16 length);

public slots:
    bool open();
    bool close();
    bool showPortSelectDialog();
    void handleErrors(QSerialPort::SerialPortError code);
    void readData();
    void initializeBuffer(quint16 length, BytesPerValue quantity, BitsOrder order);

private:
    QSerialPort m_serialPort;
    QSerialPortInfo m_serialInfo;
    QByteArray m_data;
    quint16 *v_values;
    quint16 m_length;
    BytesPerValue m_bytesPerValue;
    BitsOrder m_bytesOrder;

    void convertOneByteData();
    void convertTwoByteData();
};

#endif // QSERIALPROCESSOR_H
