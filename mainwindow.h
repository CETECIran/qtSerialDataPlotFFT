#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <qcustomplot.h>
#include "qextserialport.h"
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include <QDebug>
#include <QList>
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>
#include <QDialog>
#include "qledindicator.h"
#include <complex>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    typedef std::complex<double> Complex;

    void setupRealtimeRawDataPlot(QCustomPlot *customPlot);
    void setupFFTDataPlot(QCustomPlot *customPlot,double data);
    void setupStyledDemo(QCustomPlot *customPlot);
    void setupAdvancedAxesDemo(QCustomPlot *customPlot);
    void setupColorMapDemo(QCustomPlot *customPlot);
    void ParseSignalFile();

    void setupPlayground(QCustomPlot *customPlot);
    void init_port(); // port initialisation function
    void ReadFromBoard();
    void SendCommandToBoard(QString Data);
    void listSerialPorts();
    int FFTmain();
    void shiftData(double *data, int len, double newValue);
    void logData();

    //fft function declration
    void sdft_init_coeffs();
    void sdft_init();
    void sdft_add_data();
    void sdft();
    void isdft();
    void ft();
    double mag(Complex& c);
    void powr_spectrum(double *powr);


    void onReceiveMsg();
protected:
    void changeEvent(QEvent *e);

private slots:
    void realtimeDataSlot();
//  void screenShot();
//  void allScreenShots();
//  void transmitCmd(int value); // sending function
    void on_realtimeBtn_clicked();
    //void onReadyReadFromBoard();




private Q_SLOTS :
    void on_connectOrDisconnectBtn_clicked();
    void onPortNameChanged(const QString &name);
    void onBaudRateChanged(int idx);
    void onParityChanged(int idx);
    void onDataBitsChanged(int idx);
    void onStopBitsChanged(int idx);
    void onQueryModeChanged(int idx);
    void onTimeoutChanged(int val);
    void onReadyRead();
    void onPortAddedOrRemoved();
    void on_stopBtn_clicked();
    void on_commandLinkBtnClearConsole_clicked();
    void on_toolButton_browse_clicked();
    void radioBtnbox_randData();
    void radioBtnbox_offline();

    void on_SendToSerialPort_clicked();

private:

    Ui::MainWindow *ui;
    QString demoName;
    QTimer dataTimer;
    QTimer dataTime_fft;
    QCPItemTracer *itemDemoPhaseTracer;
    int currentDemoIndex;
    QextSerialPort *port;
    QTimer *timer;
    QextSerialEnumerator *enumerator;
    double globalSerialDataValue =0;
    int prevTempAdc;
    int currentTempAdc;
    QLedIndicator *LED0;
    QLedIndicator *LED1;
    QLedIndicator *LED2;
    QFile logFile;
    QTextStream logSerialDataStream;
    QList <double> serialDataList;

    //FFT Data Source
    static const int N = 512;
    // input signal
    int in[N];

    // frequencies of input signal after ft
    // Size increased by one because the optimized sdft code writes data to freqs[N]
    Complex freqs[N+1];

    // output signal after inverse ft of freqs
    double out1[N];
    double out2[N];

    // forward coeffs -2 PI e^iw -- normalized (divided by N)
    Complex coeffs[N];
    // inverse coeffs 2 PI e^iw
    Complex icoeffs[N];

    // global index for input and output signals
    int idx;

    int oldest_data, newest_data;
    int globalSerialDataCounter;
};

#endif // MAINWINDOW_H
