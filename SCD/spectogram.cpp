#include "spectogram.h"

#include <random>

#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DCamera>
#include <QtCore/qmath.h>
#include <QtDataVisualization/Q3DTheme>

using namespace QtDataVisualization;

Spectogram::Spectogram(Q3DSurface *surface, SignalManager* p_sm, size_t p_fft_len)
    : m_scd(p_sm->signal, p_sm->size, p_fft_len), m_graph(surface), m_sm(p_sm)
{
    // Graph Settings
    m_graph->activeTheme()->setType(Q3DTheme::Theme(3));
    m_graph->setOrthoProjection(true);

    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetDirectlyAbove);
    m_graph->scene()->activeCamera()->setXRotation(-90);

    m_graph->setAxisX(new QValue3DAxis);
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->setAxisZ(new QValue3DAxis);

    m_graph->axisX()->setRange(-1 , 1 );
    m_graph->axisY()->setRange(0 , 1 );
    m_graph->axisZ()->setRange(-0.5 , 0.5 );

    m_graph->axisX()->setLabelFormat("%.1f");
    m_graph->axisZ()->setLabelFormat("%.1f");
    m_graph->axisX()->setLabelAutoRotation(30);
    m_graph->axisY()->setLabelAutoRotation(90);
    m_graph->axisZ()->setLabelAutoRotation(30);

    //m_graph->activeTheme()->setGridEnabled(false);

    m_graph->axisZ()->setTitle(QStringLiteral("Frequenz [1/fs]"));
    m_graph->axisX()->setTitle(QStringLiteral("Alpha [1/fs]"));
    m_graph->axisY()->setTitle(QStringLiteral("Cyclic Spectrum"));
    m_graph->axisX()->setTitleVisible(true);
    m_graph->axisY()->setTitleVisible(true);
    m_graph->axisZ()->setTitleVisible(true);


    m_CyclicProxy  = new QSurfaceDataProxy();
    m_CyclicSeries = new QSurface3DSeries(m_CyclicProxy);

    m_fft_len   = 256;
    m_alpha_len = 256;

    dataArray = nullptr;

    init_data();
    m_CyclicSeries->setFlatShadingEnabled(true);
    m_graph->addSeries(m_CyclicSeries);

    // Plottet das überlappende mesh nicht
    m_CyclicSeries->setDrawMode(QSurface3DSeries::DrawFlag::DrawSurface);

    // Lights
    //m_graph->
    m_graph->activeTheme()->setLightStrength(1);
    //m_graph->activeTheme()->lightStrength(10);
    m_graph->activeTheme()->setAmbientLightStrength(1);
    m_graph->activeTheme()->setHighlightLightStrength(0);

    scale_min = 0;
    scale_max = 100;

    // Define Gradient
    QLinearGradient grAtoB1(0, 0, 100, 100);
    grAtoB1.setColorAt(0.0, Qt::darkBlue);
    grAtoB1.setColorAt(0.1, Qt::yellow);
    grAtoB1.setColorAt(0.5, Qt::red);
    grAtoB1.setColorAt(0.9, Qt::darkRed);
    grAtoB1.setColorAt(1.0, Qt::darkBlue);

    QLinearGradient grAtoB2(0, 0, 100, 100);
    grAtoB2.setColorAt(0.0,  QColor(0, 43, 94));
    grAtoB2.setColorAt(0.25, QColor(64, 148, 196));
    grAtoB2.setColorAt(0.5,  QColor(247, 247, 247));
    grAtoB2.setColorAt(0.75, QColor(214, 97, 78));
    grAtoB2.setColorAt(1.0,  QColor(163, 0, 33));

    QLinearGradient grAtoB3(0, 0, 100, 100);
    grAtoB3.setColorAt(0.0, QColor(0, 43, 94));
    grAtoB3.setColorAt(0.1, QColor(64, 148, 196));
    grAtoB3.setColorAt(0.2, QColor(247, 247, 247));
    grAtoB3.setColorAt(0.6, QColor(214, 97, 78));
    grAtoB3.setColorAt(1.0, QColor(163, 0, 33));

    QLinearGradient grAtoB4(0, 0, 100, 100);
    grAtoB4.setColorAt(0.000, QColor(0, 43, 94));
    grAtoB4.setColorAt(0.01, QColor(64, 148, 196));
    grAtoB4.setColorAt(0.05, QColor(247, 247, 247));
    grAtoB4.setColorAt(0.09, QColor(214, 97, 78));
    grAtoB4.setColorAt(0.100, QColor(163, 0, 33));
    grAtoB4.setColorAt(0.2, QColor(214, 97, 78));
    grAtoB4.setColorAt(0.6, QColor(247, 247, 247));
    grAtoB4.setColorAt(1.0, QColor(64, 148, 196));

    QLinearGradient grAtoB5(scale_min, scale_min, scale_max, scale_max);
    grAtoB5.setColorAt(0.00, QColor(16, 6, 50));
    grAtoB5.setColorAt(0.5, QColor(204, 30, 112));
    grAtoB5.setColorAt(1, QColor(235, 230, 205));

    m_graph->seriesList().at(0)->setBaseGradient(grAtoB5);
    m_graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
    m_graph->setHorizontalAspectRatio(1);
}

