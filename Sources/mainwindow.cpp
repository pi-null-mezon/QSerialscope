#include "mainwindow.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    this->setWindowTitle(APP_NAME);
    this->setMinimumSize(320, 240);

    //Central widget
    pt_centralWidget = new QWidget();
    this->setCentralWidget( pt_centralWidget );
    pt_centralLayout = new QVBoxLayout();
    pt_centralLayout->setMargin(5);
    pt_centralWidget->setLayout(pt_centralLayout);

    //Plot widges
    pt_plotWidget = new QPlot(this, "SERIAL PORT BUFFER TRACE", true, false);
    pt_plotWidget->set_unsigned(true);
    pt_signalPlot = new QPlot(this, "CENTERED & NORMALISED SIGNAL", false, true);
    pt_spectrumPlot = new QPlot(this, "AMPLITUDE SPECTRUM", false, true);
    pt_spectrumPlot->set_unsigned(true);
    pt_centralLayout->addWidget(pt_plotWidget);
    pt_centralLayout->addWidget(pt_signalPlot);
    pt_centralLayout->addWidget(pt_spectrumPlot);

    //Initialization
    _createActions();
    _createMenus();
    _createThreads();

    //Resize
    this->resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    this->statusBar()->showMessage(tr("Context menu available on right click"));
}

MainWindow::~MainWindow()
{
    pt_serialThread->quit();
    pt_serialThread->wait();

    if(pt_harmonicProcessor)
    {
        pt_harmonicThread->quit();
        pt_harmonicThread->wait();
    }
}

void MainWindow::_createActions()
{
    pt_startAction = new QAction(tr("&Start"), this);
    pt_startAction->setShortcut(QKeySequence(tr("Alt+S")));
    pt_startAction->setStatusTip(tr("Start measurements"));
    connect(pt_startAction, SIGNAL(triggered()), this, SLOT(openSerialConnection()));

    pt_stopAction = new QAction(tr("Sto&p"), this);
    pt_stopAction->setShortcut(QKeySequence(tr("Alt+X")));
    pt_stopAction->setStatusTip(tr("Stop measurements"));
    connect(pt_stopAction, SIGNAL(triggered()), this, SLOT(closeSerialConnection()));

    pt_exitAction = new QAction(tr("&Exit"), this);
    pt_exitAction->setStatusTip(tr("Exit from application"));
    connect(pt_exitAction, SIGNAL(triggered()), this, SLOT(close()));

    pt_aboutAction = new QAction(tr("&About"), this);
    pt_aboutAction->setStatusTip(tr("Open about dialog"));
    connect(pt_aboutAction, SIGNAL(triggered()), this, SLOT(open_about_dialog()));

    pt_recordAction = new QAction(tr("&Record"), this);
    pt_recordAction->setShortcut(QKeySequence(tr("Ctrl+R")));
    pt_recordAction->setStatusTip(tr("Enables/disables record"));
    connect(pt_recordAction, &QAction::triggered, this, &MainWindow::startRecord);
    pt_recordAction->setCheckable(true);

    pt_strobeAction = new QAction(tr("&Strobe"), this);
    pt_strobeAction->setStatusTip(tr("Open strobe adjustment dialog"));
    pt_strobeAction->setShortcut(QKeySequence(tr("Alt+A")));
    connect(pt_strobeAction, SIGNAL(triggered()), this, SLOT(adjustStrobe()));
}

void MainWindow::_createMenus()
{
    pt_controlMenu = this->menuBar()->addMenu(tr("&Controls"));
    pt_controlMenu->addAction(pt_startAction);
    pt_controlMenu->addAction(pt_stopAction);
    pt_controlMenu->addSeparator();
    pt_controlMenu->addAction(pt_recordAction);
    pt_controlMenu->addAction(pt_strobeAction);
    pt_controlMenu->addSeparator();
    pt_controlMenu->addAction(pt_exitAction);
    pt_helpMenu = this->menuBar()->addMenu(tr("&Help"));
    pt_helpMenu->addAction(pt_aboutAction);
}

