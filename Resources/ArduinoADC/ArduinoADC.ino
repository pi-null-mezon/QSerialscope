/*------------------------------------------------------------------------------------------------------
 *  ARDUINO SKETCH
 *  Microprogram will sampling the analogue input and right away sends digitized values to the serial port.
 *  Some settings facilities are available on the following serial terminal codes: 
 *  'm' - turns on general regime of measurements 
 *  't' - turns on sampling period measurements
 *  's' - turns on sampling period change procedure
 *  'p' - turns on pin change procedure
 *  You should know, that minimum sampling period on UNO R3 in 2015 was about 125-130 us, thus by means of 's'
 *  You can only add some time to this limit. Taranov Alex, 2015
 ------------------------------------------------------------------------------------------------------*/

#define SERIAL_SPEED 115200 // bps, serial transmission speed
#define DEFAULT_PIN 0 // pin_id for the sampling
#define DEFAULT_PERIOD 2000 // us

byte m_pin = DEFAULT_PIN;
int  m_measurement;
int  m_symboll = 'm';
long m_period = DEFAULT_PERIOD; 

void setup()
{
  Serial.begin( SERIAL_SPEED );
}

void loop()
{
  if(Serial.available()) {
    m_symboll = Serial.read();
  }
  switch(m_symboll)
  {
    case 'm':
      makeMeasurement();
      break;
     case 't':
       Serial.println(runSpeedTest());
       m_symboll = 'm';	
       break;
     case 's':
       setSamplingPeriod();
       break;
     case 'p':
       setPin();
       break;
     default:
       m_symboll = 'm';
       break;	
  }
}

void makeMeasurement() // works in BigEndian Regime
{
  m_measurement = analogRead(m_pin);                   
  if((byte)m_measurement == 0xFF) {
    Serial.write(0xFE);
  } else {
    Serial.write((byte)m_measurement);
  }
  Serial.write((byte)(m_measurement >> 8) & 0x03);
  Serial.write(0xFF); // A markup symbol: ...0xFF 0xXX 0xXX 0xFF 0xXX 0xXX 0xFF 0xXX... 
  delayMicroseconds(m_period);  
}

float runSpeedTest()
{
  unsigned long timestamp = micros();  
  for(int i = 0; i < 256; i++) {
    makeMeasurement();
  }
  return (micros() - timestamp)/256.0;    
}

void setSamplingPeriod()
{
  Serial.print("Send appropriate sampling period in us: ");
  bool readyFlag = false;
  while(!readyFlag)
  {
    if(Serial.available())
    {
      m_period = Serial.parseInt();
      readyFlag = true;
    }  
  }
}

void setPin()
{
  Serial.print("Send appropriate pin number: ");
  bool readyFlag = false;
  while(!readyFlag)
  {
    if(Serial.available())
    {
      m_pin = Serial.parseInt();
      if((m_pin < 8) && (m_pin >=0))
        readyFlag = true;
      else
        Serial.print("Out of appropriate range, try another value: ");
    }  
  }    
}




