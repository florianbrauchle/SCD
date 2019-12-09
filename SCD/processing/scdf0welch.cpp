#include "scdf0welch.h"

SCDF0Welch::SCDF0Welch(SignalManager* p_sm): m_sm(p_sm)
{
    // Default Parameter
    m_n_fft = 1024;

    // Speicher für Ergebnisse
    m_win     = nullptr;
    m_fft_in  = nullptr;
    m_fft_out = nullptr;
    m_fft_out_conj = nullptr;

    m_fft_idx  = nullptr;
    m_result   = nullptr;
    m_result_c = nullptr;

    m_f_fft    = nullptr;

    m_fft_plan_fft   = nullptr;

    m_win_type = WindowType::Rect;

    calc_log = false;
    update_parameters();
}

SCDF0Welch::~SCDF0Welch(){
    clear_all();
}

void SCDF0Welch::set_parameters(size_t window_len, WindowType window_id, size_t overlap){
    m_n_fft     = window_len;
    m_win_type  = window_id;
    m_overlap   = overlap;
}

void SCDF0Welch::update_parameters(){
    // Speicher Freigeben, größe könnte sich geändert haben
    clear_all();
    // Festlegen der FFT-Länge
    m_shift = m_n_fft - m_overlap;

    // Speicher reservieren
    m_fft_in  = new my_cplx[m_n_fft];
    m_fft_out = new my_cplx[m_n_fft];
    m_fft_out_conj = new my_cplx[m_n_fft];

    m_result  = new double[m_n_fft];
    m_result_c  = new my_cplx[m_n_fft];

    // Window
    m_win = new my_cplx[m_n_fft];
    gen_window(m_win_type, m_win, m_n_fft);

    // Init fftwf
    // FFT mit Fensterlänge für die erste Transformation
    m_fft_plan_fft   = fftwf_plan_dft_1d(m_n_fft, reinterpret_cast<fftwf_complex*>(m_fft_in), reinterpret_cast<fftwf_complex*>(m_fft_out), FFTW_FORWARD, FFTW_ESTIMATE);

    // Berechnung der Indizies für FFT-Shift
    m_fft_idx = new size_t[m_n_fft];
    size_t idx_start = (m_n_fft + 1)/2;
    for(size_t i=0; i < m_n_fft; i++){
        m_fft_idx[i] = (idx_start + i) % m_n_fft;
    }

    // Berechnung der jeweiligen Frequenzwert
    m_f_fft   = new double[m_n_fft];
    double d_f = 1.0 / m_n_fft;

    // Frequenzliste
    // f_fft =  ((0:N_FFT-1) - floor(N_FFT/2))*fs/N_FFT;
    for (int i = 0; i < m_n_fft; i++) {
      m_f_fft[i] = 2 * (i - std::floor(m_n_fft / 2)) / m_n_fft;
    }

}

void SCDF0Welch::clear_all(){
    delete[] m_win;
    m_win = nullptr;
    delete[] m_fft_in;
    m_fft_in = nullptr;
    delete[] m_fft_out;
    m_fft_out = nullptr;
    delete[] m_fft_out_conj;
    m_fft_out_conj = nullptr;

    delete[] m_result_c;
    m_result_c = nullptr;
    delete[] m_result;
    m_result = nullptr;
    delete[] m_fft_idx;
    m_fft_idx = nullptr;

    delete[] m_f_fft;
    m_f_fft = nullptr;

    fftwf_destroy_plan(m_fft_plan_fft);
    m_fft_plan_fft = nullptr;
}

