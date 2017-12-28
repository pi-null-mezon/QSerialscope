#QVASS
Qt-library-based visual advanced serialport scope

Application adc2pc was designed as a common interface for data transfering from external AnalogToDigitalConverter (ADC) to PersonalComputer (PC) with basic capabilities to harmonic analysis of the captured signal. Before use the app, you should provide a serial connection driver for the external device (ADC's board).

Expected transmission data format:

- if you use less than 9 bits per one value it is following ...0xDD0xDD0xDD... (0xDD - data digits);
- if you send from 9 to 16 bits per one value it is following ...0xFF0xDD0xDD0xFF0xDD0xDD0xFF... (0xFF - key symbol). 
