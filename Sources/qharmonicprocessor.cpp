#include "qharmonicprocessor.h"

#ifdef MINGW_COMPILER
#include <cmath>
#endif

QHarmonicProcessor::QHarmonicProcessor(QObject *parent, quint16 length_of_data, quint16 length_of_buffer) :
                    QObject(parent),
                    m_datalength(length_of_data),
                    m_bufferlength(length_of_buffer),
                    m_curpos(0),
                    m_SNR(-5.0),
                    m_frequency(0.0),
                    m_strobe(MIN_STROBE),
                    m_counter(0),
                    m_accumulator(0.0)
{   
    v_RAW = new qreal[m_datalength];
    v_Signal = new qreal[m_datalength];

    v_DataForFFT = new qreal[m_bufferlength];
    v_Spectrum = (fftw_complex*) fftw_malloc( sizeof(fftw_complex) * (m_bufferlength/2+1) );
    v_Amplitude = new qreal[m_bufferlength/2+1];

    for (unsigned int i = 0; i < m_datalength; i++)
    {
        v_RAW[i] = i;
        v_Signal[i] = 0.0;
    }
    plan = fftw_plan_dft_r2c_1d(m_bufferlength, v_DataForFFT, v_Spectrum, FFTW_ESTIMATE);
}

QHarmonicProcessor::~QHarmonicProcessor()
{
    delete[] v_RAW;
    delete[] v_Signal;
    delete[] v_DataForFFT;
    delete[] v_Amplitude;
    fftw_free(v_Spectrum);
    fftw_destroy_plan(plan);
}

void QHarmonicProcessor::readData(const quint16 *v_data, quint16 data_length)
{
    quint16 records = 0;
    for(quint16 i = 0; i < data_length; i++)
    {
        m_accumulator += v_data[i];
        m_counter++;
        if( m_counter == m_strobe )
        {
            v_RAW[loop(m_curpos + records)] = m_accumulator / m_strobe;
            m_accumulator = 0.0;
            m_counter = 0;
            records++;
        }
    }

    if(records > 0)
    {
        emit dataUpdated(v_RAW, m_datalength);
        qreal mean = 0.0;
        for(quint32 j = 0; j < m_datalength; j++)
        {
            mean += v_RAW[j];
        }
        mean /= m_datalength;

        qreal sko = 0.0;
        for(quint32 j = 0; j < m_datalength; j++)
        {
            sko += (v_RAW[j] - mean)*(v_RAW[j] - mean);
        }
        if(sko < 0.00000001) // kind a protection of division by zero
        {
            sko = 0.0000001;
        }
        sko = sqrt(sko/(m_datalength-1));

        for(quint16 i = 0; i < records; i++)
        {
            v_Signal[loop(m_curpos+i)] = (v_RAW[loop(m_curpos+i)] - mean)/sko;
        }
        emit signalUpdated(v_Signal, m_datalength);
        m_curpos = (m_curpos + records) % m_datalength;
    }
}

void QHarmonicProcessor::computeFrequency()
{
    quint32 temp_m_curpos = m_curpos-1; // save current m_curpos
    qreal buffer_duration = m_bufferlength*m_discretizationPeriod*m_strobe; // refresh data duration
    //Data preparation
    quint32 position = 0;
    for (quint32 i = 0; i < m_bufferlength; i++)
    {
        position = loop(temp_m_curpos - (m_bufferlength - 1) + i);
        v_DataForFFT[i] = v_Signal[ position ];
    }
    fftw_execute(plan);

    for(quint16 i = 0; i < m_bufferlength/2 + 1; i++)
    {
        v_Amplitude[i] = (v_Spectrum[i][0]*v_Spectrum[i][0] + v_Spectrum[i][1]*v_Spectrum[i][1])/5000;
    }
    emit spectrumUpdated(v_Amplitude,m_bufferlength/2 + 1);

    //position of max harmonic searching
    quint16 lower_bound = (quint16)(LOWER_HR_LIMIT * buffer_duration / 1000); //You should ensure that ( LOW_HR_LIMIT < discretization frequency / 2 )
    quint16 index_of_maxpower = 0;
    qreal maxpower = 0.0;

    for (int i = ( lower_bound + HALF_INTERVAL ); i < ( (m_bufferlength / 2 + 1) - HALF_INTERVAL ); i++)
    {
        qreal temp_power = v_Amplitude[i];
        if ( maxpower < temp_power )
        {
            maxpower = temp_power;
            index_of_maxpower = i;
        }
    }
    //SNR estimation
    qreal noise_power = 0.0;
    qreal signal_power = 0.0;
    for (quint16 i = lower_bound; i < (m_bufferlength / 2 + 1); i++)
    {
        if (  (i > (index_of_maxpower - HALF_INTERVAL)) && (i < (index_of_maxpower + HALF_INTERVAL)) )
        {
            signal_power += v_Amplitude[i];
        }
        else
        {
            noise_power += v_Amplitude[i];
        }
    }
    m_SNR = 10 * log10( signal_power / noise_power );

    qreal power_multiplyed_by_index = 0.0;
    for (int i = (index_of_maxpower - HALF_INTERVAL + 1); i < (index_of_maxpower + HALF_INTERVAL); i++)
    {
        power_multiplyed_by_index += i * v_Amplitude[i];
    }
    qreal index_of_mass_center = power_multiplyed_by_index / signal_power;
    m_SNR *= 1/(1 + (index_of_maxpower - index_of_mass_center)*(index_of_maxpower - index_of_mass_center));

    if(m_SNR >= SNR_TRESHOLD)
    {
        m_frequency = index_of_mass_center * 60000 / buffer_duration;
        emit frequencyUpdated(m_frequency, m_SNR);
    }
    else
    {
        emit tooNoisy(m_SNR);
    }
}

quint32 QHarmonicProcessor::getDatalength() const
{
    return m_datalength;
}

quint32 QHarmonicProcessor::getBufferlength() const
{
    return m_bufferlength;
}

void QHarmonicProcessor::setStrobe(int value)
{
    if((value > 0) && (value <= MAX_STROBE))
    {
        m_strobe = (quint8)value;
    }
    else
    {
        m_strobe = MIN_STROBE;
    }
}

quint16 QHarmonicProcessor::getStrobe() const
{
    return m_strobe;
}

void QHarmonicProcessor::setDiscretizationPeriod(qreal value) // in ms
{
    m_discretizationPeriod = value;
}
