#ifndef IQ_H
#define IQ_H

#include <QWidget>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_grid.h>

#include "signalmanager.h"
#include "mytypes.h"

namespace Ui {
class IQ;
}

class IQ : public QWidget
{
    Q_OBJECT

public:
    IQ(SignalManager* p_sm);
    ~IQ();

    void clear_all();
    void update();

    void replot();

private slots:
    void on_real_toggled(bool checked);

    void on_imag_toggled(bool checked);

    void on_abs_toggled(bool checked);

    void on_toolButton_toggled(bool checked);

    void on_button_zoom_toggled(bool checked);

    void on_update_coords(const QPointF& p_qp);

    void on_t_offset_valueChanged(int value);

    void on_win_size_valueChanged(int arg1);

    void on_sps_valueChanged(int arg1);

    void on_toffset_valueChanged(int arg1);

private:
    Ui::IQ *ui;

    SignalManager* m_sm;


    QwtPlotZoomer* m_zoomer;
    QwtPlotPicker* m_picker;
    QwtPlotGrid* m_grid;

    // Curves
    QwtPlotCurve* curve;

    // storage for all the domains
    double* m_real;
    double* m_imag;

    // Window
    size_t m_start_sample;
    size_t m_max_offset;
    size_t m_window_size;


};

#endif // TD_H
