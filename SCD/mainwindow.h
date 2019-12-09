#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets/QMessageBox>
#include <QFileDialog>

// Subprograms
#include "psd.h"
#include "td.h"
#include "scd.h"
#include "scdf0.h"
#include "waterfall.h"
#include "iq.h"

#include "signaltransform.h"

// Signal
#include "signalmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionTime_triggered();

    void on_actionSpectrum_triggered();

    void on_actionSCD_triggered();

    void on_actionLoad_Signal_triggered();

    void on_actionToolbar_toggled(bool arg1);

    void on_refresh_plot();

    void on_actionSCD_at_f_0_triggered();

    void on_actionWaterfall_triggered();

    void on_actionIQ_triggered();

private:
    Ui::MainWindow *ui;

    SignalManager mw_sm;

    PSD* mw_psd;
    TD*  mw_td;
    IQ* mw_iq;
    SCD* mw_scd;
    SCDF0* mw_scdf0;
    Waterfall* mw_waterfall;

    SignalTransform* mw_trans;

    void mw_close_widgets();
    void update_signal();
};

#endif // MAINWINDOW_H