void SCDF0Welch::calc(){
    size_t start_s = 0;
    size_t cnt = 0;

    for (size_t i=0; i < m_n_fft; ++i) {
        m_result[i] = 0;
        m_result_c[i] = 0;
    }

    while (start_s + m_n_fft < m_sm->size) {
        const float tmp = -2.0*M_PI*start_s/m_n_fft;

        // Fenstern des Eingangssignals
        for (size_t i=0; i < m_n_fft; ++i) {
            m_fft_in[i] = m_sm->signal[start_s+i] * m_win[i];
        }
        // Ausführen der PSD spectrale correlation
        fftwf_execute(m_fft_plan_fft);
        for (size_t i=0; i < m_n_fft; ++i) {
            m_fft_out[i] = m_fft_out[i] * std::exp(my_cplx(0, i*tmp));
        }


        // Berechnung SCD f0
        for (size_t j=0; j < m_n_fft; ++j) {
            m_result_c[j] += m_fft_out[m_fft_idx[j%m_n_fft]] * std::conj(m_fft_out[m_fft_idx[-j%m_n_fft]]);
        }

        // Weiter
        cnt++;
        start_s += m_shift;
    }

    // mittel berecnen
    if(cnt > 0){
//        const double tmp = double(cnt) * double(m_n_fft) * double(m_n_fft);
        const double tmp = double(cnt) * double(m_n_fft);
        for (size_t i=0; i < m_n_fft; ++i) {
            m_result[i] = std::abs(m_result_c[i]) / tmp;
//            m_result[i] = m_result[i] / double(cnt) / double(m_n_fft) / double(m_n_fft));
        }
    }

    // Calc Log if needed
    if(calc_log){
        for (size_t i=0; i < m_n_fft; ++i) {
            m_result[i] = 10*std::log10(m_result[i]);
        }
    }
}

void SCDF0Welch::calc_conj(){
    size_t start_s = 0;
    size_t cnt = 0;

    for (size_t i=0; i < m_n_fft; ++i) {
        m_result[i] = 0;
        m_result_c[i] = 0;
    }

    while (start_s + m_n_fft < m_sm->size) {
        const double tmp = 2.0*M_PI*start_s/m_n_fft;

        // Konjugiertes Eingangssignal
        // Fenstern des Eingangssignals
        for (size_t i=0; i < m_n_fft; ++i) {
            m_fft_in[i] = std::conj(m_sm->signal[start_s+i]) * m_win[i];
        }
        fftwf_execute(m_fft_plan_fft);
        for (size_t i=0; i < m_n_fft; ++i) {
            m_fft_out_conj[i] = m_fft_out[i] * std::exp(my_cplx(0, i*tmp));
        }


        // Eingangssignal // Reihenfolge wichtig da Ergebnis hier direkt aus fft out weiterverwendet wird
        // Fenstern des Eingangssignals
        for (size_t i=0; i < m_n_fft; ++i) {
            m_fft_in[i] = m_sm->signal[start_s+i] * m_win[i];
        }
        fftwf_execute(m_fft_plan_fft);
        for (size_t i=0; i < m_n_fft; ++i) {
            m_fft_out[i] = m_fft_out[i] * std::exp(my_cplx(0, i*tmp));
        }



        // Berechnung der SCD f0
        for (size_t j=0; j < m_n_fft; ++j) {
            m_result_c[j] += m_fft_out[m_fft_idx[j%m_n_fft]] * std::conj(m_fft_out_conj[m_fft_idx[-j%m_n_fft]]);
        }

        // Weiter
        cnt++;
        start_s += m_shift;
    }

    // mittel berecnen
    if(cnt > 0){
        const double tmp =  double(cnt) * double(m_n_fft); // Passt zur achse f/fs
//        const double tmp =  double(cnt) * double(m_n_fft) * double(m_n_fft); // erfüllt parsevall
        for (size_t i=0; i < m_n_fft; ++i) {
            m_result[i] = std::abs(m_result_c[i]) / tmp;
        }
    }

    // Calc Log if needed
    if(calc_log){
        for (size_t i=0; i < m_n_fft; ++i) {
            m_result[i] = 10*std::log10(m_result[i]);
        }
    }
}

double* SCDF0Welch::get_x(){
    return m_f_fft;
}

double* SCDF0Welch::get_y(){
    return m_result;
}
