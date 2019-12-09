#include "psd.h"
#include "ui_psd.h"

#include <iostream>

PSD::PSD(SignalManager* p_sm) :
    ui(new Ui::PSD),
    pwelch(p_sm)
{
    ui->setupUi(this);

    curve = nullptr;
    update_plot();

    m_zoomer = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, ui->qwtPlot->canvas() );
    m_zoomer->setEnabled(false);

    m_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,ui->qwtPlot->canvas());
    m_picker->setStateMachine(new QwtPickerClickPointMachine);

    m_grid = new QwtPlotGrid();
    m_grid->setPen(QColor(75, 75, 75),1);
    m_grid->attach(ui->qwtPlot);

    connect(m_picker, SIGNAL(selected(const QPointF&)), this, SLOT(on_update_coords(const QPointF&)) );

    m_fft_len   = 1024;
    m_overlap   = 256;
    m_win_type  = WindowType::Rect;

    ui->psd_fftlen->setCurrentIndex(1);
//    ui->psd_fftlen->currentIndexChanged(1);
    ui->qwtPlot->setAxisTitle(0, "Leistung / FFT Bin");
    ui->qwtPlot->setAxisTitle(2, "Frequenz / f_s");
}

PSD::~PSD()
{
    delete ui;
    delete curve;
}

void PSD::update(){
    pwelch.set_parameters(m_fft_len, m_win_type, m_overlap);
    pwelch.update_parameters();
    pwelch.calc();

    update_plot();

    m_zoomer->setZoomBase();
    //m_zoomer->setZoomBase();
}

void PSD::update_plot(){
    if( curve == nullptr){
        curve = new QwtPlotCurve( "PSD" );
        curve->setPen(QColor(64, 148, 196),2);
        curve->attach(ui->qwtPlot);
    }
    curve->setRawSamples(pwelch.get_x(), pwelch.get_y(), m_fft_len);

    ui->qwtPlot->setAxisScale(QwtPlot::xBottom, -0.5, 0.5, 0.1);
//    ui->qwtPlot->updateAxes();

    ui->qwtPlot->replot();
}

void PSD::on_psd_fftlen_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        m_fft_len = 64;
        break;
    case 1:
        m_fft_len = 128;
        break;
    case 2:
        m_fft_len = 256;
        break;
    case 3:
        m_fft_len = 512;
        break;
    case 4:
        m_fft_len = 1024;
        break;
    case 5:
        m_fft_len = 2048;
        break;
    case 6:
        m_fft_len = 4096;
        break;
    default:
        m_fft_len = 1024;
    }

    if(m_overlap > m_fft_len-1){
        m_overlap = m_fft_len / 2;
        ui->psd_overlap->setValue(m_overlap);
        ui->psd_overlap->setMaximum(m_fft_len - 1);
    } else {
        ui->psd_overlap->setMaximum(m_fft_len - 1);
        m_overlap = m_fft_len / 2;
        ui->psd_overlap->setValue(m_overlap);
    }

    pwelch.set_parameters(m_fft_len, m_win_type, m_overlap);
    pwelch.update_parameters();
    pwelch.calc();
    update_plot();
}

void PSD::on_psd_overlap_valueChanged(int arg1)
{
    m_overlap = arg1;
    update();
}

void PSD::on_psd_window_function_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        m_win_type = WindowType::Rect;
        break;
    case 1:
        m_win_type = WindowType::Hamming;
        break;
    case 2:
        m_win_type = WindowType::Blackman;
        break;
    case 3:
        m_win_type = WindowType::BlackmanHarris;
        break;
    default:
        m_win_type = WindowType::Rect;
    }

    pwelch.set_parameters(m_fft_len, m_win_type, m_overlap);
    pwelch.update_parameters();
    pwelch.calc();
    update_plot();
}

void PSD::on_update_coords(const QPointF& p_qp){
    ui->x_coord->setNum(p_qp.x());
    ui->y_coord->setNum(p_qp.y());
}


void PSD::on_toolButton_toggled(bool checked)
{
    m_zoomer->setEnabled(checked);
}

void PSD::on_button_log_toggled(bool checked)
{
    m_zoomer->zoom(0);
    pwelch.calc_log = checked;
    ui->qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
    ui->qwtPlot->setAxisAutoScale(QwtPlot::yLeft, true);
    update();
}

void PSD::on_button_zoom_clicked(bool checked)
{
    ui->qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
    ui->qwtPlot->setAxisAutoScale(QwtPlot::yLeft, true);
    update();
}
