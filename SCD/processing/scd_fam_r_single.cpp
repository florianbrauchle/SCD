#include "scd_fam_r_single.h"
#include <iostream>
#include <chrono>
#include <immintrin.h>

SCD32::~SCD32(){
  clear_all();
}

SCD32::SCD32(my_cplx* in, size_t len, size_t window_len): m_in(in), m_in_len(len), m_L(window_len)
{
  // Speicher für Ergebnisse
  m_in_win  = nullptr;
  m_fft_out = nullptr;
  m_In      = nullptr;  // Fourier Transformierte des Eingangssignals
  m_In_I    = nullptr;  // Fourier Transformierte des Eingangssignals
  m_In_R    = nullptr;  // Fourier Transformierte des Eingangssignals
  m_In_I_conj = nullptr;  // Fourier Transformierte des Eingangssignals
  m_In_R_conj = nullptr;  // Fourier Transformierte des Eingangssignals
  m_win     = nullptr;

  m_fft_idx  = nullptr;
  m_alpha    = nullptr; // Alpha Werte
  m_frequenz = nullptr; // Frequenz Werte
  m_scd      = nullptr; // Spectrale Korrelation

  m_fft_plan_fft   = nullptr;
  m_fs = 1.0;

  m_window_id = 0;

  update_parameters();
}

void SCD32::calc(){
  // Eingangssignal Fenstern
  #pragma omp parallel for
  for (size_t i = 0; i < m_P; i++) {
    for (size_t j = 0; j < m_n_fft; j++) {
      m_in_win[i*m_n_fft + j] = m_in[i*m_L + j] * m_win[j];
    }
  }

  // Berechnung der FFTs der Eingangssignal Fenster
  #pragma omp parallel for
  for (size_t i = 0; i < m_P; i++) {
    fftwf_execute_dft(
        m_fft_plan_fft,
        reinterpret_cast<fftwf_complex*>(&m_in_win[i*m_n_fft]), // Eingangsdaten liegen im Speicher einfach nacheinander i*m_L
        reinterpret_cast<fftwf_complex*>(&m_In[i*m_n_fft])       // Ausgangsspeicher
    );
  }

  // Phasen anpassen
  //#pragma omp parallel for
  for (size_t i = 0; i < m_P; i++) {
    const my_float tmp = 2.0 * M_PI * i * m_L / m_n_fft;
    for (size_t j = 0; j < m_n_fft; j++) {
        m_In[i*m_n_fft + j] = m_In[i*m_n_fft + j] *
                std::exp( std::complex<my_float>(0, j*tmp));
    }
  }

  // FFTSHIFT und Berechnung der konjugierten Werte
  #pragma omp parallel
  for (size_t i = 0; i < m_P; i++) { // alle fenster durchgehen
    for (size_t j = 0; j < m_n_fft; j++) { // alle FFT werte durchgehen
        m_In_R[j*m_P+i] = std::real(m_In[i*m_n_fft + m_fft_idx[j]]); // Matrix invertiert für schnellere speicher?
        m_In_I[j*m_P+i] = std::imag(m_In[i*m_n_fft + m_fft_idx[j]]); // Matrix invertiert für schnellere speicher?
    }
  }

  // Iteration durch die FFT Frequenz Kombinationen f1 f1, f1 f2, ..
  const my_float tmp_f = my_float(m_n_fft) * my_float(m_P); // passend zur achsenbeschriftung
//  const my_float tmp_f = my_float(m_n_fft) * my_float(m_n_fft) * my_float(m_P); // erfüllt parsevall
  #pragma omp parallel for
  for (size_t j = 0; j < m_n_fft; j++) {
    for (size_t i = 0; i < m_n_fft; i++) { // Schleifen invertiert damit Ergenisse Matrix der Reihe nach entsteht
      my_float tmp_r = 0;
      my_float tmp_i = 0;
      for (size_t k = 0; k < m_P; k++) {
        tmp_r += m_In_R[i*m_P+k] * m_In_R[j*m_P+k];
        tmp_r += m_In_I[i*m_P+k] * m_In_I[j*m_P+k];

        tmp_i -= m_In_R[i*m_P+k] * m_In_I[j*m_P+k];
        tmp_i += m_In_I[i*m_P+k] * m_In_R[j*m_P+k];
      }
      m_scd[j*m_n_fft+i] = my_cplx(tmp_r, tmp_i) / tmp_f;
    }
  }

}

