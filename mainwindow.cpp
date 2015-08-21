#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include <stdio.h>
#include <unistd.h>
#include "print.h"
#include <QButtonGroup>
#include <QTimer>
#include <QtCore>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    globalSerialDataCounter=0;
    sdft_init();
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        ui->portComboBox->addItem(info.portName);
    ui->portComboBox->setCurrentIndex(4);

    //make sure user can input their own port name!
    ui->portComboBox->setEditable(true);

    ui->baudRateComboBox->addItem("1200", BAUD1200);
    ui->baudRateComboBox->addItem("2400", BAUD2400);
    ui->baudRateComboBox->addItem("4800", BAUD4800);
    ui->baudRateComboBox->addItem("9600", BAUD9600);
    ui->baudRateComboBox->addItem("19200",BAUD19200);
    ui->baudRateComboBox->addItem("57600",BAUD57600);
    ui->baudRateComboBox->addItem("115200",BAUD115200);
    ui->baudRateComboBox->addItem("1500000",BAUD1500000);
    ui->baudRateComboBox->setCurrentIndex(6);

    ui->parityComboBox->addItem("NONE", PAR_NONE);
    ui->parityComboBox->addItem("ODD", PAR_ODD);
    ui->parityComboBox->addItem("EVEN", PAR_EVEN);

    ui->flowComboBox->addItem("OFF");
    ui->flowComboBox->addItem("Xon" );
    ui->flowComboBox->addItem("Xoff");
    ui->flowComboBox->setCurrentIndex(0);

    ui->dataBitComboBox->addItem("5", DATA_5);
    ui->dataBitComboBox->addItem("6", DATA_6);
    ui->dataBitComboBox->addItem("7", DATA_7);
    ui->dataBitComboBox->addItem("8", DATA_8);
    ui->dataBitComboBox->setCurrentIndex(3);

    ui->StopbitComboBox->addItem("1", STOP_1);
    ui->StopbitComboBox->addItem("2", STOP_2);

    ui->queryModeComboBox->addItem("Polling", QextSerialPort::Polling);
    ui->queryModeComboBox->addItem("EventDriven", QextSerialPort::EventDriven);

    //ui->sendcommandBox->setEnabled(false);
    ui->tabWidget->setCurrentIndex(2);

    timer = new QTimer(this);
    timer->setInterval(5);

    enumerator = new QextSerialEnumerator(this);
    enumerator->setUpNotifications();


    PortSettings settings = {BAUD9600, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    port = new QextSerialPort(ui->portComboBox->currentText(), settings, QextSerialPort::Polling);


    ui->groupBox_command->setEnabled(false);

//    ui->board1Btn->setChecked(true);

    connect(ui->baudRateComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onBaudRateChanged(int)));
    connect(ui->parityComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onParityChanged(int)));
    connect(ui->dataBitComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onDataBitsChanged(int)));
    connect(ui->StopbitComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onStopBitsChanged(int)));
    connect(ui->queryModeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onQueryModeChanged(int)));
    connect(ui->timeoutComboBox, SIGNAL(valueChanged(int)), SLOT(onTimeoutChanged(int)));
    connect(ui->portComboBox, SIGNAL(editTextChanged(QString)), SLOT(onPortNameChanged(QString)));

    connect(timer, SIGNAL(timeout()), SLOT(onReadyRead()));
    connect(port, SIGNAL(readyRead()), SLOT(onReadyRead()));

    connect(enumerator, SIGNAL(deviceDiscovered(QextPortInfo)), SLOT(onPortAddedOrRemoved()));
    connect(enumerator, SIGNAL(deviceRemoved(QextPortInfo)), SLOT(onPortAddedOrRemoved()));

//    connect(ui->logCheckBox, SIGNAL(clicked()),SLOT(fileLogger()));
    connect(ui->radioButton_randData,SIGNAL(toggled(bool)),this,SLOT(radioBtnbox_randData()));
    connect(ui->radioButton_offline,SIGNAL(toggled(bool)),this,SLOT(radioBtnbox_offline()));
    connect(ui->radioButton_online,SIGNAL(toggled(bool)) ,this,SLOT(radioBtnbox_offline()));

}