void MainWindow::_createThreads()
{
    //Serial processor
    pt_serialProcessor = new QSerialProcessor(NULL);
    pt_serialThread = new QThread(this);
    pt_serialProcessor->moveToThread(pt_serialThread);
    connect(this, SIGNAL(signalToOpenConnection()), pt_serialProcessor, SLOT(open()));
    connect(this, SIGNAL(signalToCloseConnection()), pt_serialProcessor, SLOT(close()));
    connect(pt_serialThread, SIGNAL(finished()), pt_serialProcessor, SLOT(deleteLater()));
    pt_serialThread->start();

    //Timer
    m_timer.setTimerType(Qt::PreciseTimer);
    m_timer.setInterval(1000);

    //Harmonic processor
    pt_harmonicProcessor = NULL;
    pt_harmonicThread = new QThread(this);
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(pt_startAction);
    menu.addAction(pt_stopAction);
    menu.addSeparator();
    menu.addAction(pt_strobeAction);
    menu.exec(event->globalPos());
}

void MainWindow::showAboutDialog()
{
    QDialog *aboutDialog = new QDialog();
    aboutDialog->setWindowTitle("About " + QString(APP_NAME));
    aboutDialog->setFixedSize(256,128);

    QVBoxLayout *tempLayout = new QVBoxLayout();
    tempLayout->setMargin(5);

    QLabel *projectnameLabel = new QLabel(QString(APP_NAME) + "\t" + QString(APP_VERSION));
    projectnameLabel->setFrameStyle(QFrame::Box | QFrame::Sunken);
    projectnameLabel->setAlignment(Qt::AlignCenter);
    QLabel *projectauthorsLabel = new QLabel(QString(APP_AUTHOR) + "\n\n" + QString(APP_COMPANY) + "\n\n" + QString::number(APP_YEAR));
    projectauthorsLabel->setWordWrap(true);
    projectauthorsLabel->setAlignment(Qt::AlignCenter);
    QLabel *hyperlinkLabel = new QLabel(QString(APP_WEB));
    hyperlinkLabel->setToolTip(tr("Send message"));
    hyperlinkLabel->setOpenExternalLinks(true);
    hyperlinkLabel->setAlignment(Qt::AlignCenter);

    tempLayout->addWidget(projectnameLabel);
    tempLayout->addWidget(projectauthorsLabel);
    tempLayout->addWidget(hyperlinkLabel);

    aboutDialog->setLayout(tempLayout);
    aboutDialog->exec();

       delete hyperlinkLabel;
       delete projectauthorsLabel;
       delete projectnameLabel;
       delete tempLayout;
       delete aboutDialog;
}

void MainWindow::openSerialConnection()
{  
    if(pt_serialProcessor->open())
    {
        if(pt_harmonicProcessor)
        {
            pt_harmonicThread->quit();
            pt_harmonicThread->wait();
        }

        pt_harmonicProcessor = new QHarmonicProcessor();
        pt_harmonicProcessor->moveToThread(pt_harmonicThread);
        connect(pt_harmonicThread, SIGNAL(finished()), pt_harmonicProcessor, SLOT(deleteLater()));

        pt_signalPlot->set_string(QString::number(pt_harmonicProcessor->getDatalength()));
        pt_signalPlot->set_extrastring(QString::number(pt_harmonicProcessor->getBufferlength()));
        connect(pt_serialProcessor, SIGNAL(dataUpdated(QByteArray)), pt_harmonicProcessor, SLOT(readByteBuffer(QByteArray)));
        connect(pt_harmonicProcessor, SIGNAL(signalUpdated(const qreal*,quint32)), pt_signalPlot, SLOT(read_Data(const qreal*,quint32)));
        connect(&m_timer, SIGNAL(timeout()), pt_harmonicProcessor,SLOT(computeFrequency()));
        connect(pt_harmonicProcessor, SIGNAL(spectrumUpdated(const qreal*,quint32)), pt_spectrumPlot, SLOT(read_Data(const qreal*,quint32)));
        connect(pt_harmonicProcessor, SIGNAL(frequencyUpdated(qreal,qreal)), pt_spectrumPlot, SLOT(take_frequency(qreal,qreal)));



    }

    emit signalToOpenConnection();
    m_timer.start();
}

