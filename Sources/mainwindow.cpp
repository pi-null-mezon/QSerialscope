#include "mainwindow.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 576

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    this->setWindowTitle(APP_NAME);
    this->setMinimumSize(320, 240);

    //Central widget
    pt_centralWidget = new QWidget();
    this->setCentralWidget( pt_centralWidget );
    pt_centralLayout = new QHBoxLayout();
    pt_centralLayout->setMargin(5);
    pt_centralWidget->setLayout(pt_centralLayout);

    //Plot widges
    pt_signalPlot = new QEasyPlot(NULL, tr("Count index"), tr("Amplitude"));
    pt_signalPlot->set_horizontal_Borders(0,100);
    pt_signalPlot->set_vertical_Borders(0,16);
    pt_signalPlot->set_X_Ticks(11);
    pt_signalPlot->set_Y_Ticks(9);
    pt_signalPlot->set_coordinatesPrecision(0,0);

    pt_secondSignalPlot = new QEasyPlot(NULL, tr("Count index"), tr("Amplitude"));
    pt_secondSignalPlot->set_tracePen(QPen(QBrush(Qt::NoBrush),1.0), QColor(255,0,0));
    pt_secondSignalPlot->set_horizontal_Borders(0,100);
    pt_secondSignalPlot->set_vertical_Borders(0,16);
    pt_secondSignalPlot->set_X_Ticks(11);
    pt_secondSignalPlot->set_Y_Ticks(9);
    pt_secondSignalPlot->set_coordinatesPrecision(0,0);

    pt_centralLayout->addWidget(pt_signalPlot);
    pt_centralLayout->addWidget(pt_secondSignalPlot);

    _createSpectrumPlot();
    _createCNSignalPlot();

    //Initialization
    _createActions();
    _createMenus();
    _createThreads();


    //Resize
    this->resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    statusBar()->addWidget(&m_infoLabel);
    this->statusBar()->showMessage(tr("Context menu available on right click"));
}

MainWindow::~MainWindow()
{
    closeSerialConnection();

    if(pt_firstHarmonicProcessor)
    {
        pt_harmonicThread->quit();
        pt_harmonicThread->wait();
    }

    delete pt_spectrumPlot;
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
    pt_strobeAction->setStatusTip(tr("Change discretization period"));
    pt_strobeAction->setShortcut(QKeySequence(tr("Alt+A")));
    connect(pt_strobeAction, SIGNAL(triggered()), this, SLOT(adjustStrobe()));

    pt_backgroundColorAct = new QAction(tr("&Background"), this);
    pt_backgroundColorAct->setStatusTip(tr("Select color for background"));
    connect(pt_backgroundColorAct, SIGNAL(triggered()), pt_signalPlot, SLOT(open_backgroundColorDialog()));

    pt_traceColorAct = new QAction(tr("&Trace"), this);
    pt_traceColorAct->setStatusTip(tr("Select color for trace"));
    connect(pt_traceColorAct, SIGNAL(triggered()), pt_signalPlot, SLOT(open_traceColorDialog()));

    pt_fontAct = new QAction(tr("&Font"), this);
    pt_fontAct->setStatusTip(tr("Select font"));
    connect(pt_fontAct, SIGNAL(triggered()), pt_signalPlot, SLOT(open_fontSelectDialog()));

    pt_coordinateColorAct = new QAction(tr("&Coordinates"), this);
    pt_coordinateColorAct->setStatusTip(tr("Select color for coordinate axis"));
    connect(pt_coordinateColorAct, SIGNAL(triggered()), pt_signalPlot, SLOT(open_coordinatesystemColorDialog()));

    pt_spectrumAct = new QAction(tr("&Spectrum"), this);
    pt_spectrumAct->setStatusTip(tr("Show power spectrum in new window"));
    connect(pt_spectrumAct, SIGNAL(triggered()), pt_spectrumPlot, SLOT(show()));
    pt_cnsignalAct = new QAction(tr("&CNsignal"), this);
    pt_cnsignalAct->setStatusTip(tr("Show centered and normalized signal in new window"));
    connect(pt_cnsignalAct, SIGNAL(triggered()), pt_cnsignalPlot, SLOT(show()));

    pt_timerAct = new QAction(tr("&Timer"), this);
    pt_timerAct->setStatusTip(tr("Adjust time interval between successive spectrum evaluations"));
    pt_timerAct->setShortcut(QKeySequence(tr("Alt+T")));
    connect(pt_timerAct, SIGNAL(triggered()), this, SLOT(adjustTimer()));

}