Spectogram::~Spectogram()
{

}

void Spectogram::init_data(){
    if(!calc_conj){
        m_scd.calc();
    } else {
        m_scd.calc_conj();
    }

    // delete old data
    if(dataArray != nullptr){
        dataArray->clear();
    } else {
        dataArray = new QSurfaceDataArray;
    }
    dataArray->reserve(m_fft_len);
    for (int i = 0 ; i < m_fft_len ; i++) {
        QSurfaceDataRow *newRow = new QSurfaceDataRow(m_fft_len);
        int index = 0;
        for (int j = 0; j < m_fft_len; j++) {
            float x = m_scd.get_a_xy(i,j); // Frequenzen auf X-Achse
            float z = m_scd.get_f_xy(i,j); // Alphas auf y-Achse
            float y = m_scd.get_xy(j,i);
            (*newRow)[index++].setPosition(QVector3D(x, y, z));
        }
        *dataArray << newRow;
    }
    m_CyclicProxy->resetArray(dataArray);
}

void Spectogram::set_fftlen(size_t new_len){
    m_fft_len = new_len;
    m_scd.set_fftlen(new_len);
    init_data();
}

void Spectogram::update_data(){
    if(!calc_conj){
        m_scd.calc();
    } else {
        m_scd.calc_conj();
    }

    for (size_t i = 0 ; i < m_fft_len ; i++) {
        for (size_t j = 0; j < m_fft_len; j++) {
            (*dataArray->at(i))[j].setY(m_scd.get_xy(j, i));
        }
    }



    m_CyclicProxy->arrayReset(); // Updates the Plot
}

void Spectogram::updatePlotData(){
    update_data();
}

void Spectogram::updateSignal(){
    m_scd.set_in_buffer(m_sm->signal, m_sm->size);
    set_max_input_len(m_max_input_len);

    update_data();
}


void Spectogram::set_scale(double p_min, double p_max){
    scale_min = p_min;
    scale_max = p_max;
    m_graph->axisY()->setRange(scale_min, scale_max);
}


void Spectogram::set_cm_midpoint(double p_mid){
    // Update Colormap
    QLinearGradient grAtoB5(scale_min, scale_min, scale_max, scale_max);
    grAtoB5.setColorAt(0, QColor(16, 6, 50));
    grAtoB5.setColorAt(p_mid, QColor(204, 30, 112));
    grAtoB5.setColorAt(1, QColor(235, 230, 205));

    m_graph->seriesList().at(0)->setBaseGradient(grAtoB5);
}


void Spectogram::set_cm(double p_min, double p_max){
    // Update Colormap
    double midpoint = p_min + (p_max - p_min) / 2;

    QLinearGradient grAtoB5(scale_min, scale_min, scale_max, scale_max);
    grAtoB5.setColorAt(0, QColor(16, 6, 50));
    grAtoB5.setColorAt(p_min, QColor(16, 6, 50));
    grAtoB5.setColorAt(midpoint, QColor(204, 30, 112));
    grAtoB5.setColorAt(p_max, QColor(235, 230, 205));
    grAtoB5.setColorAt(1, QColor(235, 230, 205));

    m_graph->seriesList().at(0)->setBaseGradient(grAtoB5);
}


size_t Spectogram::set_max_input_len(size_t new_len){
    // Calculate the necessary Samples to get n windows
    m_max_input_len = (new_len + 3) * m_fft_len / 4; // Sonst enthält das letzte Fenster nur 1 Frame

    if(m_max_input_len > m_sm->size){
        m_max_input_len = m_sm->size;
        m_max_offset = 0;
    } else {
        m_max_offset = m_sm->size - m_max_input_len;
        m_offset = 0;

        m_scd.set_in_buffer(m_sm->signal + m_offset, m_max_input_len);
        update_data();
    }

    return m_max_offset;
}

void Spectogram::set_signal_offset(size_t new_offset){
    if( new_offset < m_max_offset){
        m_scd.set_in_buffer(m_sm->signal+new_offset, m_max_input_len);
    } else {
        m_scd.set_in_buffer(m_sm->signal+new_offset, m_max_input_len);
    }
    update_data();
}

void Spectogram::set_grid(bool state){
    if(state){
        m_CyclicSeries->setDrawMode(QSurface3DSeries::DrawFlag::DrawSurfaceAndWireframe);
    }else{
        m_CyclicSeries->setDrawMode(QSurface3DSeries::DrawFlag::DrawSurface);
    }
}

void Spectogram::set_log(bool state){
        m_scd.set_log(state);
        m_graph->axisY()->setRange(scale_min, scale_max);
//        if(state){
//            m_graph->axisY()->setRange(-10, 0);
//        } else {
//            m_graph->axisY()->setRange(0, 1);
//        }
}

void Spectogram::set_aspect_ratio(double val){
    m_graph->setHorizontalAspectRatio(val);
}