void SCD32::calc_conj(){
    // Eingangssignal Fenstern
    #pragma omp parallel for
    for (size_t i = 0; i < m_P; i++) {
      for (size_t j = 0; j < m_n_fft; j++) {
        m_in_win[i*m_n_fft + j] = m_in[i*m_L + j] * m_win[j];
      }
    }
    // Berechnung der FFTs der Eingangssignal Fenster
    #pragma omp parallel for
    for (size_t i = 0; i < m_P; i++) {
      fftwf_execute_dft(
          m_fft_plan_fft,
          reinterpret_cast<fftwf_complex*>(&m_in_win[i*m_n_fft]), // Eingangsdaten liegen im Speicher einfach nacheinander i*m_L
          reinterpret_cast<fftwf_complex*>(&m_In[i*m_n_fft])       // Ausgangsspeicher
      );
    }
    // Phasen anpassen
    //#pragma omp parallel for
    for (size_t i = 0; i < m_P; i++) {
      const my_float tmp = 2.0 * M_PI * i * m_L / m_n_fft;
      for (size_t j = 0; j < m_n_fft; j++) {
        m_In[i*m_n_fft + j] = m_In[i*m_n_fft + j]
                * std::exp( std::complex<my_float>(0, j * tmp ));
      }
    }
    // FFTSHIFT und Berechnung der konjugierten Werte
    #pragma omp parallel
    for (size_t i = 0; i < m_P; i++) { // alle fenster durchgehen
      for (size_t j = 0; j < m_n_fft; j++) { // alle FFT werte durchgehen
          m_In_R[j*m_P+i] = std::real(m_In[i*m_n_fft + m_fft_idx[j]]); // Matrix invertiert für schnellere speicher?
          m_In_I[j*m_P+i] = std::imag(m_In[i*m_n_fft + m_fft_idx[j]]); // Matrix invertiert für schnellere speicher?
      }
    }



    // Eingangssignal Fenstern
    #pragma omp parallel for
    for (size_t i = 0; i < m_P; i++) {
      for (size_t j = 0; j < m_n_fft; j++) {
        m_in_win[i*m_n_fft + j] = std::conj(m_in[i*m_L + j]) * m_win[j];
      }
    }
    // Berechnung der FFTs der Eingangssignal Fenster
    #pragma omp parallel for
    for (size_t i = 0; i < m_P; i++) {
      fftwf_execute_dft(
          m_fft_plan_fft,
          reinterpret_cast<fftwf_complex*>(&m_in_win[i*m_n_fft]), // Eingangsdaten liegen im Speicher einfach nacheinander i*m_L
          reinterpret_cast<fftwf_complex*>(&m_In[i*m_n_fft])       // Ausgangsspeicher
      );
    }
    // Phasen anpassen
    //#pragma omp parallel for
    for (size_t i = 0; i < m_P; i++) {
      const my_float tmp = 2.0 * M_PI * i * m_L / m_n_fft;
      for (size_t j = 0; j < m_n_fft; j++) {
          m_In[i*m_n_fft + j] = m_In[i*m_n_fft + j] *
                  std::exp( std::complex<my_float>(0, j*tmp));
      }
    }
    // FFTSHIFT und Berechnung der konjugierten Werte
    #pragma omp parallel
    for (size_t i = 0; i < m_P; i++) { // alle fenster durchgehen
      for (size_t j = 0; j < m_n_fft; j++) { // alle FFT werte durchgehen
          m_In_R_conj[j*m_P+i] = std::real(m_In[i*m_n_fft + m_fft_idx[j]]); // Matrix invertiert für schnellere speicher?
          m_In_I_conj[j*m_P+i] = std::imag(m_In[i*m_n_fft + m_fft_idx[j]]); // Matrix invertiert für schnellere speicher?
      }
    }



    // Iteration durch die FFT Frequenz Kombinationen f1 f1, f1 f2, ..
    const my_float tmp_f = my_float(m_n_fft) * my_float(m_P); // passend zur achsenbeschriftung
  //  const my_float tmp_f = my_float(m_n_fft) * my_float(m_n_fft) * my_float(m_P); // erfüllt parsevall
    #pragma omp parallel for
    for (size_t j = 0; j < m_n_fft; j++) {
      for (size_t i = 0; i < m_n_fft; i++) { // Schleifen invertiert damit Ergenisse Matrix der Reihe nach entsteht
        my_float tmp_r = 0;
        my_float tmp_i = 0;
        for (size_t k = 0; k < m_P; k++) {
            tmp_r += m_In_R[i*m_P+k] * m_In_R_conj[j*m_P+k];
            tmp_r += m_In_I[i*m_P+k] * m_In_I_conj[j*m_P+k];

            tmp_i -= m_In_R[i*m_P+k] * m_In_I_conj[j*m_P+k];
            tmp_i += m_In_I[i*m_P+k] * m_In_R_conj[j*m_P+k];
        }
        m_scd[j*m_n_fft+i] = my_cplx(tmp_r, tmp_i) / tmp_f;
      }
    }
}

