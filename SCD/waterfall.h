#ifndef WATERFALL_H
#define WATERFALL_H

#include <QWidget>
#include <QImage>

#include <fftw3.h>

#include "mytypes.h"
#include "signalmanager.h"
#include "windowfunctions.h"

namespace Ui {
class Waterfall;
}

class Waterfall : public QWidget
{
    Q_OBJECT

public:
    Waterfall(SignalManager* p_sm);
    ~Waterfall();

    void set_parameters();
    void update();
    void clear_all();

    QRgb my_cmap(my_float in);

private slots:
    void on_amp_scale_valueChanged(double arg1);

    void on_fftlen_currentIndexChanged(int index);

    void on_overlap_valueChanged(int arg1);

    void on_window_function_currentIndexChanged(int index);

    void on_button_log_clicked(bool checked);

    void on_min_db_valueChanged(int arg1);

    void on_max_db_valueChanged(int arg1);

private:
    Ui::Waterfall *ui;

    SignalManager* m_sm;

    QImage* m_waterfall;

    WindowType m_win_type;

    bool calc_log;

    size_t m_fft_len;
    size_t m_overlap;
    size_t m_shift;
    size_t m_n_windows;

    size_t m_img_x;
    size_t m_img_y;

    // Colorscaling
    my_float min;
    my_float max;

    // FFT
    fftwf_plan m_fft_plan_fft; // Einganswerte Transformieren
    size_t* m_fft_idx;         // Speicher für FFT Shift Indizies
    my_cplx* m_win;
    my_cplx* m_fft_in;
    my_cplx* m_fft_out;

    my_float m_scale;
};

#endif // WATERFALL_H
