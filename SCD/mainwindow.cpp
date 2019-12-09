#include "mainwindow.h"
#include "ui_mainwindow.h"

// Include the Subprogram
#include "psd.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mw_sm(2<<24)
{
    ui->setupUi(this);

    mw_psd = nullptr;
    mw_td  = nullptr;
    mw_scd = nullptr;
    mw_scdf0 = nullptr;
    mw_waterfall = nullptr;
    mw_iq = nullptr;

    mw_psd = new PSD(&mw_sm);

    ui->verticalLayout->insertWidget(0, mw_psd, 1);

    mw_trans = new SignalTransform(this, &mw_sm);
    ui->verticalLayout->insertWidget(1, mw_trans, 0);
    mw_trans->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete mw_psd;
    delete mw_td;
    delete mw_scd;
    delete mw_scdf0;
    delete mw_waterfall;
    delete mw_iq;

    delete mw_trans;

    delete ui;
}



void MainWindow::mw_close_widgets(){
    delete mw_psd;
    mw_psd = nullptr;
    delete mw_td;
    mw_td = nullptr;
    delete mw_scd;
    mw_scd = nullptr;
    delete mw_scdf0;
    mw_scdf0 = nullptr;
    delete mw_waterfall;
    mw_waterfall = nullptr;
    delete mw_iq;
    mw_iq = nullptr;
}

void MainWindow::on_actionTime_triggered()
{
    mw_close_widgets();
    mw_td = new TD(&mw_sm);
    ui->verticalLayout->insertWidget(0, mw_td, 1);
    this->setWindowTitle("SA - Time Domain");
}

void MainWindow::on_actionSpectrum_triggered()
{
    mw_close_widgets();
    mw_psd = new PSD(&mw_sm);
    ui->verticalLayout->insertWidget(0, mw_psd, 1);
    this->setWindowTitle("SA - Power Spectral Density");
}

void MainWindow::on_actionSCD_triggered()
{
    mw_close_widgets();
    mw_scd = new SCD(&mw_sm);
    ui->verticalLayout->insertWidget(0, mw_scd, 1);
    this->setWindowTitle("SA - Spectral Correlation Density");
}

void MainWindow::on_actionSCD_at_f_0_triggered()
{
    mw_close_widgets();
    mw_scdf0 = new SCDF0(&mw_sm);
    ui->verticalLayout->insertWidget(0, mw_scdf0, 1);
    this->setWindowTitle("SA - Power Spectral Density at f = 0");
}

void MainWindow::on_actionWaterfall_triggered()
{
    mw_close_widgets();
    mw_waterfall = new Waterfall(&mw_sm);
    ui->verticalLayout->insertWidget(0, mw_waterfall, 1);
    this->setWindowTitle("SA - Waterfall");
}

void MainWindow::on_actionIQ_triggered()
{
    mw_close_widgets();
    mw_iq = new IQ(&mw_sm);
    ui->verticalLayout->insertWidget(0, mw_iq, 1);
    this->setWindowTitle("SA - IQ");
}

void MainWindow::on_actionLoad_Signal_triggered()
{
    QString filename = QFileDialog::getOpenFileName();

    if (filename.isEmpty())
        return;
    else {
        std::cout << filename.toUtf8().constData();
        mw_sm.open_file(filename.toUtf8().constData());

        std::cout << ", Samples: " << mw_sm.size << std::endl;
    }
    update_signal();
}

/***************************************************************************/
void MainWindow::update_signal(){
    if( mw_scd != nullptr ){
        mw_scd->update();
    }

    if( mw_psd != nullptr ){
        mw_psd->update();
    }

    if( mw_scdf0 != nullptr ){
        mw_scdf0->update();
    }

    if( mw_waterfall != nullptr ){
        mw_waterfall->update();
    }

    if( mw_td != nullptr ){
        mw_td->update();
    }

    if( mw_iq != nullptr ){
        mw_iq->update();
    }
}

void MainWindow::on_refresh_plot(){
    update_signal();
}

void MainWindow::on_actionToolbar_toggled(bool arg1)
{
    mw_trans->setVisible(arg1);
}
