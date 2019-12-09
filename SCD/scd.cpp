#include "scd.h"
#include "ui_scd.h"

SCD::SCD(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SCD)
{
    ui->setupUi(this);

    // SPectogram
    graph = new Q3DSurface();
    container = QWidget::createWindowContainer(graph);

    if (!graph->hasContext()) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize the OpenGL context.");
        msgBox.exec();
    }

    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setFocusPolicy(Qt::StrongFocus);

    ui->verticalLayout->insertWidget(0, container, 1);

    // timer für Plot Updates
    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(plot_timer_update()));
}

SCD::SCD(SignalManager* p_sm) :
    ui(new Ui::SCD),
    m_sm(p_sm)
{
    ui->setupUi(this);

    cyc_plt = nullptr;

    // SPectogram
    graph = new Q3DSurface();
    container = QWidget::createWindowContainer(graph);

    if (!graph->hasContext()) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize the OpenGL context.");
        msgBox.exec();
    }

    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setFocusPolicy(Qt::StrongFocus);

    ui->verticalLayout->insertWidget(0, container, 1);

    // SCD Setup
    m_fft_len = 64;
    m_windows = ui->scd_P->value();

    cyc_plt = new Spectogram(graph, m_sm, 64);
    on_min_db_3_valueChanged(1);

    // timer für Plot Updates
    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(plot_timer_update()));
}

SCD::~SCD()
{
    delete graph; //destroyed by container?
    delete container;
    delete cyc_plt;
    delete timer;
    delete ui;
}

void SCD::update(){
    // Scale
    if(ui->scd_logarithmic->isChecked()){
        cyc_plt->set_scale(ui->min_db_3->value(), ui->max_db_3->value());
    } else {
        double min = std::pow(10, ui->min_db_3->value()/10.0);
        double max = std::pow(10, ui->max_db_3->value()/10.0);
        cyc_plt->set_scale(min, max);
    }

    // Window and FFT Len
    size_t m_max_windows = m_sm->size * 4 / m_fft_len - 3;
    if( m_max_windows < m_windows){
        ui->scd_P->setValue(m_max_windows);
        m_windows = m_max_windows;
    }

    size_t m_max_input_offset = cyc_plt->set_max_input_len(m_windows);

    if( m_max_input_offset > 0){
        ui->slider_toff->setMaximum(m_max_input_offset);
        ui->slider_toff->setEnabled(true);
        ui->b_startPlot->setEnabled(true);
    } else {
        ui->slider_toff->setMaximum(0);
        ui->slider_toff->setEnabled(false);
        ui->b_startPlot->setEnabled(false);
        ui->b_stopPlot->setEnabled(false);
    }

    cyc_plt->updateSignal();
}

void SCD::on_scd_fftlen_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        cyc_plt->m_scd.set_fftlen(64);
        cyc_plt->set_fftlen(64);
        m_fft_len = 64;
        break;
    case 1:
        cyc_plt->m_scd.set_fftlen(128);
        cyc_plt->set_fftlen(128);
        m_fft_len = 128;
        break;
    case 2:
        cyc_plt->m_scd.set_fftlen(256);
        cyc_plt->set_fftlen(256);
        m_fft_len = 256;
        break;
    case 3:
        cyc_plt->m_scd.set_fftlen(512);
        cyc_plt->set_fftlen(512);
        m_fft_len = 512;
        break;
    case 4:
        cyc_plt->m_scd.set_fftlen(1024);
        cyc_plt->set_fftlen(1024);
        m_fft_len = 1024;
        break;
    default:
        cyc_plt->m_scd.set_fftlen(128);
        cyc_plt->set_fftlen(128);
        m_fft_len = 128;
    }

    cyc_plt->updatePlotData();
}

void SCD::on_scd_window_function_currentIndexChanged(int index)
{
    cyc_plt->m_scd.set_window(index);
    cyc_plt->updatePlotData();
}

void SCD::on_scd_P_valueChanged(int arg1)
{
    size_t m_max_windows = m_sm->size * 4 / m_fft_len - 3;
    if( m_max_windows < arg1){
        ui->scd_P->setValue(m_max_windows);
        arg1 = m_max_windows;
    }

    size_t m_max_input_offset = cyc_plt->set_max_input_len(arg1);

    if( m_max_input_offset > 0){
        ui->slider_toff->setMaximum(m_max_input_offset);
        ui->slider_toff->setEnabled(true);
        ui->b_startPlot->setEnabled(true);
    } else {
        ui->slider_toff->setMaximum(0);
        ui->slider_toff->setEnabled(false);
        ui->b_startPlot->setEnabled(false);
        ui->b_stopPlot->setEnabled(false);
    }
}

void SCD::on_scd_conj_toggled(bool checked)
{
    cyc_plt->calc_conj = checked;
    cyc_plt->updatePlotData();
}

void SCD::on_slider_toff_sliderMoved(int position)
{
    cyc_plt->set_signal_offset(position);
}

void SCD::on_scd_logarithmic_toggled(bool checked)
{
    if(checked){
        cyc_plt->set_scale(ui->min_db_3->value(), ui->max_db_3->value());
    } else {
        double min = std::pow(10, ui->min_db_3->value()/10);
        double max = std::pow(10, ui->max_db_3->value()/10);
        cyc_plt->set_scale(min, max);
    }

    cyc_plt->set_log(checked);
    cyc_plt->updatePlotData();
}

void SCD::on_min_db_3_valueChanged(int arg1)
{
    // Scale
    if(ui->scd_logarithmic->isChecked()){
        cyc_plt->set_scale(ui->min_db_3->value(), ui->max_db_3->value());
    } else {
        double min = std::pow(10, ui->min_db_3->value()/10.0);
        double max = std::pow(10, ui->max_db_3->value()/10.0);
        cyc_plt->set_scale(min, max);
    }
}

void SCD::on_max_db_3_valueChanged(int arg1)
{
    // Scale
    if(ui->scd_logarithmic->isChecked()){
        cyc_plt->set_scale(ui->min_db_3->value(), ui->max_db_3->value());
    } else {
        double min = std::pow(10, ui->min_db_3->value()/10.0);
        double max = std::pow(10, ui->max_db_3->value()/10.0);
        cyc_plt->set_scale(min, max);
    }
}


void SCD::on_cs_min_valueChanged(double arg1)
{
    cyc_plt->set_cm(ui->cs_min->value(), ui->cs_max->value());
}

void SCD::on_cs_max_valueChanged(double arg1)
{
    cyc_plt->set_cm(ui->cs_min->value(), ui->cs_max->value());
}


void SCD::plot_timer_update(){
    ui->slider_toff->setValue(ui->slider_toff->value()+ui->sb_step_size->value());
    on_slider_toff_sliderMoved(ui->slider_toff->value());
    if(ui->slider_toff->maximum() <= ui->slider_toff->value()){
        on_b_stopPlot_clicked();
    }
}

void SCD::on_b_stopPlot_clicked()
{
    timer->stop();
    ui->b_stopPlot->setEnabled(false);
    ui->b_startPlot->setEnabled(true);
}

void SCD::on_b_startPlot_clicked()
{
    timer->start(1/ui->refreshRate->value());
    ui->b_stopPlot->setEnabled(true);
    ui->b_startPlot->setEnabled(false);
}

void SCD::on_scd_aspect_ratio_valueChanged(double arg1)
{
    cyc_plt->set_aspect_ratio(arg1);
}