void SCD32::gen_window(size_t window_id){
    // Window
    delete[] m_win;
    m_win = new my_cplx[m_n_fft];

    switch (window_id) {
        case 0: // Rechteck
        {
            for (size_t i = 0; i < m_n_fft; i++) {
              m_win[i] = my_cplx(1,0);
            }
        }
        break;
        case 1: // Hamming
        {
            my_float a0 = 25.0/46.0;
            my_float a1 = 1.0 - a0;

            for (size_t i = 0; i < m_n_fft; i++) {
              m_win[i] = my_cplx(a0 - a1 * std::cos( (2.0*M_PI*i)/(m_n_fft-1)),0);
            }
            break;
        }
        case 2: // Blackman
        {
            double alpha = 0.16;
            double a0 = (1.0 - alpha) / 2.0;
            double a1 = 1.0/2.0;
            double a2 = alpha / 2.0;

            for (int i = 0; i < m_n_fft; i++) {
              m_win[i] = my_cplx(a0 - a1 * std::cos( (2.0*M_PI*i)/(m_n_fft-1)) + a2 * std::cos( (4.0*M_PI*i)/(m_n_fft-1)),0);
            }
            break;
        }
        case 3: // BlackmanHarris
        {
            double a0 = 0.35875;
            double a1 = 0.48829;
            double a2 = 0.14128;
            double a3 = 0.01168;

            for (int i = 0; i < m_n_fft; i++) {
              m_win[i] = my_cplx(a0 - a1 * std::cos( (2.0*M_PI*i)/(m_n_fft-1)) + a2 * std::cos( (4.0*M_PI*i)/(m_n_fft-1)) - a3 * std::cos( (6.0*M_PI*i)/(m_n_fft-1)),0);
            }
            break;
        }
    }


    // Normalize
    my_float fac = 0;
    for (size_t i = 0; i < m_n_fft; ++i) {
        fac += std::abs(m_win[i] * m_win[i]);
    }
    fac = std::sqrt(m_n_fft/fac);

    for (size_t i = 0; i < m_n_fft; ++i) {
        m_win[i] *= fac;
    }
}

void SCD32::clear_all(){
    delete[] m_in_win;
    delete[] m_win;
    delete[] m_fft_idx;
    delete[] m_fft_out;
    delete[] m_In_R;
    delete[] m_In_I_conj;
    delete[] m_In_R_conj;
    delete[] m_In_I;
    delete[] m_alpha;
    delete[] m_frequenz;
    delete[] m_scd;


    m_in_win = nullptr;
    m_win = nullptr;
    m_fft_idx = nullptr;
    m_fft_out = nullptr;
    m_In_R = nullptr;
    m_In_I = nullptr;
    m_In_R_conj = nullptr;
    m_In_I_conj = nullptr;
    m_alpha = nullptr;
    m_frequenz = nullptr;
    m_scd = nullptr;

    // Zerstöre Pläne
    fftwf_destroy_plan(m_fft_plan_fft);
}

void SCD32::console_get_parameters(){
    std::cout << " .Input Size     : " << m_in_len << " Samples\n";
    std::cout << " .Window Size (L): " << m_L << " Samples\n";
    std::cout << " .FFT Size (N')  : " << m_n_fft << " Samples\n";
    std::cout << " .Windows (P)    : " << m_P << "\n\n";

    std::cout << " .d_alpha : " << 1.0/(m_n_fft) << std::endl;
    std::cout << " .d_freq  : " << 2.0/(m_n_fft) << std::endl;
    std::cout << " .dfda    : " << 2.0/(m_n_fft*m_n_fft) << "\n\n";

    // matrix am ende + fft berechnung + conj berechnung
    size_t flops = m_P * m_n_fft * m_n_fft + m_P * m_n_fft * std::log(m_n_fft) + m_P * m_n_fft;

    std::cout << " Est. Flop: " << flops << std::endl;
    std::cout << " Big Matrix Memory: " << sizeof(std::complex<my_float>) * m_n_fft * m_P / 1e6 << " MB" << std::endl;

    std::cout << " " << m_n_fft * m_n_fft << " SCD Values\n" << std::endl;

}