void MainWindow::radioBtnbox_randData()
{
    if (ui->radioButton_randData->isChecked())
    {
        ui->groupBox_hardware->setEnabled(false);
        ui->groupBox_command->setEnabled(false);
    }

}

void MainWindow::radioBtnbox_offline()
{
    if (ui->radioButton_offline->isChecked())
    {
        ui->groupBox_offline->setEnabled(true);
//        ui->groupBox_online->setEnabled(false);
        ui->groupBox_hardware->setEnabled(false);
        ui->groupBox_command->setEnabled(false);
    }
    else {
        ui->groupBox_offline->setEnabled(false);
//        ui->groupBox_online->setEnabled(true);
        ui->groupBox_hardware->setEnabled(true);
        ui->groupBox_command->setEnabled(true);
    }
}

void MainWindow::on_realtimeBtn_clicked()
{
    setupRealtimeRawDataPlot(ui->customPlot);
    ui->customPlot->replot();

}

void MainWindow::setupPlayground(QCustomPlot *customPlot)
{
  Q_UNUSED(customPlot)
}

MainWindow::~MainWindow()
{
   //port->close(); //we close the port at the end of the program

   //port =NULL;
   if (ui->logCheckBox->isChecked())
      logData();
   delete ui;
   delete port;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


//******************Signal Plot Segment*******************************

void MainWindow::setupRealtimeRawDataPlot(QCustomPlot *customPlot)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  QMessageBox::critical(this, "", "You're using Qt < 4.7, the realtime data demo needs functions that are available with Qt 4.7 to work properly");
#endif

  // include this section to fully disable antialiasing for higher performance:

  customPlot->setNotAntialiasedElements(QCP::aeAll);
  QFont font;
  font.setStyleStrategy(QFont::NoAntialias);
  customPlot->xAxis->setTickLabelFont(font);
  customPlot->yAxis->setTickLabelFont(font);
  customPlot->legend->setFont(font);

  customPlot->addGraph(); // blue line
  customPlot->graph(0)->setPen(QPen(Qt::blue));
  customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
  customPlot->graph(0)->setAntialiasedFill(false);
  customPlot->addGraph(); // red line
  customPlot->graph(1)->setPen(QPen(Qt::red));
  customPlot->graph(0)->setChannelFillGraph(customPlot->graph(1));

  customPlot->addGraph(); // blue dot
  customPlot->graph(2)->setPen(QPen(Qt::blue));
  customPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
  customPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
  customPlot->addGraph(); // red dot
  customPlot->graph(3)->setPen(QPen(Qt::red));
  customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
  customPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);

  customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
  customPlot->xAxis->setAutoTickStep(false);
  customPlot->xAxis->setTickStep(2);
  customPlot->axisRect()->setupFullAxesBox();

  // make left and bottom axes transfer their ranges to right and top axes:
  connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
  connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

  // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
  connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
  dataTimer.start(0); // Interval 0 means to refresh as fast as possible
  //QFuture<void> future;
  //future = QtConcurrent::run (this,&MainWindow::/*directPowerSsvepDetection*/ParseSignalFile);
}

