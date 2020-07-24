#include "waterfall.h"
#include "ui_waterfall.h"
#include <QPixmap>

Waterfall::Waterfall(SignalManager* p_sm) :
    ui(new Ui::Waterfall),
    m_sm(p_sm)
{
    ui->setupUi(this);

    m_waterfall = nullptr;

    m_fft_idx  = nullptr;
    m_fft_plan_fft   = nullptr;
    m_fft_in   = nullptr;
    m_fft_out  = nullptr;

    m_win = nullptr;

    m_win_type = WindowType::Rect;

    m_fft_len = 1024;
    m_overlap = 0;

    m_scale = 1;

    calc_log = false;
    on_pushButton_clicked();
}

Waterfall::~Waterfall()
{
    clear_all();
    delete ui;
}

void Waterfall::clear_all(){
    delete m_waterfall;
    m_waterfall = nullptr;

    delete[] m_fft_in;
    m_fft_in = nullptr;
    delete[] m_fft_out;
    m_fft_out = nullptr;
    delete[] m_fft_idx;
    m_fft_idx = nullptr;
    delete[] m_win;
    m_win = nullptr;

    fftwf_destroy_plan(m_fft_plan_fft);
    m_fft_plan_fft = nullptr;
}

void Waterfall::update(){
    clear_all();

    if(calc_log){
        min = ui->min_db->value();
        max = ui->max_db->value();
    } else {
        min = std::pow(10.0, ui->min_db->value()/10.0);
        max = std::pow(10.0, ui->max_db->value()/10.0);
    }

    m_fft_in       = new my_cplx[m_fft_len];
    m_fft_out      = new my_cplx[m_fft_len];
    m_fft_plan_fft = fftwf_plan_dft_1d(m_fft_len, reinterpret_cast<fftwf_complex*>(m_fft_in), reinterpret_cast<fftwf_complex*>(m_fft_out), FFTW_FORWARD, FFTW_ESTIMATE);

    // Berechnung der Indizies für FFT-Shift
    m_fft_idx = new size_t[m_fft_len];
    size_t idx_start = (m_fft_len + 1)/2;
    for(size_t i=0; i < m_fft_len; i++){
        m_fft_idx[i] = (idx_start + i) % m_fft_len;
    }


    // Input Size
    m_shift = m_fft_len - m_overlap;
    m_n_windows = (m_sm->size - m_fft_len) / m_shift + 1;

    m_waterfall = new QImage(m_fft_len, m_n_windows, QImage::Format_RGB32);

    // Generate Window
    m_win = new my_cplx[m_fft_len];
    gen_window(m_win_type, m_win, m_fft_len);

    for (size_t i = 0; i < m_n_windows; ++i) {
        // Calc FFT
        for (size_t p=0; p < m_fft_len; ++p) {
            m_fft_in[p] = m_win[p] * m_sm->signal[i*m_shift + p];
            m_fft_in[p] = m_fft_in[p] / my_float(m_fft_len);
        }

        fftwf_execute(m_fft_plan_fft);
        // Store color values...
        for (size_t p=0; p < m_fft_len; ++p) {
            m_waterfall->setPixel(p, i%m_n_windows, my_cmap(std::abs(m_fft_out[m_fft_idx[p]])));
        }
    }
    ui->waterfall->setPixmap(QPixmap::fromImage(*m_waterfall));
}


//QRgb Waterfall::my_cmap(my_float in){
//    in = in * in;
//    if(calc_log){
//        in = 10*std::log10(in) + 100;
//        in = in / 100;
//    }

//    in = m_scale * in;

//    if(in < 0){
//        return qRgb(0,0,0);
//    } else if (in > 1) {
//        return qRgb(64, 148, 196);
//    } else {
//        return qRgb(64 * in, 148 * in, 196 * in);
//    }
//}


QRgb Waterfall::my_cmap(my_float in){
    in = in * in;

    if(calc_log){
        in = 10*std::log10(in);
    }

    in = (in - min) / (max - min);

    // 16 6 50
    // 204 30 112   delta: 188 24 62
    // 235 230 205  delta: 31 200 93

    if(in < 0){
        return qRgb(16, 6, 50);
    } else if (in > 1) {
//        return qRgb(64, 148, 196);
        return qRgb(235, 230, 205);
    } else if ( in < 0.5 ) {
        return qRgb(16 + 188 * 2 * in, 6 + 24 * 2 * in , 50 + 62 * 2 * in);
    } else {
        return qRgb(204 + (in - 0.5) * 2 * 31, 30 + (in - 0.5) * 2 * 200, 112 + (in - 0.5) * 2 * 93);
    }
}



void Waterfall::on_amp_scale_valueChanged(double arg1)
{
    m_scale = arg1;
    max = arg1;
    update();
}

void Waterfall::on_fftlen_currentIndexChanged(int index)
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
        ui->overlap->setValue(m_overlap);
        ui->overlap->setMaximum(m_fft_len - 1);
    } else {
        ui->overlap->setMaximum(m_fft_len - 1);
        m_overlap = m_fft_len / 2;
        ui->overlap->setValue(m_overlap);
    }

    update();
}

void Waterfall::on_overlap_valueChanged(int arg1)
{
    m_overlap = arg1;
    update();
}

void Waterfall::on_window_function_currentIndexChanged(int index)
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

    update();
}

void Waterfall::on_button_log_clicked(bool checked)
{
    calc_log = checked;
    update();
}

void Waterfall::on_min_db_valueChanged(int arg1)
{
    update();
}

void Waterfall::on_max_db_valueChanged(int arg1)
{
    update();
}

void Waterfall::on_pushButton_clicked()
{
    // Calculate Signal Power
    float s_power = 0;
    for (size_t i = 0; i < m_sm->size; i++) {
        s_power +=std::norm(m_sm->signal[i]);
    }
    s_power = s_power / m_sm->size / m_fft_len;
    s_power = 10 * log10(s_power);
    std::cout << s_power << std::endl;
    ui->min_db->setValue(s_power - 30);
    ui->max_db->setValue(s_power + 10);
    update();
}
