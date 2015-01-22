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
    pt_signalPlot = new QEasyPlot(NULL, tr("Counts"), tr("Amplitude, g.e."));
    pt_signalPlot->set_horizontal_Borders(0,100);
    pt_signalPlot->set_vertical_Borders(0,16);
    pt_signalPlot->set_X_Ticks(11);
    pt_signalPlot->set_Y_Ticks(9);
    pt_signalPlot->set_coordinatesPrecision(0,0);
    pt_centralLayout->addWidget(pt_signalPlot);

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
    closeSerialConnection();

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
    connect(pt_aboutAction, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

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
    pt_serialProcessor = new QSerialProcessor(this);

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
    closeSerialConnection();

    if(pt_serialProcessor->showPortSelectDialog())
    {
        if(m_transmissionDialog.exec() == QDialog::Accepted)
        {
            quint16 tempLength = m_transmissionDialog.getOveralTime()/m_transmissionDialog.getDiscretizationPeriod();
            QSerialProcessor::BytesPerValue bytesNumber = QSerialProcessor::One;
            if(m_transmissionDialog.getBitsNumber() > 8)
                bytesNumber = QSerialProcessor::Two;

            pt_serialProcessor->initializeBuffer(tempLength, bytesNumber, m_transmissionDialog.getBitsOrder());
            pt_signalPlot->set_vertical_Borders(0.0, (qreal)(0x00001 << m_transmissionDialog.getBitsNumber()));


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
                connect(pt_serialProcessor, SIGNAL(dataUpdated(const quint16*,quint16)), pt_harmonicProcessor, SLOT(readData(const quint16*,quint16)));
                connect(pt_harmonicProcessor, SIGNAL(dataUpdated(const qreal*,quint16)), pt_signalPlot, SLOT(set_externalArray(const qreal*,quint16)));
                pt_harmonicThread->start();
            }
            m_timer.start();
        }
    }
}

void MainWindow::closeSerialConnection()
{
    m_timer.stop();
    pt_serialProcessor->close();
}

void MainWindow::startRecord()
{
    if(!m_outputfile.isOpen())
    {
        QString str = QFileDialog::getSaveFileName(this, tr("Save file"), "/records/record.txt", tr("Text file(*.txt)"));
        m_outputfile.setFileName(str);
        while(!m_outputfile.open(QFile::WriteOnly))
        {
            QMessageBox msg(QMessageBox::Warning, tr("Warning message"), tr("Can not save"), QMessageBox::Open | QMessageBox::Close, this);
            if(msg.exec() == QMessageBox::Open)
            {
                str = QFileDialog::getSaveFileName(this, tr("Save file"), "/records/record.txt", tr("Text file(*.txt)"));
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
                         << "discretization period: " << QString::number(m_transmissionDialog.getDiscretizationPeriod(), 'f', 3)
                         << " ms , gen.unit: 5.0 / " << QString::number( (0x00001 << m_transmissionDialog.getBitsNumber()))
                         << " V\nValue, gen.unit\n";
            connect(pt_serialProcessor, SIGNAL(dataUpdated(const quint16*,quint16)), this, SLOT(makeRecord(const quint16*,quint16)));
            pt_recordAction->setChecked(true);
        }
    }
    else
    {
        m_outputfile.close();
        disconnect(pt_serialProcessor, SIGNAL(dataUpdated(const quint16*,quint16)), this, SLOT(makeRecord(const quint16*,quint16)));
        pt_recordAction->setChecked(false);
    }
}

void MainWindow::makeRecord(const quint16*pointer, quint16 length)
{
   if(m_outputfile.isOpen())
   {
       for(quint16 i = 0; i < length; i++)
           m_textstream << pointer[i] << "\n";
   }
}

void MainWindow::adjustStrobe()
{
    if(pt_harmonicProcessor)
    {
        QDialog dialog;
        dialog.setWindowTitle(tr("Strobe dialog"));
        dialog.setFixedSize(196,128);

        QHBoxLayout layout;
        layout.setMargin(7);

        QGroupBox groupbox(tr("Adjust strobe factor:"));
        QHBoxLayout l_groupbox;
        l_groupbox.setMargin(7);

        QLabel label;
        label.setFont(QFont("Tahoma", 16.0, QFont::DemiBold));
        label.setAlignment(Qt::AlignCenter);
        label.setNum(pt_harmonicProcessor->getStrobe());
        label.setFrameStyle(QLabel::Sunken | QLabel::Box);
        QDial dial;
        dial.setNotchesVisible(true);
        dial.setWrapping(false);
        dial.setMinimum(1);
        dial.setMaximum(256);
        dial.setSingleStep(1);
        dial.setFixedSize(64,64);
        //dial.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        dial.setValue(pt_harmonicProcessor->getStrobe());
        connect(&dial, SIGNAL(valueChanged(int)), &label, SLOT(setNum(int)));            
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
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Nothing to strobe"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
}
