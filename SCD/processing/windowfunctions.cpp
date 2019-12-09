#include "windowfunctions.h"

void gen_window(WindowType window_id, my_cplx* mem, size_t size){
    switch (window_id) {
        case WindowType::Rect: // Rechteck
        {
            for (size_t i = 0; i < size; i++) {
              mem[i] = my_cplx(1,0);
            }
        }
        break;
        case WindowType::Hamming: // Hamming
        {
            my_float a0 = 25.0/46.0;
            my_float a1 = 1.0 - a0;

            for (size_t i = 0; i < size; i++) {
              mem[i] = my_cplx(a0 - a1 * std::cos( (2.0*M_PI*i)/(size-1)),0);
            }
            break;
        }
        case WindowType::Blackman: // Blackman
        {
            double alpha = 0.16;
            double a0 = (1.0 - alpha) / 2.0;
            double a1 = 1.0/2.0;
            double a2 = alpha / 2.0;

            for (int i = 0; i < size; i++) {
              mem[i] = my_cplx( a0 - a1 * std::cos( (2.0*M_PI*i)/(size-1))
                                   + a2 * std::cos( (4.0*M_PI*i)/(size-1)), 0);
            }
            break;
        }
        case WindowType::BlackmanHarris: // BlackmanHarris
        {
            double a0 = 0.35875;
            double a1 = 0.48829;
            double a2 = 0.14128;
            double a3 = 0.01168;

            for (int i = 0; i < size; i++) {
              mem[i] = my_cplx(a0 - a1 * std::cos( (2.0*M_PI*i)/(size-1))
                                  + a2 * std::cos( (4.0*M_PI*i)/(size-1))
                                  - a3 * std::cos( (6.0*M_PI*i)/(size-1)), 0);
            }
            break;
        }
    }

    // Normalize
    my_float fac = 0;
    for (size_t i = 0; i < size; ++i) {
        fac += std::abs(mem[i] * mem[i]);
    }
    fac = std::sqrt(size/fac);
//    fac = std::sqrt(fac);

    for (size_t i = 0; i < size; ++i) {
        mem[i] *= fac;
    }

};
