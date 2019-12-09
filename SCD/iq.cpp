#include "iq.h"
#include "ui_iq.h"

IQ::IQ(SignalManager* p_sm):
    ui(new Ui::IQ),
    m_sm(p_sm)
{
    ui->setupUi(this);
    curve = new QwtPlotCurve( "IQ" );
    curve->setPen(QColor(64, 148, 196),3);
    curve->setStyle(QwtPlotCurve::CurveStyle::Dots);
//    curve->setSymbol()
    curve->attach(ui->qwtPlot);

    m_real   = nullptr;
    m_imag   = nullptr;

    m_grid = new QwtPlotGrid();
    m_grid->setPen(QColor(75, 75, 75),1);
    m_grid->attach(ui->qwtPlot);

    // Window
    m_start_sample = 0;
    m_max_offset = 0;
    m_window_size = ui->win_size->value();

    update();

    m_zoomer = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, ui->qwtPlot->canvas() );
    m_zoomer->setEnabled(false);

    m_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, ui->qwtPlot->canvas());
    m_picker->setStateMachine(new QwtPickerClickPointMachine);

    connect(m_picker, SIGNAL(selected(const QPointF&)), this, SLOT(on_update_coords(const QPointF&)) );
}

IQ::~IQ()
{
    clear_all();

    delete curve;

    delete ui;
}

void IQ::clear_all(){
    delete[] m_real;
    m_real = nullptr;
    delete[] m_imag;
    m_imag = nullptr;
}

void IQ::update(){
    clear_all();
    std::cout << "TEST uca " << std::endl;

    // Window Size
    m_max_offset = std::floor(m_sm->size - ui->win_size->value());
    if(m_max_offset > 0 ){
        ui->t_offset->setEnabled(true);
        ui->t_offset->setMaximum(m_max_offset);
    } else {
        ui->t_offset->setEnabled(false);
        ui->t_offset->setMaximum(m_max_offset);
    }
    std::cout << "TEST uca 1 " << std::endl;

    if(ui->win_size->value() > m_sm->size){
        ui->win_size->setValue(m_sm->size);
    } else {
        replot();
    }
    std::cout << "TEST uca  2" << std::endl;

    m_real   = new double[m_sm->size];
    m_imag   = new double[m_sm->size];

    size_t samples = 0;
    for(size_t i = ui->toffset->value(); i < m_sm->size; i=i+ui->sps->value()){
        m_real[samples]  = std::real(m_sm->signal[i]);
        m_imag[samples]  = std::imag(m_sm->signal[i]);
        samples++;
    }

    curve->setRawSamples(m_real + ui->t_offset->value(), m_imag + ui->t_offset->value(), samples);

    std::cout << "TEST uca rere" << std::endl;
    replot();
}

void IQ::replot(){
    curve->setVisible(true);

//    ui->qwtPlot->setAxisScale(QwtPlot::xBottom, ui->t_offset->value()
//                                              , ui->t_offset->value() + ui->win_size->value());

    ui->qwtPlot->replot();
}

void IQ::on_real_toggled(bool checked)
{
    replot();
}

void IQ::on_imag_toggled(bool checked)
{
    replot();
}

void IQ::on_abs_toggled(bool checked)
{
    replot();
}

void IQ::on_toolButton_toggled(bool checked)
{
    m_zoomer->setEnabled(checked);
}


void IQ::on_update_coords(const QPointF& p_qp){
    ui->x_coord->setNum(p_qp.x());
    ui->y_coord->setNum(p_qp.y());
}

void IQ::on_button_zoom_toggled(bool checked)
{
    ui->qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
    ui->qwtPlot->setAxisAutoScale(QwtPlot::yLeft, true);
    update();
}

void IQ::on_t_offset_valueChanged(int value)
{
    curve->setRawSamples(m_real + value, m_imag + value, ui->win_size->value());
    replot();
}

void IQ::on_win_size_valueChanged(int arg1)
{
    // Window Size
    m_max_offset = std::floor(m_sm->size - ui->win_size->value());
    if(m_max_offset > 0 ){
        ui->t_offset->setEnabled(true);
        ui->t_offset->setMaximum(m_max_offset);
    } else {
        ui->t_offset->setEnabled(false);
        ui->t_offset->setMaximum(0);
//        ui->t_offset->setValue(0);
    }

    if(ui->win_size->value() > m_sm->size){
        ui->win_size->setValue(m_sm->size);
    }

    curve->setRawSamples(m_real + ui->t_offset->value(), m_imag + ui->t_offset->value(), ui->win_size->value());

    replot();

}

void IQ::on_sps_valueChanged(int arg1)
{
    update();
}

void IQ::on_toffset_valueChanged(int arg1)
{
    update();
}
