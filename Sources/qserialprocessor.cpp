#include "qserialprocessor.h"


QSerialProcessor::QSerialProcessor(QObject *parent) :
    QObject(parent)
{
    connect(&m_serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(&m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleErrors(QSerialPort::SerialPortError)));
}

QSerialProcessor::~QSerialProcessor()
{
    close(); // close serial communication
}

bool QSerialProcessor::showPortSelectDialog()
{
    QDialog dialog;
    dialog.setWindowTitle(tr("SerialPort select"));
    dialog.setFixedSize(200,100);

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
     }
}

bool QSerialProcessor::open()
{
    this->close();

    m_serialPort.setPort(m_serialInfo);

    if(m_serialPort.open(QIODevice::ReadOnly) )
    {
        if( /*m_serialPort.setBaudRate(QSerialPort::Baud9600)
            && m_serialPort.setFlowControl(QSerialPort::NoFlowControl)
            && m_serialPort.setDataBits(QSerialPort::Data8)
            && m_serialPort.setParity(QSerialPort::NoParity)
            && m_serialPort.setStopBits(QSerialPort::OneStop)
            && m_serialPort.setReadBufferSize(256)
            &&*/ m_serialPort.setDataTerminalReady(QSerialPort::IgnorePolicy) ) // Qt 5.2.1 says that QSerialPort::IgnorePolicy is obsolete, but without this property explicit set, the transmission will not start...
        {
            qWarning() << "current portName = " << m_serialPort.portName();
            qWarning() << "current baudRate = " << m_serialPort.baudRate() << " bps";
            qWarning() << "current dataBits = " << m_serialPort.dataBits() << " bit";
            qWarning() << "current flowControl = " << m_serialPort.flowControl();
            qWarning() << "current parity = " << m_serialPort.parity();
            qWarning() << "current stopBits = " << m_serialPort.stopBits();
            qWarning() << "current portBuffer size = " << m_serialPort.readBufferSize() << " bits";
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
    m_data = m_serialPort.readAll() ;
    qWarning() << "readyRead() signal has occured, the quantity of incoming bytes is: " << m_data.size();
    emit dataUpdated(m_data); // time should be in ms format
}