void MainWindow::setupFFTDataPlot(QCustomPlot *customPlot,double data)
{

  // add two new graphs and set their look:
  customPlot->addGraph();
  customPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
  customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
  customPlot->addGraph();
  customPlot->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
  // generate some points of data (y0 for first, y1 for second graph):
  QVector<double> x(N), y0(N), y1(N);

  //****************FFT****************************

  oldest_data = in[idx];
  newest_data = in[idx] = data;// */ double(rand() / double(N));
  sdft();

  // Mess about with freqs[] here
  //isdft();

  if (++idx == N) idx = 0;

  double powr1[N/2];
//  powr_spectrum(powr1);


   // bump global index
  //*********************************************
  for (int i=0; i<N; i++)
  {
    x[i] = i;
    y0[i] =freqs[i].real()+20; 
  }
  // configure right and top axis to show ticks but no labels:
  // (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
  customPlot->xAxis2->setVisible(true);
  customPlot->xAxis2->setTickLabels(false);
  customPlot->yAxis2->setVisible(true);
  customPlot->yAxis2->setTickLabels(false);
  // make left and bottom axes always transfer their ranges to right and top axes:
  connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
  connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

  // pass data points to graphs:
  customPlot->graph(0)->setData(x, y0);
  customPlot->graph(1)->setData(x, y1);
  // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
  customPlot->graph(0)->rescaleAxes();
  // same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
  customPlot->graph(1)->rescaleAxes(true);
  // Note: we could have also just called customPlot->rescaleAxes(); instead
  // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
  customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

void MainWindow::realtimeDataSlot()
{
  // calculate two new data points:
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  double key = 0;
#else
  double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/500.0;
#endif

  //**********************************************************************
  static double lastPointKey = 0;
  double value0 = 0;
  double value1 = 0;
  if (key-lastPointKey > 0.0005) // at most add point every 10 ms
  {


    if  (ui->radioButton_randData->isChecked())
    {
        value0 = double(rand());
        serialDataList << value0;
    }
    else if (ui->radioButton_offline->isChecked() || ui->radioButton_online->isChecked())
        value0 = globalSerialDataValue;

    value1 = value0 ;

    // add data to lines:
    ui->customPlot->graph(0)->addData(key, value0);
    ui->customPlot->graph(1)->addData(key, value1);
    // set data of dots:
    ui->customPlot->graph(2)->clearData();
    ui->customPlot->graph(2)->addData(key, value0);
    ui->customPlot->graph(3)->clearData();
    ui->customPlot->graph(3)->addData(key, value1);
    // rescale value (vertical) axis to fit the current data:
    ui->customPlot->graph(0)->rescaleValueAxis(true);
    ui->customPlot->graph(1)->rescaleValueAxis(true);

    lastPointKey = key;
  }

  // make key axis range scroll with the data (at a constant range size of 8):
  ui->customPlot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
  ui->customPlot->replot();


  // calculate frames per second:
  static double lastFpsKey;
  static int frameCount;
  ++frameCount;
  if (key-lastFpsKey > 2) // average fps over 2 seconds
  {
//    ui->statusBar->showMessage(
//          QString("%1 FPS, Total Data points: %2")
//          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
//          .arg(ui->customPlot->graph(0)->data()->count()+ui->customPlot->graph(1)->data()->count())
//          , 0);
    lastFpsKey = key;
    frameCount = 0;
  }

  // CAll FFT Plot And Update Values
  //**************************************************************
  setupFFTDataPlot(ui->customPlot_FFT,value0);
  statusBar()->clearMessage();
  ui->customPlot_FFT->replot();
  //**************************************************************
}


//********************Serial Communication Segment Start ****************************
//*************************************************************************************

void MainWindow::SendCommandToBoard(QString Data)
{
  if (port->isOpen() && !ui->sendLineEdit->text().isEmpty())
        port->write(ui->sendLineEdit->text().toLatin1());
  port->flush();
}





//************************Serial port Handling Segment Begining********************
//*********************************************************************************


bool status = true;

void MainWindow::listSerialPorts()
{
    //! [1]
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    //! [1]
    qDebug() << "List of ports:";
    //! [2]
    foreach (QextPortInfo info, ports) {
        qDebug() << "port name:"       << info.portName;
        qDebug() << "friendly name:"   << info.friendName;
        qDebug() << "physical name:"   << info.physName;
        qDebug() << "enumerator name:" << info.enumName;
        qDebug() << "vendor ID:"       << info.vendorID;
        qDebug() << "product ID:"      << info.productID;

        qDebug() << "===================================";
    }
    //! [2]
}
void MainWindow::onPortNameChanged(const QString & /*name*/)
{
    if (port->isOpen()) {
        port->close();
        //ui->led->turnOff();
    }
}
//! [2]
void MainWindow::onBaudRateChanged(int idx)
{
    port->setBaudRate((BaudRateType)ui->baudRateComboBox->itemData(idx).toInt());
}

void MainWindow::onParityChanged(int idx)
{
    port->setParity((ParityType)ui->parityComboBox->itemData(idx).toInt());
}

void MainWindow::onDataBitsChanged(int idx)
{
    port->setDataBits((DataBitsType)ui->dataBitComboBox->itemData(idx).toInt());
}

void MainWindow::onStopBitsChanged(int idx)
{
    port->setStopBits((StopBitsType)ui->StopbitComboBox->itemData(idx).toInt());
}

void MainWindow::onQueryModeChanged(int idx)
{
    port->setQueryMode((QextSerialPort::QueryMode)ui->queryModeComboBox->itemData(idx).toInt());
}

void MainWindow::onTimeoutChanged(int val)
{
    port->setTimeout(val);
}
//! [2]
//! [3]
void MainWindow::on_connectOrDisconnectBtn_clicked()
{
//    timer = new QTimer(this);
//    timer->setInterval(250);

    if (!port->isOpen()) {
        port->setPortName(ui->portComboBox->currentText());
        port->open(QIODevice::ReadWrite);

        if (port->isOpen())
        {
        ui->connectOrDisconnectBtn->setText("Disconnect");
        //ui->groupBox_->setEnabled(false);
        ui->groupBox_command->setEnabled(true);
        ui->ledIndicator_0->toggle();
        ui->ledIndicator_1->toggle();
        //ui->sendcommandBox->setEnabled(true);
        }
    }
    else {
        port->close();
        //port->flush();

        ui->connectOrDisconnectBtn->setText("Connect To Board");
        //ui->groupBox_->setEnabled(true);
        ui->groupBox_command->setEnabled(false);
        ui->ledIndicator_0->toggle();
        ui->ledIndicator_1->toggle();
        ui->ledIndicator_2->toggle();
        //ui->sendcommandBox->setEnabled(false);

    }

    //If using polling mode, we need a QTimer
    if (port->isOpen() && port->queryMode() == QextSerialPort::Polling)
        timer->start();
    else
        timer->stop();

    //update led's status
    //ui->led->turnOn(port->isOpen());

    //ui->led->turnOn(port->isOpen());
}



void MainWindow::on_SendToSerialPort_clicked()
{
    if (port->isOpen() && !ui->sendLineEdit->text().isEmpty())
    {
        port->write(ui->sendLineEdit->text().toLatin1());
        ui->ledIndicator_2->toggle();
        //ui->ledIndicator_2->setChecked(true);
        ui->sendLineEdit->clear();
    }
    usleep(100000);
    ui->ledIndicator_2->toggle();
}


void MainWindow::onPortAddedOrRemoved()
{
    QString current = ui->portComboBox->currentText();

    ui->portComboBox->blockSignals(true);
    ui->portComboBox->clear();
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        ui->portComboBox->addItem(info.portName);

    ui->portComboBox->setCurrentIndex(ui->portComboBox->findText(current));

    ui->portComboBox->blockSignals(false);
}


void MainWindow::onReadyRead()
{


    if (port->bytesAvailable()) {

       ui->console->moveCursor(QTextCursor::End);
       QString recevedStr = QString::fromLatin1(port->readAll());
//       ui->console->insertPlainText(recevedStr);

       QRegExp rx("(\\,)");
       QStringList query = recevedStr.split(rx);
       //read each tokens
        for (int i=0 ; i < query.length(); i++)
        {
            bool isNumeric = false;

            recevedStr = query.at(i);
            ui->console->insertPlainText(recevedStr);
            int tempData = (recevedStr.toInt(&isNumeric)) ;

              if (isNumeric )
              {

                globalSerialDataValue=tempData;
                ui->console->insertPlainText(recevedStr);
                ui->console->insertPlainText("\n");

                serialDataList <<globalSerialDataValue;

        }
      }
   }
}

//************************File Handling Segment End********************************
//*********************************************************************************


void MainWindow::on_stopBtn_clicked()
{
    dataTimer.stop();
}



void MainWindow::on_commandLinkBtnClearConsole_clicked()
{
    ui->console->clear();
}


//************************FFT Calculation Segment begin*******************************
//************************************************************************************
#define PI  3.141592653589793238460

//initilaize e-to-the-i-thetas for theta = 0..2PI in increments of 1/N
void MainWindow::sdft_init_coeffs()
{
    for (int i = 0; i < N; ++i) {
        double a = -2.0 * PI * i  / double(N);
        coeffs[i] = Complex(cos(a)/* / N */, sin(a) /* / N */);
    }
    for (int i = 0; i < N; ++i) {
        double a = 2.0 * PI * i  / double(N);
        icoeffs[i] = Complex(cos(a),sin(a));
    }
}


// initialize all data buffers
void MainWindow::sdft_init()
{
    // clear data
    for (int i = 0; i < N; ++i)
        in[i] = 0.0;
//     seed rand()
//    srand(857); // for random number generation
    sdft_init_coeffs();
    oldest_data = newest_data = 0.0;
    idx = 0;
}

// simulating adding data to circular buffer
void MainWindow::sdft_add_data()
{

    oldest_data = in[idx];
    newest_data = in[idx] =0;// data;//  double(rand() / double(N));

}

// sliding dft
void MainWindow::sdft()
{
    double delta;
        delta = newest_data - oldest_data;
    int ci = 0;

    for (int i = 0; i < N; ++i)
    {
            //std::cout <<"ci ......"<<ci<<"\n";
            freqs[i] += delta * coeffs[ci];
//            if (freqs[i] >100)
//                freqs[i]=0;
            //std::cout << "\n sdtf Function  ....\n";
            //std::cout<<freqs[i][j];
            if ((ci += idx) >= N)
                ci -= N;
    }
}

double MainWindow::mag(Complex& c)
{
    return sqrt(c.real() * c.real() + c.imag() * c.imag());
}

void MainWindow::powr_spectrum(double *powr)
{
    for (int i = 0; i < N/2; ++i) {
        powr[i] = mag(freqs[i]);
    }

}
//************************FFT Calculation Segment End*******************************
//**********************************************************************************




//************************File Handling Segment Begin*******************************
//**********************************************************************************

#include <QFileDialog>

//global File Name
QString fileName="";
void MainWindow::on_toolButton_browse_clicked()
{
    fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"/path/to/file/",tr(".txt Files (*.txt)"));
    ui->lineEdit_browse_file->setText(fileName);
    ParseSignalFile();
//    ui->listWidget->addItems(fileNames);
}


