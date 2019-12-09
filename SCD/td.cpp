#include "td.h"
#include "ui_td.h"

TD::TD(SignalManager* p_sm):
    ui(new Ui::TD),
    m_sm(p_sm)
{
    ui->setupUi(this);
    curve_real = new QwtPlotCurve( "real" );
    curve_real->setPen(QColor(64, 148, 196),2);
    curve_real->attach(ui->qwtPlot);

    curve_imag = new QwtPlotCurve( "imag" );
    curve_imag->setPen(QColor(196, 64, 148),2, Qt::PenStyle::DashDotLine);
    curve_imag->attach(ui->qwtPlot);

    curve_abs = new QwtPlotCurve( "abs" );
    curve_abs->setPen(QColor(148, 196, 64),2, Qt::PenStyle::DotLine);
    curve_abs->attach(ui->qwtPlot);

    curve_angle = new QwtPlotCurve( "Angle" );
    curve_angle->setPen(QColor(148, 196, 64),2, Qt::PenStyle::DotLine);
    curve_angle->attach(ui->qwtPlot);

    curve_unwrap = new QwtPlotCurve( "Unwrap" );
    curve_unwrap->setPen(QColor(148, 196, 64),2, Qt::PenStyle::DotLine);
    curve_unwrap->attach(ui->qwtPlot);

    m_x_axis = nullptr;

    m_real   = nullptr;
    m_imag   = nullptr;
    m_abs    = nullptr;
    m_angle  = nullptr;
    m_unwrap = nullptr;

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

    ui->qwtPlot->setAxisTitle(0, "Amplitude");
    ui->qwtPlot->setAxisTitle(2, "Samples");
}

TD::~TD()
{
    clear_all();

    delete curve_real;
    delete curve_imag;
    delete curve_abs;
    delete curve_angle;
    delete curve_unwrap;

    delete ui;
}

void TD::clear_all(){
    delete[] m_x_axis;
    m_x_axis = nullptr;
    delete[] m_real;
    m_real = nullptr;
    delete[] m_imag;
    m_imag = nullptr;
    delete[] m_abs;
    m_abs = nullptr;
    delete[] m_angle;
    m_angle = nullptr;
    delete[] m_unwrap;
    m_unwrap = nullptr;
}

void TD::update(){
    clear_all();

    // Window Size
    m_max_offset = std::floor(m_sm->size - ui->win_size->value());
    if(m_max_offset > 0 ){
        ui->t_offset->setEnabled(true);
        ui->t_offset->setMaximum(m_max_offset);
    } else {
        ui->t_offset->setEnabled(false);
        ui->t_offset->setMaximum(m_max_offset);
    }

    if(ui->win_size->value() > m_sm->size){
        ui->win_size->setValue(m_sm->size);
    } else {
        replot();
    }

    m_x_axis = new double[m_sm->size];

    m_real   = new double[m_sm->size];
    m_imag   = new double[m_sm->size];
    m_abs    = new double[m_sm->size];
    m_angle  = new double[m_sm->size];
    m_unwrap = new double[m_sm->size];

    for(size_t i = 0; i < m_sm->size; ++i){
        m_x_axis[i] = i;
        m_real[i]  = std::real(m_sm->signal[i]);
        m_imag[i]  = std::imag(m_sm->signal[i]);
        m_abs[i]   = std::abs(m_sm->signal[i]);
        m_angle[i] = std::arg(m_sm->signal[i]);
    }

    double ph_off = 0;
    if(m_sm->size > 0){
        m_unwrap[0] = m_angle[0];
        for(size_t i = 1; i < m_sm->size; ++i){
            m_unwrap[i] = m_angle[i] + ph_off;
            double dif = m_unwrap[i] - m_unwrap[i-1];
            dif = dif / M_PI;
            if( dif > 1){
                m_unwrap[i] -= 2*M_PI;
                ph_off      -= 2*M_PI;
            } else if ( dif < -1 ) {
                m_unwrap[i] += 2*M_PI;
                ph_off      += 2*M_PI;
            }
        }
    }


    curve_real->setRawSamples(m_x_axis + ui->t_offset->value(), m_real + ui->t_offset->value(), ui->win_size->value());
    curve_imag->setRawSamples(m_x_axis + ui->t_offset->value(), m_imag + ui->t_offset->value(), ui->win_size->value());
    curve_abs->setRawSamples(m_x_axis + ui->t_offset->value(), m_abs + ui->t_offset->value(), ui->win_size->value());
    curve_angle->setRawSamples(m_x_axis + ui->t_offset->value(), m_angle + ui->t_offset->value(), ui->win_size->value());
    curve_unwrap->setRawSamples(m_x_axis + ui->t_offset->value(), m_unwrap + ui->t_offset->value(), ui->win_size->value());

    replot();
}

