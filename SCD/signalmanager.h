#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <complex>

#include "mytypes.h"

class SignalManager
{
public:
    SignalManager(size_t p_max_size);
    ~SignalManager();
    bool open_file(std::string p_filename);

    void backup();
    void restore();
    void apply_changes();

    my_cplx* signal;

    my_cplx* original;
    bool transformed;
    // contains the number of samples in the file
    size_t size;
    size_t original_size;

    // the maximum number of samples to store in memory (prevent overflos)
    size_t mem_max_samples;

    // File
    std::ifstream m_file;
};

#endif // SIGNALMANAGER_H
