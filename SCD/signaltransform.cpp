#include "signaltransform.h"
#include "ui_signaltransform.h"

#include <iostream>

SignalTransform::SignalTransform(SignalManager* p_sm) :
    ui(new Ui::SignalTransform),
    m_sm(p_sm)
{
    ui->setupUi(this);
}

SignalTransform::SignalTransform(QWidget* parent, SignalManager* p_sm) :
    ui(new Ui::SignalTransform),
    m_sm(p_sm),
    main_window(parent)
{
    ui->setupUi(this);

    QObject::connect(this, SIGNAL(refresh_plot()), main_window, SLOT(on_refresh_plot()));

    non_linearity = 0;
}

SignalTransform::~SignalTransform()
{
    delete ui;
}


void SignalTransform::reset(){
    for (size_t i = 0; i < m_sm->size; ++i) {
        m_sm->signal[i] = m_sm->original[i];
    }
}


void SignalTransform::none(){

}

void SignalTransform::square(){
    for (size_t i = 0; i < m_sm->size; ++i) {
        m_sm->signal[i] = std::pow(m_sm->signal[i], 2);
    }
}

void SignalTransform::qube(){
    for (size_t i = 0; i < m_sm->size; ++i) {
        m_sm->signal[i] = std::pow(m_sm->signal[i], 4);
    }
}

void SignalTransform::abs(){
    for (size_t i = 0; i < m_sm->size; ++i) {
        m_sm->signal[i] = std::abs(m_sm->signal[i]);
    }
}

void SignalTransform::abs_square(){
    for (size_t i = 0; i < m_sm->size; ++i) {
        //m_sm->signal[i] = std::abs(std::pow(m_sm->signal[i], 2));
        m_sm->signal[i] = std::pow(std::abs(m_sm->signal[i]), 2);
    }
}

//void SignalTransform::abs_square(){
//    for (size_t i = 0; i < m_sm->size; ++i) {
//        m_sm->signal[i] = std::pow(std::abs(m_sm->signal[i]), 2);
//    }
//}

void SignalTransform::f_shift(float freq){
    f_off = freq;
    m_sm->backup();
    std::cout << freq << std::endl;
    float phase = 0;
    for (size_t i = 0; i < m_sm->size; ++i) {
        m_sm->signal[i] = m_sm->original[i] * std::exp(my_cplx(0, phase));

        phase += 2.0*M_PI*freq;

        if(phase < -2.0*M_PI) {
            phase += 2.0*M_PI;
        }
        if(phase > 2.0*M_PI) {
            phase -= 2.0*M_PI;
        }
    }

}


void SignalTransform::on_tf_foff_box_valueChanged(double arg1)
{
    f_off = arg1;
    transform();
}


void SignalTransform::on_button_none_clicked(bool checked)
{
    if(checked){
        non_linearity = 0;
    }
    transform();
}

void SignalTransform::on_button_square_clicked(bool checked)
{
    if(checked){
        non_linearity = 1;
    }
    transform();
}

void SignalTransform::on_button_abs_square_clicked(bool checked)
{
    if(checked){
        non_linearity = 2;
    }
    transform();
}

void SignalTransform::on_button_qube_clicked(bool checked)
{
    if(checked){
        non_linearity = 3;
    }
    transform();
}

void SignalTransform::transform(){
    m_sm->backup();
    reset();

    if(f_off != 0){
        f_shift(f_off);
    }

    switch (non_linearity) {
    case 0:
        none();
        break;
    case 1:
        square();
        break;
    case 2:
        abs_square();
        break;
    case 3:
        qube();
        break;
    default:
        none();
    }

    refresh_plot();
}