void SCD32::update_parameters(){
  // Speicher Freigeben, größe könnte sich geändert haben
  clear_all();
  // Festlegen der FFT-Länge
  m_n_fft = m_L * 4;
  // Berechnung der Fenster
  m_P = size_t( (m_in_len - m_n_fft + m_L) / m_L); // C++ default behavior -> truncating towards zero / abrunden

  // Speicher reservieren
  m_in_win  = new my_cplx[m_P * m_n_fft];
  m_fft_out = new my_cplx[m_P * m_n_fft];
  m_In      = new my_cplx[m_P * m_n_fft];

  m_In_I    = new my_float[m_P * m_n_fft];
  m_In_R    = new my_float[m_P * m_n_fft];

  m_In_I_conj = new my_float[m_P * m_n_fft];
  m_In_R_conj = new my_float[m_P * m_n_fft];

  // Window
  gen_window(m_window_id);

  //Alpha Werte
  // Für jede Kombination fft_len**2 ergeben sich m_n_window Alphas
  size_t n_fft_comb = m_n_fft*m_n_fft;

  m_alpha    = new my_float[n_fft_comb];
  m_frequenz = new my_float[n_fft_comb];
  m_scd      = new my_cplx[n_fft_comb];

  // Init fftwf
  // FFT mit Fensterlänge für die erste Transformation
  m_fft_plan_fft   = fftwf_plan_dft_1d(m_n_fft, reinterpret_cast<fftwf_complex*>(m_in), reinterpret_cast<fftwf_complex*>(m_fft_out), FFTW_FORWARD, FFTW_ESTIMATE);

  // Berechnung der Indizies für FFT-Shift
  m_fft_idx = new size_t[m_n_fft];
  size_t idx_start = (m_n_fft + 1)/2;
  for(size_t i=0; i < m_n_fft; i++){
      m_fft_idx[i] = (idx_start + i) % m_n_fft;
  }

  // Berechnung der jeweiligen Alpha und Frequenzwert
  my_float* m_f_fft   = new my_float[m_n_fft];
  my_float d_f = m_fs / m_n_fft;

  // Frequenzliste
  // f_fft =  ((0:N_FFT-1) - floor(N_FFT/2))*fs/N_FFT;
  for (int i = 0; i < m_n_fft; i++) {
    m_f_fft[i] = (i - std::floor(m_n_fft / 2)) * m_fs / m_n_fft;
  }

  // Berechnung der Alpha Werte
  for (size_t i = 0; i < m_n_fft; i++) {
    for (size_t j = 0; j < m_n_fft; j++) {
      m_frequenz[(i*m_n_fft)+j] = (m_f_fft[i] + m_f_fft[j]) / 2.0;
      m_alpha[(i*m_n_fft)+j]    =  m_f_fft[i] - m_f_fft[j];
    }
  }

  delete[] m_f_fft;
}

void SCD32::set_parameters(my_cplx* in, size_t len, size_t window_len){
  m_in  = in;
  m_in_len = len;
  m_L = window_len;

  update_parameters();
}

void SCD32::set_fftlen(size_t new_fft_len){
  m_L = new_fft_len / 4;
  update_parameters();
}

void SCD32::set_in_buffer(my_cplx* new_in, size_t new_len){
    m_in_len = new_len;
    m_in = new_in;

    update_parameters();
}

my_float SCD32::get_xy(size_t x, size_t y){
    if(!apply_log){
        return std::abs(m_scd[x * m_n_fft + y]);
    } else {
        return 10 * std::log10(std::abs(m_scd[x * m_n_fft + y]));
    }
}

my_float SCD32::get_f_xy(size_t x, size_t y){
    return m_frequenz[x*m_n_fft+y];
}

my_float SCD32::get_a_xy(size_t x, size_t y){
    return m_alpha[x*m_n_fft+y];
}

void SCD32::set_log(bool state){
    apply_log = state;
}

void SCD32::set_window(size_t win_id){
    m_window_id = win_id;
    gen_window(win_id);
}