void TD::replot(){
    if(ui->real->isChecked()){
        curve_real->setVisible(true);
    } else {
        curve_real->setVisible(false);
    }

    if(ui->imag->isChecked()){
        curve_imag->setVisible(true);
    } else {
        curve_imag->setVisible(false);
    }

    if(ui->abs->isChecked()){
        curve_abs->setVisible(true);
    } else {
        curve_abs->setVisible(false);
    }

    if(ui->angle->isChecked()){
        curve_angle->setVisible(true);
    } else {
        curve_angle->setVisible(false);
    }

    if(ui->unwrap->isChecked()){
        curve_unwrap->setVisible(true);
    } else {
        curve_unwrap->setVisible(false);
    }

    ui->qwtPlot->setAxisScale(QwtPlot::xBottom, ui->t_offset->value()
                                              , ui->t_offset->value() + ui->win_size->value());

    ui->qwtPlot->replot();
}

void TD::on_real_toggled(bool checked)
{
    replot();
}

void TD::on_imag_toggled(bool checked)
{
    replot();
}

void TD::on_abs_toggled(bool checked)
{
    replot();
}

void TD::on_toolButton_toggled(bool checked)
{
    m_zoomer->setEnabled(checked);
}


void TD::on_update_coords(const QPointF& p_qp){
    ui->x_coord->setNum(p_qp.x());
    ui->y_coord->setNum(p_qp.y());
}

void TD::on_button_zoom_toggled(bool checked)
{
    ui->qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
    ui->qwtPlot->setAxisAutoScale(QwtPlot::yLeft, true);
    update();
}

void TD::on_t_offset_valueChanged(int value)
{

    curve_real->setRawSamples(m_x_axis + value, m_real + value, ui->win_size->value());
    curve_imag->setRawSamples(m_x_axis + value, m_imag + value, ui->win_size->value());
    curve_abs->setRawSamples(m_x_axis + value, m_abs + value, ui->win_size->value());
    curve_angle->setRawSamples(m_x_axis + value, m_angle + value, ui->win_size->value());
    curve_unwrap->setRawSamples(m_x_axis + value, m_unwrap + value, ui->win_size->value());

    replot();


}

void TD::on_win_size_valueChanged(int arg1)
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

    curve_real->setRawSamples(m_x_axis + ui->t_offset->value(), m_real + ui->t_offset->value(), ui->win_size->value());
    curve_imag->setRawSamples(m_x_axis + ui->t_offset->value(), m_imag + ui->t_offset->value(), ui->win_size->value());
    curve_abs->setRawSamples(m_x_axis + ui->t_offset->value(), m_abs + ui->t_offset->value(), ui->win_size->value());
    curve_angle->setRawSamples(m_x_axis + ui->t_offset->value(), m_angle + ui->t_offset->value(), ui->win_size->value());
    curve_unwrap->setRawSamples(m_x_axis + ui->t_offset->value(), m_unwrap + ui->t_offset->value(), ui->win_size->value());

    replot();

}
