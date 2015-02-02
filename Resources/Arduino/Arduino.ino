/*------------------------------------------------------------------------------------------------------
Taranov Alex, 2014
A sketch for the Arduino Uno (rev.3)
Microprogram will sampling the analogue input and right away sends digitized values to the serial port.
Average sampling_&_transmission period should be around 128.98 us (only for board default settings).
You can measure sampling_&_transmission period by means of speed_test() function.
------------------------------------------------------------------------------------------------------*/

#define SERIAL_SPEED 115200 // bps, serial transmission speed
#define DELAY_TIME 700

byte signal1 = 0;
byte signal2 = 0;
char symboll = '1';

void setup()
{
	Serial.begin( SERIAL_SPEED );
}

void loop()
{
	if(Serial.available())
	{
		symboll = Serial.read();
	}
	
	if(symboll == '1')
	{	
		signal1 = (byte)(analogRead(A0) >> 2);
                Serial.write(signal1);
                signal2 = (byte)(analogRead(A15) >> 2);
		Serial.write(signal2);
                Serial.write(0xFF);
                delayMicroseconds(DELAY_TIME);		
	}

        if(symboll == '2')
        {
         Serial.println(speed_test()); 
        }
}

float speed_test()
{
  unsigned long timestamp = micros();
  for(int i = 0; i < 256; i++)
  {
    signal1 = (byte)(analogRead(A0) >> 2);
    Serial.write(signal1);
    signal2 = (byte)(analogRead(A15) >> 2);
    Serial.write(signal2);
    Serial.write(0xFF);
    delayMicroseconds(DELAY_TIME);
  }  
  timestamp = micros() - timestamp;
  
  return (timestamp/256.0);    
}


