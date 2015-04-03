/*------------------------------------------------------------------------------------------------------
 *  ARDUINO SKETCH
 *  Microprogram will sampling the analogue input and right away sends digitized values to the serial port.
 *  Some settings facilities are available on the following serial terminal codes: 
 *  'm' - start measurements 
 *  't' - measure sampling period & stop
 *  'd' - set delay & stop
 *  'p' - set pin & stop
 *  Remember that the board is reset on serial connection establishing (see docs...),
 *  so if you do not fix this particular feature, all settings will desappear after new connection!  
 *  Taranov Alex, 2015
 ------------------------------------------------------------------------------------------------------*/

#define SERIAL_SPEED 115200 // bps, serial transmission speed
#define DEFAULT_PIN 0 // pin_id for the sampling
#define DEFAULT_DELAY 100 // in us

byte m_pin = DEFAULT_PIN;
int  m_measurement;
int  m_symbol = 'm';
long m_delay = DEFAULT_DELAY; 

void setup()
{
  Serial.begin( SERIAL_SPEED );
}

void loop()
{
  if(Serial.available()) {
    m_symbol = Serial.read();
  }
  switch(m_symbol) {
     case 'm':
       makeMeasurement();
       break;
     case 't':
       runSpeedTest();
       m_symbol = 0;
       break;
     case 'd':
       setDelay();
       m_symbol = 0;
       break;
     case 'p':
       setPin();
       m_symbol = 0;
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
  delayMicroseconds(m_delay);  
}

void runSpeedTest()
{
  unsigned long timestamp = micros();  
  for(int i = 0; i < 256; i++) {
    makeMeasurement();
  }
  Serial.println((micros() - timestamp)/256.0);   
}

void setDelay()
{
  Serial.print("Set desirable delay in us: ");
  bool readyFlag = false;
  while(!readyFlag) {
    if(Serial.available()) {
      m_delay = Serial.parseInt();
      Serial.print (m_delay);
      Serial.println(" us");
      readyFlag = true;
    }  
  }
}

void setPin()
{
  Serial.print("Set desirable pin number: ");
  bool readyFlag = false;
  while(!readyFlag)  {
    if(Serial.available()) {
      m_pin = Serial.parseInt();
      if((m_pin >=0) && (m_pin < 8)) {
        Serial.print(m_pin);
        Serial.println(" pin selected");
        readyFlag = true;
      } else {
        Serial.print("out of range! try another value: ");
      }
    }  
  }    
}




