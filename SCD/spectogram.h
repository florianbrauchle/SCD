#ifndef SPECTOGRAM_H
#define SPECTOGRAM_H

#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtWidgets/QSlider>

#include "scd_fam_r_single.h"
#include "mytypes.h"
#include "signalmanager.h"

using namespace QtDataVisualization;

class Spectogram : public QObject
{
    Q_OBJECT
public:
    explicit Spectogram(Q3DSurface *surface, SignalManager* p_sm, size_t p_fft_len);
    ~Spectogram();

    SCD32 m_scd;

    void set_fftlen(size_t new_len);
    size_t set_max_input_len(size_t new_len);
    void set_signal_offset(size_t new_offset);

    void set_log(bool state);
    void set_grid(bool state);
    void set_aspect_ratio(double val);
    void set_scale(double p_min, double p_max);

    void set_cm_midpoint(double p_mid);
    void set_cm(double p_min, double p_max);

    void update_data(); // Für regelmäßige updates mit gleichen aprameterern
    void update_parameters(); // Alles muss neu berechnet werden und speicher neu belegt
    void init_data();

    Q3DSurface *m_graph;

    bool calc_conj = false;

public Q_SLOTS:
    void updatePlotData();
    void updateSignal();


private:
    QSurfaceDataProxy *m_CyclicProxy;
    QSurface3DSeries *m_CyclicSeries;

    QSurfaceDataArray *dataArray; // Speichert die Daten

    SignalManager* m_sm;

    size_t m_max_input_len;
    size_t m_max_offset;
    size_t m_offset;

    size_t m_fft_len;
    size_t m_alpha_len;

    double scale_min;
    double scale_max;
};


#endif // SPECTOGRAM_H
