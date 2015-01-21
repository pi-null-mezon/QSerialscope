#include "qharmonicprocessor.h"

#ifdef MINGW_COMPILER
#include <cmath>
#endif

QHarmonicProcessor::QHarmonicProcessor(QObject *parent, quint32 length_of_data, quint32 length_of_buffer) :
                    QObject(parent),
                    m_datalength(length_of_data),
                    m_bufferlength(length_of_buffer),
                    m_curpos(0),
                    m_SNR(-5.0),
                    m_frequency(0.0),
                    m_strobe(64),
                    m_counter(0),
                    m_accumulator(0.0),
{   
    v_RAW = new qreal[m_datalength];
    v_Signal = new qreal[m_datalength];

    v_DataForFFT = new qreal[m_bufferlength];
    v_Spectrum = (fftw_complex*) fftw_malloc( sizeof(fftw_complex) * (m_bufferlength/2+1) );
    v_Amplitude = new qreal[m_bufferlength/2+1];

    for (unsigned int i = 0; i < m_datalength; i++)
    {
        v_RAW[i] = 10.0;
        v_Signal[i] = 0.0;
    }
}

QHarmonicProcessor::~QHarmonicProcessor()
{
    delete[] v_RAW;
    delete[] v_Signal;
    delete[] v_DataForFFT;
    delete[] v_Amplitude;
    fftw_free(v_Spectrum);
}

void QHarmonicProcessor::readByteBuffer(const QByteArray &buffer)
{
    quint16 records = 0;
    //Enroll new data with strobe, may work slow on low m_strobe values
    for(int i = 0; i < buffer.length(); i++) // from the beginning to the end
    {
        m_accumulator += (quint8)buffer.at(i);
        if( (m_counter++) == m_strobe )
        {
            v_RAW[loop(m_curpos + records)] = m_accumulator / m_strobe;
            //refresh accumulators
            m_accumulator = 0.0;
            m_counter = 0;
            records++; // increment counter of records for v_RAW
        }
    }

    if(records > 0)
    {
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

qreal QHarmonicProcessor::computeFrequency()
{
    quint32 temp_m_curpos = m_curpos-1; // save current m_curpos
    qreal buffer_duration = 0.0; // refresh data duration

    fftw_plan p = fftw_plan_dft_r2c_1d(m_bufferlength, v_RAWForFFT, v_Spectrum, FFTW_ESTIMATE);
    //Data preparation
    quint32 position = 0;
    for (quint32 i = 0; i < m_bufferlength; i++)
    {
        position = loop(temp_m_curpos - (m_bufferlength - 1) + i);
        v_DataForFFT[i] = v_Signal[ position ];
    }
    fftw_execute(p);

    for(quint32 i = 0; i < m_bufferlength/2 + 1; i++)
    {
        v_Amplitude[i] = (v_Spectrum[i][0]*v_Spectrum[i][0] + v_Spectrum[i][1]*v_Spectrum[i][1])/5000;
    }
    emit spectrumUpdated(v_Amplitude,m_bufferlength/2 + 1);

    //position of max harmonic searching
    quint32 lower_bound = (quint32)(LOWER_HR_LIMIT * buffer_duration / 1000); //You should ensure that ( LOW_HR_LIMIT < discretization frequency / 2 )
    quint32 index_of_maxpower = 0;
    qreal maxpower = 0.0;

    for (unsigned int i = ( lower_bound + HALF_INTERVAL ); i < ( (m_bufferlength / 2 + 1) - HALF_INTERVAL ); i++)
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
    for (quint32 i = lower_bound; i < (m_bufferlength / 2 + 1); i++)
    {
        if ( ( (i > (index_of_maxpower - HALF_INTERVAL )) && (i < (index_of_maxpower + HALF_INTERVAL) ) ) || ( (i > (2 * index_of_maxpower - HALF_INTERVAL )) && (i < (2 * index_of_maxpower + HALF_INTERVAL) ) ) )
        {
            signal_power += v_Amplitude[i];
        }
        else
        {
            noise_power += v_Amplitude[i];
        }
    }
    if ((noise_power > 0.0) && (signal_power > 0.0))
    {
        m_SNR = 10 * log10( signal_power / noise_power );
        if ( m_SNR > SNR_TRESHOLD )
        {
            qreal power_multiplyed_by_index = 0.0;
            qreal power_of_first_harmonic = 0.0;
            for (unsigned int i = (index_of_maxpower - HALF_INTERVAL + 1); i < (index_of_maxpower + HALF_INTERVAL); i++)
            {
                power_of_first_harmonic += v_Amplitude[i];
                power_multiplyed_by_index += i * v_Amplitude[i];
            }
            m_frequency = (power_multiplyed_by_index / power_of_first_harmonic) * 60000 / buffer_duration;
         }
     }
     fftw_destroy_plan(p);
     emit frequencyUpdated(m_frequency, m_SNR);
     return m_frequency;
}

quint32 QHarmonicProcessor::getDatalength() const
{
    return m_datalength;
}

quint32 QHarmonicProcessor::getBufferlength() const
{
    return m_bufferlength;
}

quint16 QHarmonicProcessor::setStrobe(int value)
{
    if(value <= 0)
    {
        m_strobe = MIN_STROBE;
        return -1; // incoming value were incorrect code
    }
    // implicit else branch
    m_strobe = value;
    return 0; // Ok code
}


quint16 QHarmonicProcessor::getStrobe() const
{
    return m_strobe;
}
