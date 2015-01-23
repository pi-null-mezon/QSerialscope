#include "qserialprocessor.h"


QSerialProcessor::QSerialProcessor(QObject *parent, quint16 bufferLength) :
    QObject(parent),
    m_bytesOrder(LittleEndian),
    m_bytesPerValue(One)
{
    m_bufferLength = bufferLength;
    v_signalCounts = new quint16[bufferLength];

    connect(&m_serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(&m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleErrors(QSerialPort::SerialPortError)));
}

QSerialProcessor::~QSerialProcessor()
{
    close(); // close serial communication
    delete[] v_signalCounts;
}

bool QSerialProcessor::showPortSelectDialog()
{
    QDialog dialog;
    dialog.setWindowTitle(tr("SerialPort select"));
    dialog.setFixedSize(196,118);

    QVBoxLayout layout;

        QGroupBox groupbox(tr("Select appropriate port"));
        QVBoxLayout l_groupbox;
            QComboBox combobox;
            QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
            for(int i = 0; i < ports.size(); i++)
            {
                combobox.addItem( ports.at(i).portName() );
            }
        l_groupbox.addWidget(&combobox);
        groupbox.setLayout(&l_groupbox);

        QHBoxLayout l_buttons;
            QPushButton b_accept(tr("Accept"));
            connect(&b_accept, SIGNAL(clicked()), &dialog, SLOT(accept()));
            QPushButton b_cancel(tr("Cancel"));
            connect(&b_cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
        l_buttons.addWidget(&b_accept);
        l_buttons.addWidget(&b_cancel);

     layout.addWidget(&groupbox);
     layout.addLayout(&l_buttons);

     dialog.setLayout(&layout);

     if(dialog.exec() == QDialog::Accepted)
     {
         m_serialInfo = ports[combobox.currentIndex()];
         return true;
     }
     else
     {
        return false;
     }
}

bool QSerialProcessor::open()
{
    this->close();

    m_serialPort.setPort(m_serialInfo);

    if(m_serialPort.open(QIODevice::ReadOnly) )
    {
        if( /*m_serialPort.setBaudRate(QSerialPort::Baud115200)
            && m_serialPort.setFlowControl(QSerialPort::NoFlowControl)
            && m_serialPort.setDataBits(QSerialPort::Data8)
            && m_serialPort.setParity(QSerialPort::NoParity)
            && m_serialPort.setStopBits(QSerialPort::OneStop)
            &&*/ m_serialPort.setDataTerminalReady(QSerialPort::IgnorePolicy) ) // Qt 5.2.1 says that QSerialPort::IgnorePolicy is obsolete, but without this property explicit set, the transmission will not start...
        {
            m_serialPort.setReadBufferSize( m_bufferLength );
            qWarning() << "portName = " << m_serialPort.portName();
            qWarning() << "baudRate = " << m_serialPort.baudRate() << " bps";
            qWarning() << "dataBits = " << m_serialPort.dataBits() << " bit";
            qWarning() << "flowControl = " << m_serialPort.flowControl();
            qWarning() << "parity = " << m_serialPort.parity();
            qWarning() << "stopBits = " << m_serialPort.stopBits();
            qWarning() << "readBufferSize = " << m_serialPort.readBufferSize() << " bytes";
        }
        else
        {
            qWarning() << "Can not set appropriate settings for the COM port!";
        }
        return true;
    }
    return false;
}

bool QSerialProcessor::close()
{
    if(m_serialPort.isOpen())
    {
        m_serialPort.close();
        return true;
    }
    return false;
}

void QSerialProcessor::handleErrors(QSerialPort::SerialPortError code)
{
    qWarning() << "Error has occurred, error code: " << code;
    //TO DO...
}

void QSerialProcessor::readData()
{
    m_dataBuffer = m_serialPort.readAll();
    qWarning() << "readyRead() signal has occured, the quantity of incoming bytes is: " << (quint8)m_dataBuffer.size();
    switch(m_bytesPerValue)
    {
        case One:
            convertOneByteData();
            break;
        case Two:
            convertTwoByteData();
            break;
    }
}

void QSerialProcessor::setDataFormat(BytesPerValue perValue, BitsOrder order)
{
    m_bytesPerValue = perValue;
    m_bytesOrder = order;
}

void QSerialProcessor::convertOneByteData()
{
    for(int i = 0; i < m_dataBuffer.size(); i++)
    {
        v_signalCounts[i] = 0x00FF & (quint16)m_dataBuffer.at(i);
    }
    emit dataUpdated(v_signalCounts, m_dataBuffer.size());
}

void QSerialProcessor::convertTwoByteData()
{
    /* Note that in case of Two bytes per value format
     * application waits following format of byte flow:
     * ...(0xFF)(0xXX)(0xXX)(0xFF)(0xXX)(0xXX)...
     * where 0xFF is used as format pointer         */

    quint8 shift = 0; // initial shift of data bytes in incoming flow
    if(m_dataBuffer.size() > 5)
    {
        if((0xFF & m_dataBuffer.at(0) & m_dataBuffer.at(3)) == 0xFF)
            shift = 1;
        else if((0xFF & m_dataBuffer.at(1) & m_dataBuffer.at(4)) == 0xFF)
            shift = 2;

        int position;
        if(m_bytesOrder == LittleEndian)
        {
            for(int i = 0; (i * 3 + shift + 1) < m_dataBuffer.size(); i++)
            {
                position = i * 3 + shift;
                v_signalCounts[i] = (0x00FF & (quint16)m_dataBuffer.at( position )) << 8;
                v_signalCounts[i] |= 0x00FF & (quint16)m_dataBuffer.at( position + 1 );
            }
        }
        else // m_bytesOrder == BigEndian
        {
            for(int i = 0; (i * 3 + shift + 1) < m_dataBuffer.size(); i++)
            {
                position = i * 3 + shift;
                v_signalCounts[i] = 0x00FF & (quint16)m_dataBuffer.at( position );
                v_signalCounts[i] |= (0x00FF &(quint16)m_dataBuffer.at( position + 1 )) << 8;
            }
        }
        emit dataUpdated(v_signalCounts, m_dataBuffer.size()/3);
    }
}

