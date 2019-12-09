#include "signalmanager.h"


SignalManager::SignalManager(size_t p_max_size){
  // reserviert den Speicher für das Signal
  mem_max_samples = p_max_size * sizeof (my_cplx);
  signal = nullptr;

  // Generates a empty test signal to make sure initialization of everything else doesn't fail
  transformed = false;
  signal = new my_cplx[1024];
  size = 1024;
}

SignalManager::~SignalManager(){
  // Free Memory
  delete[] signal;
  signal = nullptr;
}

bool SignalManager::open_file(std::string p_filename){
  // Try to open file
  m_file.open(p_filename, std::ios::binary);

  // Test if it worked
  if( m_file.is_open() ){
    // Get the File size / SIgnal Length
    std::streampos begin, end;
    begin = m_file.tellg();
    m_file.seekg(0, std::ios::end);
    end = m_file.tellg();

    size = (end - begin) / (2 * sizeof(float));
    std::cout << "size is: " << size << " cfloats.\n";

    // Load only the first batch of samples if signal is too big
    if( size > mem_max_samples ){
        size = mem_max_samples;
        std::cout << "limit reached!" << std::endl;
    }

    signal = new my_cplx[size];

    m_file.seekg(0, std::ios::beg);
    float real, imag;
    for (size_t i = 0; i < size; i++) {
      m_file.read(reinterpret_cast<char *>(&real), sizeof(real));
      m_file.read(reinterpret_cast<char *>(&imag), sizeof(imag));
      signal[i] = my_cplx(real, imag);
    }

    m_file.close();
    transformed = false;
    return true;
  } else {
    return false;
  }
}

void SignalManager::backup(){
    if(!transformed){
        original = new my_cplx[size];
        for (size_t i=0;i<size;++i) {
            original[i] = signal[i];
        }
        transformed = true;
    }
}

void SignalManager::restore(){
    if(transformed){
        for (size_t i=0;i<size;++i) {
            signal[i] = original[i];
        }
    }
}

void SignalManager::apply_changes(){
    transformed = false;
}
