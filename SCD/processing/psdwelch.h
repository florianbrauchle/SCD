#ifndef PSDWELCH_H
#define PSDWELCH_H

#include "signalmanager.h"
#include "mytypes.h"
#include "windowfunctions.h"

#include <fftw3.h>

#include <iostream>

class PSDWelch
{
public:
    PSDWelch(SignalManager* p_sm);
    ~PSDWelch();

    void calc();

    void set_parameters(size_t window_len, WindowType window_id, size_t overlap);
    void update_parameters();
    void clear_all();

    double* get_x();
    double* get_y();

    // Eingangsparameter
    SignalManager* m_sm;
    bool calc_log;

    // parameters
    size_t m_n_fft;      // FFT Länge
    size_t m_shift;
    size_t windows;
    WindowType m_win_type;
    size_t m_overlap;

    // storage
    my_cplx* m_win;
    my_cplx* m_fft_in;
    my_cplx* m_fft_out;

    double* m_f_fft;
    double* m_result;

    // FFTs
    fftwf_plan m_fft_plan_fft; // Einganswerte Transformieren
    size_t* m_fft_idx;         // Speicher für FFT Shift Indizies

};

#endif // PSDWELCH_H
