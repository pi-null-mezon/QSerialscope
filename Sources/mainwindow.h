#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QContextMenuEvent>
#include <QDialog>
#include <QString>
#include <QTimer>
#include <QIcon>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QLineEdit>
#include <QDial>
#include <QMessageBox>
#include <QThread>

#include "about.h"
#include "qeasyplot.h"
#include "qserialprocessor.h"
#include "qharmonicprocessor.h"
#include "qtransmissiondialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void showAboutDialog();         // Show about information as dialog
    void openSerialConnection();    // Open a new one serial connection
    void closeSerialConnection();   // Close previous serial connection
    void startRecord();             // Open dialog to choose output file name
    void makeRecord(const quint16 *pointer, quint16 length); // Make record to output file
    void adjustStrobe();
    void frequencyInStatusBar(qreal freq, qreal snr);
    void warningInStatusBar(qreal snr);
    void adjustTimer();
    void updateAxis(int strobe_value);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    QWidget *pt_centralWidget;
    QHBoxLayout *pt_centralLayout;
    QEasyPlot *pt_signalPlot;
    QAction *pt_startAction;
    QAction *pt_stopAction;
    QAction *pt_aboutAction;
    QAction *pt_exitAction;
    QAction *pt_recordAction;
    QAction *pt_strobeAction;
    QAction *pt_timerAct;
    QMenu *pt_controlMenu;
    QMenu *pt_helpMenu;
    QTimer m_timer;
    QTextStream m_textstream;
    QFile m_outputfile; 
    QSerialProcessor *pt_serialProcessor;
    QHarmonicProcessor *pt_harmonicProcessor;
    QThread *pt_harmonicThread;
    QTransmissionDialog m_transmissionDialog;
    QLabel m_infoLabel;

    QEasyPlot *pt_spectrumPlot;
    QAction *pt_spectrumAct;
    QMenu *pt_windowsMenu;
    QEasyPlot *pt_cnsignalPlot;
    QAction *pt_cnsignalAct;


    QAction *pt_backgroundColorAct;
    QAction *pt_traceColorAct;
    QAction *pt_fontAct;
    QAction *pt_coordinateColorAct;
    QMenu *pt_appearenceMenu;

    void _createActions();
    void _createMenus();
    void _createThreads();
    void _createSpectrumPlot();
    void _createCNSignalPlot();

};

#endif // MAINWINDOW_H
