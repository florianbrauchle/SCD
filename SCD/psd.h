#ifndef PSD_H
#define PSD_H

#include <QWidget>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_grid.h>

#include "psdwelch.h"
#include "signalmanager.h"



namespace Ui {
class PSD;
}

class PSD : public QWidget
{
    Q_OBJECT

public:
    PSD(SignalManager* p_sm);
    ~PSD();
    void update();
    void update_plot();

private slots:
    void on_psd_fftlen_currentIndexChanged(int index);

    void on_psd_window_function_currentIndexChanged(int index);

    void on_update_coords(const QPointF& p_qp);

    void on_toolButton_toggled(bool checked);

    void on_button_log_toggled(bool checked);

    void on_button_zoom_clicked(bool checked);

    void on_psd_overlap_valueChanged(int arg1);

private:
    Ui::PSD *ui;

    QwtPlotZoomer* m_zoomer;
    QwtPlotPicker* m_picker;
    QwtPlotGrid* m_grid;

    PSDWelch pwelch;
    size_t m_fft_len   = 1024;
    WindowType m_win_type = WindowType::Rect;
    size_t m_overlap   = 256;

    QwtPlotCurve* curve;
};

#endif // PSD_H