void MainWindow::closeSerialConnection()
{
    m_timer.stop();
    emit signalToCloseConnection();
}

void MainWindow::startRecord()
{
    if(!m_outputfile.isOpen())
    {
        QString str = QFileDialog::getOpenFileName(this, tr("Save file"), "/records/record_1.txt", tr("Text file(*.txt)"));
        m_outputfile.setFileName(str);
        while(!m_outputfile.open(QFile::WriteOnly))
        {
            QMessageBox msg(QMessageBox::Warning, tr("Warning message"), tr("Can not save"), QMessageBox::Open | QMessageBox::Close, this);
            if(msg.exec() == QMessageBox::Open)
            {
                str = QFileDialog::getOpenFileName(this, tr("Save file"), "/records/record_1.txt", tr("Text file(*.txt)"));
                m_outputfile.setFileName(str);
            }
            else
            {
                pt_recordAction->setChecked(false);
                break;
            }
        }
        if(m_outputfile.isOpen())
        {
            m_textstream.setDevice(&m_outputfile);
            m_textstream << QString(APP_NAME) + " output record\n"
                         << "Record was started at " + QDateTime::currentDateTime().toString("dd.MM.yyy hh.mm.ss") + "\n"
                         << "discretization period: xx ms , quantification step: yy V/unit\n"
                         << "Value, unit\n";
            connect(pt_serialProcessor, SIGNAL(dataUpdated(const QByteArray&)), this, SLOT(makeRecord(QByteArray)));
            pt_recordAction->setChecked(true);
        }
    }
    else
    {
        m_outputfile.close();
        disconnect(pt_serialProcessor, SIGNAL(dataUpdated(const QByteArray&)), this, SLOT(makeRecord(QByteArray)));
        pt_recordAction->setChecked(false);
    }
}

void MainWindow::makeRecord(const QByteArray &data)
{
   if(m_outputfile.isOpen())
   {
       m_textstream << data << "\n";
   }
}

void MainWindow::adjustStrobe()
{
    if(pt_harmonicProcessor)
    {
        QDialog dialog;
        dialog.setWindowTitle(tr("Strobe select dialog"));
        dialog.setFixedSize(196,128);

        QHBoxLayout layout;
        layout.setMargin(7);

        QGroupBox groupbox(tr("Adjust to appropriate strobe:"));
        QHBoxLayout l_groupbox;
        l_groupbox.setMargin(7);

        QLabel label;
        label.setFont(QFont("Tahoma", 24.0, QFont::DemiBold));
        label.setAlignment(Qt::AlignCenter);
        QDial dial;
        dial.setNotchesVisible(true);
        dial.setWrapping(false);
        dial.setMinimum(1);
        dial.setMaximum(128);
        dial.setSingleStep(1);
        connect(&dial, SIGNAL(valueChanged(int)), &label, SLOT(setNum(int)));
        dial.setValue(pt_harmonicProcessor->getStrobe());
        label.setNum(pt_harmonicProcessor->getStrobe());
        connect(&dial, &QDial::valueChanged, pt_harmonicProcessor, &QHarmonicProcessor::setStrobe);

        l_groupbox.addWidget(&dial);
        l_groupbox.addWidget(&label);

        groupbox.setLayout(&l_groupbox);
        layout.addWidget(&groupbox);

        dialog.setLayout(&layout);
        dialog.exec();
    }
    else
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not open before start"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
}
