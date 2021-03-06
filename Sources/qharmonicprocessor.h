/*------------------------------------------------------------------------------------------------
 * Taranov Alex, 2015
 *
 * For propper work you should use only magnitudes of 2 for m_datalength and m_bufferlength
 * ---------------------------------------------------------------------------------------------*/

#ifndef QHARMONICPROCESSOR_H
#define QHARMONICPROCESSOR_H

#include <QObject>
#include "fftw3.h"

//---------------------------------------------------------------------------
#define BOTTOM_HR_LIMIT 0.5  // in s^-1, it is 30 bpm
#define TOP_HR_LIMIT 3.0    // in s^-1, it is 180 bpm
#define SNR_TRESHOLD 0.5
#define HALF_INTERVAL 3     // defines the number of averaging indexes when frequency is evaluated, this value should be >= 1
#define MIN_STROBE 1        // defines the min value of the m_strobe (how much raw counts represents one count in output data, in another words it is frequency divider)
#define MAX_STROBE 255      // because m_strobe is quint8 type
//---------------------------------------------------------------------------

class QHarmonicProcessor : public QObject
{
    Q_OBJECT
public:
    explicit QHarmonicProcessor(QObject *parent = 0, quint16 length_of_data = 512, quint16 length_of_buffer = 512);
    ~QHarmonicProcessor();
    quint32 getDatalength() const;
    quint32 getBufferlength() const;

signals:
    void signalUpdated(const qreal *pointer_to_data, quint16 length_of_data);
    void spectrumUpdated(const qreal *pointer_to_data, quint16 length_of_data);
    void frequencyUpdated(qreal frequency_value, qreal snr);
    void dataUpdated(const qreal *pointer_to_data, quint16 length_of_data);
    void tooNoisy(qreal snr);
    void countUpdated(quint16 value);

public slots:
    void  readData(const quint16 *v_data, quint16 data_length);
    void computeFrequency();
    void setStrobe(int value);
    void setDiscretizationPeriod(qreal value);
    quint16 getStrobe() const;
    qreal getDiscretizationPeriod() const;

private:
    qreal *v_RAW;           //a pointer to spattialy averaged data (you should use it to write data to an instance of a class)
    qreal *v_DataForFFT;     //a pointer to data prepared for FFT, explicit float because fftwf (single precision) is used
    qreal m_frequency;      //a variable for storing a last evaluated frequency of the 'strongest' harmonic
    quint16 m_curpos;       //a current position I meant
    quint16 m_datalength;   //a length of data array
    quint16 m_bufferlength; //a lenght of sub data array for FFT (m_bufferlength should be <= m_datalength)
    qreal *v_Signal;        //a pointer to centered and normalized data (typedefinition from fftw3.h, a single precision complex float number type)
    qreal m_SNR;            // a variable for signal-to-noise ratio estimation storing
    fftw_complex *v_Spectrum;  // a pointer to an array for FFT-spectrum
    qreal *v_Amplitude;     // a pointer to Amplitude spectrum
    quint8 m_strobe;       // strobe factor for data acquire
    quint8 m_counter;      // a counter of strobe length
    qreal m_accumulator;    // accumulator for values in strobe
    qreal m_discretizationPeriod; // in ms
    fftw_plan plan;

    quint16 loop(qint32) const; //a function that return a loop-index (not '' because 'inline' )
};
//---------------------------------------------------------------------------
inline quint16 QHarmonicProcessor::loop(qint32 difference) const
{
    return ((m_datalength + (difference % m_datalength)) % m_datalength); // have finded it on wikipedia ), it always returns positive result
}
//---------------------------------------------------------------------------


#endif // QHARMONICPROCESSOR_H
