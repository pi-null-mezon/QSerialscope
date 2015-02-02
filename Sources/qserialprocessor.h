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
    explicit QSerialProcessor(QObject *parent = 0, quint16 bufferLength = 128);
    ~QSerialProcessor();
    enum BytesPerValue {One, Two, Two8bitSignals};
    enum BitsOrder {LittleEndian, BigEndian};

signals:
    void dataUpdated(const quint16 *pt, quint16 length);
    void dataUpdatedSecondChanell(const quint16 *pt, quint16 length);
    void signalsUpdated(const quint16 *pt1, const quint16 *pt2, quint16 length);

public slots:
    bool open();
    bool close();
    bool showPortSelectDialog();
    void handleErrors(QSerialPort::SerialPortError code);
    void readData();
    void setDataFormat(BytesPerValue perValue, BitsOrder order);

private:
    QSerialPort m_serialPort;
    QSerialPortInfo m_serialInfo;
    QByteArray m_dataBuffer;
    quint16 m_bufferLength;
    quint16 *v_firstSignalCounts;
    quint16 *v_secondSignalCounts;
    BytesPerValue m_bytesPerValue;
    BitsOrder m_bytesOrder;
    quint8 m_lastByte;
    void convertOneByteData();
    void convertTwoByteData();
    void convertTwo8bitSignalsData();
};

#endif // QSERIALPROCESSOR_H