void MainWindow::_createMenus()
{
    pt_controlMenu = this->menuBar()->addMenu(tr("&Controls"));
    pt_controlMenu->addAction(pt_startAction);
    pt_controlMenu->addAction(pt_stopAction);
    pt_controlMenu->addSeparator();
    pt_controlMenu->addAction(pt_recordAction);
    pt_controlMenu->addAction(pt_strobeAction);
    pt_controlMenu->addAction(pt_timerAct);
    pt_controlMenu->addSeparator();
    pt_controlMenu->addAction(pt_exitAction);

    /*pt_appearenceMenu = this->menuBar()->addMenu(tr("&Appearence"));
    pt_appearenceMenu->addAction(pt_backgroundColorAct);
    pt_appearenceMenu->addAction(pt_coordinateColorAct);
    pt_appearenceMenu->addAction(pt_traceColorAct);
    pt_appearenceMenu->addAction(pt_fontAct);

    pt_windowsMenu = this->menuBar()->addMenu(tr("&Plots"));
    pt_windowsMenu->addAction(pt_cnsignalAct);
    pt_windowsMenu->addAction(pt_spectrumAct);*/


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
    pt_firstHarmonicProcessor = NULL;
    pt_harmonicThread = new QThread(this);
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(pt_startAction);
    menu.addAction(pt_stopAction);
    menu.addSeparator();
    menu.addAction(pt_strobeAction);
   // menu.addAction(pt_timerAct);
    menu.exec(event->globalPos());
}

