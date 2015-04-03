/*------------------------------------------------------------------------------------------------------------
Microprogram for Arduino Uno (rev.3) that gets you a simple interface to AT commands interchange between
Arduino board and Bluetooth module HC-06. Just compile (with appropriate values), upload, open serial port
monitor and wait. Do not forget to plug in your HC-06 (you can do int on the fly). Take in mind that after
the speed change, the HC-06 will stop to respond on commands, that were sent with another speed. Therefore,
second time you want to change settings, you have to recompile project with appropriate speed of COM port.  
Taranov Alex, 2014
-------------------------------------------------------------------------------------------------------------*/

#define BUFFER_LENGTH 64 // the size of the serial receive buffer for ATmega328
#define SERIAL_SPEED 115200 // bps
#define DELAY 1000 // in ms

char buffer[BUFFER_LENGTH];
const char *at = "AT"; 
const char *at_version = "AT+VERSION";

//VALUES FOR MODIFICATION----------------------------------------------------------------------------------------------
const char *at_baud = "AT+BAUD8"; // Change the end-number by your self: AT+BAUD#, if # == 4 then 9600, if # ==8 115200
const char *at_pin = "AT+PIN1234"; // Change the end-number by your self: AT+PIN#
const char *at_name = "AT+NAMEHC-06"; // Change the end-string by your self: AT+NAME#
//---------------------------------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(SERIAL_SPEED);
}

void loop()
{ 
  Serial.println("Start scanning: ");
  
  ask_command(at, 2); // do not change the number inside call if you do not understand what does it mean

  ask_command(at_version, 10); // do not change the number inside call if you do not understand what does it mean

  ask_command(at_baud, 8); // do not change the number inside call if you do not understand what does it mean

  ask_command(at_name, 12); // do not change the number inside call if you do not understand what does it mean

  ask_command(at_pin, 10); // do not change the number inside call if you do not understand what does it mean 
}

void ask_command(const char *command, byte length)
{
  Serial.print("We ask: \t");
  delay(DELAY);
  for(byte i = 0; i < length; i++) // maybe this cycle can be overwritten as simple Serial.write(command), try it...
  {
    Serial.print(command[i]); 
  }  
  delay(DELAY);
  Serial.print('\n');
  if(Serial.available())
  {   
    byte symbols = Serial.readBytes(buffer,BUFFER_LENGTH);
    Serial.print("Device says:\t");
    for(byte i = 0; i < symbols; i++)
    {
      Serial.print(buffer[i]);
    }
    Serial.print('\n');
  }
  else
  {
    Serial.println("No answers...");
  }  
}


