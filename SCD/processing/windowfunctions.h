#ifndef WINDOWFUNCTIONS_H
#define WINDOWFUNCTIONS_H

#include "mytypes.h"

enum WindowType{
    Rect,
    Hamming,
    Blackman,
    BlackmanHarris
};

void gen_window(WindowType window_id, my_cplx* mem, size_t size);


#endif // WINDOWFUNCTIONS_H