void MainWindow::showAboutDialog()
{
    QDialog *aboutDialog = new QDialog();
    aboutDialog->setWindowTitle("About " + QString(APP_NAME));
    aboutDialog->setFixedSize(256,210);

    QVBoxLayout *tempLayout = new QVBoxLayout();
    tempLayout->setMargin(5);

    QLabel *projectnameLabel = new QLabel(QString(APP_NAME) + "\t" + QString(APP_VERSION));
    projectnameLabel->setFrameStyle(QFrame::Box | QFrame::Raised);
    projectnameLabel->setAlignment(Qt::AlignCenter);
    QLabel *projectauthorsLabel = new QLabel(QString(APP_AUTHOR) + "\n" + QString(APP_COMPANY) + "\n" + QString::number(APP_YEAR));
    projectauthorsLabel->setAlignment(Qt::AlignCenter);
    QLabel *hyperlinkLabel = new QLabel(QString(APP_WEB));
    hyperlinkLabel->setToolTip(tr("Send an email"));
    hyperlinkLabel->setOpenExternalLinks(true);
    hyperlinkLabel->setAlignment(Qt::AlignCenter);

    QLabel *serialLabel = new QLabel("Expected serial connection settings:\n- BAUD RATE 115200\n- 8 DATA BITS\n- ONE STOP BIT\n- NO FLOW CONTROL\n- NO PARITY");
    serialLabel->setFrameStyle(QFrame::Sunken | QFrame::Box);
    serialLabel->setMargin(6);

    tempLayout->addWidget(projectnameLabel);
    tempLayout->addWidget(projectauthorsLabel);
    tempLayout->addWidget(serialLabel);
    tempLayout->addWidget(hyperlinkLabel);


    aboutDialog->setLayout(tempLayout);
    aboutDialog->exec();

    delete hyperlinkLabel;
    delete projectauthorsLabel;
    delete projectnameLabel;
    delete serialLabel;
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
            QSerialProcessor::BytesPerValue bytesNumber;
            if(m_transmissionDialog.getBitsNumber() > 8)
                bytesNumber = QSerialProcessor::Two;
            else
                bytesNumber = QSerialProcessor::One;

            pt_serialProcessor->setDataFormat(bytesNumber, m_transmissionDialog.getBitsOrder());
            pt_signalPlot->set_vertical_Borders(0.0, (qreal)(0x00001 << m_transmissionDialog.getBitsNumber()));
            pt_secondSignalPlot->set_vertical_Borders(0.0, (qreal)(0x00001 << m_transmissionDialog.getBitsNumber()));

            this->updateAxis(MIN_STROBE);

            if(pt_serialProcessor->open())
            {
                if(pt_firstHarmonicProcessor)
                {
                    pt_harmonicThread->quit();
                    pt_harmonicThread->wait();
                }

                pt_firstHarmonicProcessor = new QHarmonicProcessor();
                pt_firstHarmonicProcessor->moveToThread(pt_harmonicThread);
                pt_firstHarmonicProcessor->setDiscretizationPeriod(m_transmissionDialog.getDiscretizationPeriod());
                connect(pt_harmonicThread, SIGNAL(finished()), pt_firstHarmonicProcessor, SLOT(deleteLater()));
                connect(pt_serialProcessor, SIGNAL(dataUpdated(const quint16*,quint16)), pt_firstHarmonicProcessor, SLOT(readData(const quint16*,quint16)));
                connect(pt_firstHarmonicProcessor, SIGNAL(dataUpdated(const qreal*,quint16)), pt_signalPlot, SLOT(set_externalArray(const qreal*,quint16)));
                //connect(&m_timer, SIGNAL(timeout()), pt_firstHarmonicProcessor, SLOT(computeFrequency()));
                connect(pt_firstHarmonicProcessor, SIGNAL(frequencyUpdated(qreal,qreal)), this, SLOT(frequencyInStatusBar(qreal,qreal)));
                connect(pt_firstHarmonicProcessor, SIGNAL(tooNoisy(qreal)), this, SLOT(warningInStatusBar(qreal)));
                connect(pt_firstHarmonicProcessor, SIGNAL(spectrumUpdated(const qreal*,quint16)), pt_spectrumPlot, SLOT(set_externalArray(const qreal*,quint16)));
                connect(pt_firstHarmonicProcessor, SIGNAL(signalUpdated(const qreal*,quint16)), pt_cnsignalPlot, SLOT(set_externalArray(const qreal*,quint16)));

                if(m_transmissionDialog.isTwoSignals())
                {
                    pt_serialProcessor->setDataFormat(QSerialProcessor::Two8bitSignals, (QSerialProcessor::BitsOrder)0);
                    pt_secondHarmonicProcessor = new QHarmonicProcessor();
                    pt_secondHarmonicProcessor->moveToThread(pt_harmonicThread);
                    pt_secondHarmonicProcessor->setDiscretizationPeriod(m_transmissionDialog.getDiscretizationPeriod());
                    connect(pt_harmonicThread, SIGNAL(finished()), pt_secondHarmonicProcessor, SLOT(deleteLater()));
                    connect(pt_serialProcessor, SIGNAL(dataUpdatedSecondChanell(const quint16*,quint16)), pt_secondHarmonicProcessor, SLOT(readData(const quint16*,quint16)));
                    connect(pt_secondHarmonicProcessor, SIGNAL(dataUpdated(const qreal*,quint16)), pt_secondSignalPlot, SLOT(set_externalArray(const qreal*,quint16)));
                    //connect(&m_timer, SIGNAL(timeout()), pt_secondHarmonicProcessor, SLOT(computeFrequency()));
                }

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
            if(m_transmissionDialog.isTwoSignals())
            {
                m_textstream << QString(APP_NAME) + " output record\n"
                             << "Record was started at " + QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss") + "\n"
                             << "discretization period: " << QString::number(m_transmissionDialog.getDiscretizationPeriod(), 'f', 3) // strobe does not effwct on this, this is raw data
                             << " ms , gen.unit: " << QString::number(m_transmissionDialog.getReferenceVoltage(), 'f', 2)
                             << "/" << QString::number( (0x00001 << m_transmissionDialog.getBitsNumber()))
                             << " V\nSignal1, gen.unit\tSignal2, gen.unit\n";
                connect(pt_serialProcessor, SIGNAL(signalsUpdated(const quint16*,const quint16*,quint16)), this, SLOT(makeRecord(const quint16*, const quint16*,quint16)));
            }
            else
            {
                m_textstream << QString(APP_NAME) + " output record\n"
                             << "Record was started at " + QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss") + "\n"
                             << "discretization period: " << QString::number(m_transmissionDialog.getDiscretizationPeriod(), 'f', 3) // strobe does not effwct on this, this is raw data
                             << " ms , gen.unit: " << QString::number(m_transmissionDialog.getReferenceVoltage(), 'f', 2)
                             << "/" << QString::number( (0x00001 << m_transmissionDialog.getBitsNumber()))
                             << " V\nSignal, gen.unit\n";
                connect(pt_serialProcessor, SIGNAL(dataUpdated(const quint16*,quint16)), this, SLOT(makeRecord(const quint16*,quint16)));
            }
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

void MainWindow::makeRecord(const quint16 *pointer1, const quint16 *pointer2, quint16 length)
{
    if(m_outputfile.isOpen())
    {
        for(quint16 i = 0; i < length; i++)
            m_textstream << pointer1[i] << "\t" << pointer2[i] <<  "\n";
    }
}

void MainWindow::adjustStrobe()
{
    if(pt_firstHarmonicProcessor)
    {
        QDialog dialog;
        dialog.setWindowTitle(tr("Strobe"));
        dialog.setFixedSize(196,100);

        QHBoxLayout layout;
        layout.setMargin(7);

        QGroupBox groupbox(tr("Adjust strobe factor:"));
        QHBoxLayout l_groupbox;
        l_groupbox.setMargin(7);

        QLabel label;
        label.setFont(QFont("MS Shell Dlg", 14.0, QFont::DemiBold));
        label.setAlignment(Qt::AlignCenter);
        label.setNum(pt_firstHarmonicProcessor->getStrobe());
        label.setFrameStyle(QLabel::Sunken | QLabel::Box);
        QDial dial;
        dial.setNotchesVisible(true);
        dial.setWrapping(false);
        dial.setMinimum(MIN_STROBE);
        dial.setMaximum(MAX_STROBE);
        dial.setSingleStep(1);
        dial.setFixedSize(64,64);
        //dial.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        dial.setValue(pt_firstHarmonicProcessor->getStrobe());
        connect(&dial, SIGNAL(valueChanged(int)), &label, SLOT(setNum(int)));            
        connect(&dial, &QDial::valueChanged, pt_firstHarmonicProcessor, &QHarmonicProcessor::setStrobe);
        connect(&dial, &QDial::valueChanged, pt_secondHarmonicProcessor, &QHarmonicProcessor::setStrobe);
        connect(&dial, SIGNAL(valueChanged(int)), this, SLOT(updateAxis(int)));

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

void MainWindow::frequencyInStatusBar(qreal freq, qreal snr)
{
    m_infoLabel.setText("Pulse harmonic frequency: " + QString::number(qRound(freq)) + " bpm, snr: " + QString::number(snr, 'f', 2) + " db");
}

void MainWindow::warningInStatusBar(qreal snr)
{
    m_infoLabel.setText("Too noisy or more than one dominate harmonic, snr: " + QString::number(snr, 'f', 2) + " db");
}

void MainWindow::_createSpectrumPlot()
{
    pt_spectrumPlot = new QEasyPlot(NULL,tr("Harmonic count"), tr("Normalized harmonic power, g.e."));
    pt_spectrumPlot->setWindowFlags(Qt::Window);
    pt_spectrumPlot->setContextMenuPolicy(Qt::ActionsContextMenu);
    pt_spectrumPlot->set_coordinatesPrecision(0,3);
    pt_spectrumPlot->set_vertical_Borders(0.0,1.0);
    pt_spectrumPlot->set_horizontal_Borders(0, 100);
    pt_spectrumPlot->set_DrawRegime(QEasyPlot::FilledTraceRegime);
    pt_spectrumPlot->set_tracePen(QPen(QBrush(Qt::Dense5Pattern),1.0), QColor(255,0,0));

    //----------------------------------Actions----------------------------------
    QAction *pt_spBackgroundAct = new QAction(tr("&BG_color"),pt_spectrumPlot);
    connect(pt_spBackgroundAct,SIGNAL(triggered()), pt_spectrumPlot, SLOT(open_backgroundColorDialog()));
    QAction *pt_spCoordinateColorAct = new QAction(tr("&CS_color"),pt_spectrumPlot);
    connect(pt_spCoordinateColorAct,SIGNAL(triggered()), pt_spectrumPlot, SLOT(open_coordinatesystemColorDialog()));
    QAction *pt_spTraceColorAct = new QAction(tr("&TC_color"),pt_spectrumPlot);
    connect(pt_spTraceColorAct,SIGNAL(triggered()), pt_spectrumPlot, SLOT(open_traceColorDialog()));
    QAction *pt_spFontAct = new QAction(tr("&CS_font"),pt_spectrumPlot);
    connect(pt_spFontAct,SIGNAL(triggered()), pt_spectrumPlot, SLOT(open_fontSelectDialog()));
    pt_spectrumPlot->addAction(pt_spBackgroundAct);
    pt_spectrumPlot->addAction(pt_spCoordinateColorAct);
    pt_spectrumPlot->addAction(pt_spTraceColorAct);
    pt_spectrumPlot->addAction(pt_spFontAct);
    //----------------------------------------------------------------------------
}

void MainWindow::_createCNSignalPlot()
{
    pt_cnsignalPlot = new QEasyPlot(NULL,tr("Count index"), tr("Centered & normalized signal"));
    pt_cnsignalPlot->setWindowFlags(Qt::Window);
    pt_cnsignalPlot->setContextMenuPolicy(Qt::ActionsContextMenu);
    pt_cnsignalPlot->set_coordinatesPrecision(0,1);
    pt_cnsignalPlot->set_vertical_Borders(5.0,-5.0);
    pt_cnsignalPlot->set_horizontal_Borders(0, 100);
    pt_cnsignalPlot->set_tracePen(QPen(QBrush(Qt::NoBrush),1.0), QColor(255,255,255));

    //----------------------------------Actions----------------------------------
    QAction *pt_spBackgroundAct = new QAction(tr("&BG_color"),pt_cnsignalPlot);
    connect(pt_spBackgroundAct,SIGNAL(triggered()), pt_spectrumPlot, SLOT(open_backgroundColorDialog()));
    QAction *pt_spCoordinateColorAct = new QAction(tr("&CS_color"),pt_cnsignalPlot);
    connect(pt_spCoordinateColorAct,SIGNAL(triggered()), pt_cnsignalPlot, SLOT(open_coordinatesystemColorDialog()));
    QAction *pt_spTraceColorAct = new QAction(tr("&TC_color"),pt_cnsignalPlot);
    connect(pt_spTraceColorAct,SIGNAL(triggered()), pt_cnsignalPlot, SLOT(open_traceColorDialog()));
    QAction *pt_spFontAct = new QAction(tr("&CS_font"),pt_cnsignalPlot);
    connect(pt_spFontAct,SIGNAL(triggered()), pt_cnsignalPlot, SLOT(open_fontSelectDialog()));
    pt_cnsignalPlot->addAction(pt_spBackgroundAct);
    pt_cnsignalPlot->addAction(pt_spCoordinateColorAct);
    pt_cnsignalPlot->addAction(pt_spTraceColorAct);
    pt_cnsignalPlot->addAction(pt_spFontAct);
    //----------------------------------------------------------------------------
}

void MainWindow::adjustTimer()
{
    QDialog dialog;
    dialog.setWindowTitle(tr("Timer"));
    dialog.setFixedSize(196,100);

    QHBoxLayout layout;
    layout.setMargin(7);

    QGroupBox groupbox(tr("Adjust spectrum timer, ms:"));
    QHBoxLayout l_groupbox;
    l_groupbox.setMargin(7);

    QLabel label;
    label.setFont(QFont("MS Shell Dlg", 14.0, QFont::DemiBold));
    label.setAlignment(Qt::AlignCenter);
    label.setNum(m_timer.interval());
    label.setFrameStyle(QLabel::Sunken | QLabel::Box);
    QDial dial;
    dial.setNotchesVisible(true);
    dial.setWrapping(false);
    dial.setMinimum(100);
    dial.setMaximum(10000);
    dial.setPageStep(100);
    dial.setSingleStep(10);
    dial.setFixedSize(64,64);
    //dial.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    dial.setValue(m_timer.interval());
    connect(&dial, SIGNAL(valueChanged(int)), &label, SLOT(setNum(int)));
    connect(&dial, &QDial::valueChanged, &m_timer, &QTimer::setInterval);

    l_groupbox.addWidget(&label);
    l_groupbox.addWidget(&dial);

    groupbox.setLayout(&l_groupbox);
    layout.addWidget(&groupbox);

    dialog.setLayout(&layout);
    dialog.exec();
}

void MainWindow::closeEvent(QCloseEvent */*event*/)
{
    pt_cnsignalPlot->close();
    pt_spectrumPlot->close();
}

void MainWindow::updateAxis(int strobe_value)
{
    pt_signalPlot->set_axis_names("Count, " + QString::number( 1000 * strobe_value * m_transmissionDialog.getDiscretizationPeriod(), 'f', 1 ) + " us per count",
                                  "Voltage, " + QString::number( 1000 * m_transmissionDialog.getReferenceVoltage()/(0x00001 << m_transmissionDialog.getBitsNumber()),'f', 3) + " mV per unit");
}
