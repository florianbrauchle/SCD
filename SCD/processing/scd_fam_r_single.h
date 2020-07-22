#ifndef __SCD__
#define __SCD__

#include <complex>
#include <fftw3.h>

#include <iostream>
#include "mytypes.h"

class SCD32 {
private:
  // Eingangsparameter
  my_cplx* m_in;         // Eingangssignal
  my_cplx* m_in_win;     // Eingangssignal Gefenstert
  size_t m_in_len;        // Länge des Eingangssignals
  size_t m_L;             // Fensterlänge, nochmal prüfen speichert im moment die offsets zwischen zwei Fenster/ Fensterlänge - Overlap
  size_t m_P;             // Anzahl der FFTs die durchgeführt werden

//  size_t m_max_P;
//  bool m_use_full = true;

  size_t m_n_fft;      // FFT Länge

  // Sample Rate
  my_float m_fs;

  // Zusätzliche Werte
  my_cplx* m_fft_out;
  my_cplx* m_In;       // Fourier Transformierte des Eingangssignals

  my_float* m_In_R;       // Fourier Transformierte des Eingangssignals
  my_float* m_In_I;       // Fourier Transformierte des Eingangssignals

  my_float* m_In_R_conj;       // Fourier Transformierte des Eingangssignals
  my_float* m_In_I_conj;       // Fourier Transformierte des Eingangssignals

  my_cplx* m_win;      // Speicher für die Fensterfunktion
  size_t m_window_id;
  void gen_window(size_t window_id);

  // Speicher für Ergebnisse
  my_cplx* m_scd;     // Spectrale Korrelation

  my_float* m_alpha;    // Alpha Werte
  my_float* m_frequenz; // Frequenz Werte

  // FFTs
  fftwf_plan m_fft_plan_fft; // Einganswerte Transformieren
  // Speicher für FFT Shift Indizies
  size_t* m_fft_idx;

  // Output
  bool apply_log = true;

public:
 SCD32(my_cplx* in, size_t len, size_t p_fft_len);
 ~SCD32();

 void update_parameters();
 void set_parameters(my_cplx* in, size_t len, size_t window_len);
 void set_log(bool state);

 void clear_all();

 void calc();
 void calc_conj();
 void set_fftlen(size_t new_fft_len);
 void set_in_buffer(my_cplx* new_in, size_t new_len);

 void set_window(size_t win_id);

 my_float get_xy(size_t x, size_t y);
 my_float get_a_xy(size_t x, size_t y);
 my_float get_f_xy(size_t x, size_t y);

 void console_get_parameters();
};

#endif // SPECTOGRAM_H
