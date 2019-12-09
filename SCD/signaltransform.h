#ifndef SIGNALTRANSFORM_H
#define SIGNALTRANSFORM_H

#include <QWidget>
#include "signalmanager.h"

namespace Ui {
class SignalTransform;
}

class SignalTransform : public QWidget
{
    Q_OBJECT

public:
    SignalTransform(SignalManager* p_sm);
    SignalTransform(QWidget* parent, SignalManager* p_sm);
    ~SignalTransform();

private slots:
    void on_tf_foff_box_valueChanged(double arg1);

    void on_button_abs_square_clicked(bool checked);

    void on_button_qube_clicked(bool checked);

    void on_button_square_clicked(bool checked);

    void on_button_none_clicked(bool checked);

signals:
    void refresh_plot();

private:
    Ui::SignalTransform *ui;
    SignalManager* m_sm;
    QWidget* main_window;

    my_float f_off;
    int non_linearity;

    void none();
    void square();
    void qube();
    void abs();
    void abs_square();
    void f_shift(float freq);

    void reset();

    void transform();
};

#endif // SIGNALTRANSFORM_H
