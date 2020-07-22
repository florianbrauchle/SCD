#ifndef SCD_H
#define SCD_H

#include <QWidget>
#include <QTimer>
#include <QMessageBox>
#include "spectogram.h"
#include "signalmanager.h"

namespace Ui {
class SCD;
}

class SCD : public QWidget
{
    Q_OBJECT

public:
    explicit SCD(QWidget *parent = nullptr);
    SCD(SignalManager* p_sm);

    ~SCD();

    void update();
    void autoscale();

private slots:
    void on_scd_fftlen_currentIndexChanged(int index);

    void on_scd_window_function_currentIndexChanged(int index);

    void on_scd_P_valueChanged(int arg1);

    void on_scd_conj_toggled(bool checked);

    void on_slider_toff_sliderMoved(int position);

    void on_scd_logarithmic_toggled(bool checked);

    void on_min_db_3_valueChanged(int arg1);

    void on_max_db_3_valueChanged(int arg1);

    void on_cs_min_valueChanged(double arg1);

    void on_cs_max_valueChanged(double arg1);

    void plot_timer_update();

    void on_b_stopPlot_clicked();

    void on_b_startPlot_clicked();

    void on_scd_aspect_ratio_valueChanged(double arg1);

    void on_pB_AutoScale_clicked();

private:
    Ui::SCD *ui;
    // Data
    SignalManager* m_sm;

    // plot data
    QTimer *timer;

    // Surface Plot
    Q3DSurface *graph;
    QWidget *container;

    Spectogram *cyc_plt;

    // Parameters
    size_t m_fft_len;
    size_t m_windows;
};

#endif // SCD_H