void MainWindow::ParseSignalFile(){

    if (fileName !="")
    {
        int tempgData;
        QFile file(fileName);
        //ui->statusBar->showMessage(filename);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {

            while (!file.atEnd())
            {
                QString line = file.readLine();
                QStringList query = line.split(",");
                for (int i=0 ; i < query.length(); i++)
                {
                    bool isNumeric = false;
                    QString tokenStr = query.at(i);
//                    ui->console->insertPlainText(tokenStr);
                    int tempgData = (tokenStr.toInt(&isNumeric));

                    if (isNumeric)
                    {
                        globalSerialDataValue=tempgData;
                        serialDataList<<globalSerialDataValue;
                    }
                }
            }
            file.close();
        }
    }
}

void MainWindow::logData(){

    qDebug()<<"eeee";
     //Log File Configuration
     //***********************************************
     QDateTime local(QDateTime::currentDateTime());
     qDebug() << "Local time is:" << local.toString();
     QFile logFile("LOG/LOG."+local.toString()+".log");

     if (logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
     {
         QTextStream logSerialDataStream(&logFile);
         for (int i = 0; i < serialDataList.size(); ++i) {
             qDebug()<<serialDataList.at(i);
             logSerialDataStream << serialDataList.at(i)<<"\n";
          }
     }

     //***********************************************
}

//************************File Handling Segment End********************************
//*********************************************************************************
