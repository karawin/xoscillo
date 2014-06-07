// For arduino Due
// logic and analogic scope
// Front-end Xoscillo.exe
// Freely updated from https://code.google.com/p/xoscillo/source/clones 
// by Jp Cocatrix jp@karawin.fr
// duetimer is a library from https://github.com/ivanseidel/DueTimer
// Use the usb native

#include <DueTimer.h>
//
//
// pins assignment
//
#define BUILTINLED 13
#define PWM_GENERATOR 11
#define ledPin 13    // LED connected to digital pin 13
#define CHAN0 33
#define CHAN1 34
#define CHAN2 35
#define CHAN3 36
#define CHAN4 37
#define CHAN5 38
#define CHAN6 39
#define CHAN7 40
volatile RoReg* pio = portInputRegister(PIOC);
#define CHANPIN (*pio)>>1
//
// globals
//
#define MAXMEM  80000
#define PANALOG 8
#define PDIGITAL 2
//
// Commands
//
#define CMD_IDLE 0
#define CMD_RESET 175
#define CMD_PING '?'
#define CMD_TEST 'a'
#define CMD_READ_ADC_TRACE 170
#define CMD_READ_BIN_TRACE 171

unsigned char triggerVoltage = 0;
uint32_t lastADC = 0;
unsigned char triggered = 0;
unsigned int DataRemaining = MAXMEM;
unsigned int DataMain;
unsigned char channel=0;
unsigned char numChannels=1;
unsigned char channels[4];
volatile bool flagTimer1 = false;
volatile bool flagTimer2 = false;
unsigned int  period;
unsigned int  cindex;
unsigned char buffer[MAXMEM+10];  // un peu de marge
unsigned char command = CMD_IDLE;

void finterrupt1()  // Clignote pour faire joli
{
	flagTimer1 = !flagTimer1;
        digitalWrite(ledPin, flagTimer1?LOW:HIGH);
}   
void finterrupt2()  // base de temps
{
	flagTimer2 = true;
}  
  
void setup() 
{
  analogReadResolution(8);  //sur 8 bits seulement
  pinMode( PWM_GENERATOR, OUTPUT );
  analogWrite(PWM_GENERATOR, 128);

  pinMode( ledPin, OUTPUT );
  digitalWrite(4, HIGH);    
  digitalWrite(5, LOW);    
  analogWrite(9, 64);
  analogWrite(10, 128);
  analogWrite(8, 192);  //PWM pour test
  Timer1.attachInterrupt(finterrupt1);
  Timer2.attachInterrupt(finterrupt2);
  Timer1.start(1000000);
  Timer2.setPeriod(PANALOG) ; //µs samplerate 125000
  period = PANALOG;
  Timer2.start();
  SerialUSB.begin(330400); //ignored bauds
  
  for(int i=CHAN0;i<=CHAN7;i++)
  {
    pinMode(i, INPUT);      // sets the digital pins  as input
    digitalWrite(i, LOW);    
  }
}
//**************************************************************************************/
void ProcessSerialUSBCommand( byte in )
{
  if ( in == CMD_PING )
  {
    SerialUSB.write( 79 ) ;
    SerialUSB.write( 67 ) ;
    SerialUSB.write( triggerVoltage ) ;
    SerialUSB.write( DataRemaining>>8 ) ;
    SerialUSB.write( DataRemaining&0xff ) ;
    for (int i=0;i<2;i++)
    {
      SerialUSB.write( triggerVoltage ) ;
    }
  }
  else if ((in == CMD_TEST)) 
  {
	 SerialUSB.write( "OK 1 2 3 4 5 6 7 8 9 " ) ;  
  }
  else if (( in == CMD_RESET ) )
  {
    command = CMD_IDLE;
    SerialUSB.write( "OK" ) ;
  } 
  else if ( in == CMD_READ_ADC_TRACE )
  {
    while( SerialUSB.available() < 9);
    triggerVoltage = SerialUSB.read();
    DataMain = SerialUSB.read()<<24;
    DataMain |= SerialUSB.read()<<16;
    DataMain |= SerialUSB.read()<<8;
    DataMain |= SerialUSB.read();
    numChannels = SerialUSB.read();
    for (int i=0;i<4;i++)
    {
      channels[i] = SerialUSB.read() + A0;
    }
    DataMain = DataMain * numChannels;
    if (DataMain > MAXMEM)
    {
       DataRemaining = MAXMEM;
       DataMain -= MAXMEM;
    }
    else 
    {
      DataRemaining = DataMain;
      DataMain = 0;
    }  
    
    cindex = 0;
    analogWrite(PWM_GENERATOR, SerialUSB.read());
    
    SerialUSB.write( 85 );
    if (period != PANALOG) { Timer2.setPeriod(PANALOG) ; Timer2.start();}
    period = PANALOG;
    triggered = 0;     
    channel = 0;

        //get a fresher value for lastADC
    lastADC = analogRead(channels[channel]);

    command = CMD_READ_ADC_TRACE;
    flagTimer2 = false;
  }
  else if ( in == CMD_READ_BIN_TRACE )
  {
    while( SerialUSB.available() < 3);
    triggerVoltage = SerialUSB.read();
    DataMain = SerialUSB.read()<<24;
    DataMain |= SerialUSB.read()<<16;
    DataMain |= SerialUSB.read()<<8;
    DataMain |= SerialUSB.read();
    if (DataMain > MAXMEM)
    {
       DataRemaining = MAXMEM;
       DataMain -= MAXMEM;
    }
    else 
    {
      DataRemaining = DataMain;
      DataMain = 0;
    }  
    if (period != PDIGITAL) { Timer2.setPeriod(PDIGITAL) ; Timer2.start();}
    period = PDIGITAL;    
    cindex = 0;
    triggered = 0;  
    lastADC = CHANPIN&1;   
    command = CMD_READ_BIN_TRACE;
    flagTimer2 = false;
    SerialUSB.write( 85 );
  }
}

//**************************************************************************************/
inline void computeBuffer()
{
    cindex++;
    DataRemaining--;
    if ( DataRemaining == 0 )
    {
	SerialUSB.write(buffer,cindex);
        cindex = 0;
        if (DataMain == 0) 
        {
          command = CMD_IDLE;
          return;
        }  
        if (DataMain > MAXMEM)
        {
           DataRemaining = MAXMEM;
           DataMain -= MAXMEM;
        } else
        {
          DataRemaining = DataMain;
          DataMain = 0 ;
        }
    }  
}  
//**************************************************************************************/
static inline uint32_t mapResolution(uint32_t value, uint32_t from, uint32_t to) {
	if (from == to)
		return value;
	if (from > to)
		return value >> (from-to);
	else
		return value << (to-from);
}
//**************************************************************************************/
inline uint32_t myanalogRead(uint32_t ulPin)
{
  uint32_t ulValue = 0;
  uint32_t ulChannel;
  static uint32_t latestSelectedChannel = -1;
  
  ulChannel = g_APinDescription[ulPin].ulADCChannelNumber ;
			// Enable the corresponding channel
			if (ulChannel != latestSelectedChannel) {
				adc_enable_channel( ADC, (adc_channel_num_t)ulChannel );
				if ( latestSelectedChannel != -1 )
					adc_disable_channel( ADC, (adc_channel_num_t)latestSelectedChannel );
				latestSelectedChannel = ulChannel;
			}

			// Start the ADC
			adc_start( ADC );

			// Wait for end of conversion
			while ((adc_get_status(ADC) & ADC_ISR_DRDY) != ADC_ISR_DRDY)
				;

			// Read the value
			ulValue = adc_get_latest_value(ADC); 
                        ulValue = mapResolution(ulValue, ADC_RESOLUTION, 8);
  return ulValue;
}  
//**************************************************************************************/
void loop() 
{
  if (SerialUSB.available() > 0) 
  {
    ProcessSerialUSBCommand( SerialUSB.read() );
  }
  
  if ( command == CMD_READ_ADC_TRACE )
  {
      while (!flagTimer2) ; 
      flagTimer2 = false;
      unsigned char v = myanalogRead(channels[channel]);         
      if ( triggered == 0  )
    {
      if ( ((v >= triggerVoltage) && ( lastADC < triggerVoltage )) || (triggerVoltage == 0) )
      {
        triggered = 1;
      }
      else
      {
        lastADC = v;   
        return;
      }
    }
      
    channel++; 
    channel = channel% numChannels; 

    buffer[cindex] = v;
    computeBuffer();
  }
  else if ( command == CMD_READ_BIN_TRACE )
  {
    while (!flagTimer2) ; flagTimer2 = false;
        unsigned char v = CHANPIN;
    if ( triggered == 0  )
    {
      if ( (v&1 ==1) && (lastADC == 0 )  )
      {
        triggered = 1;
      }
      else
      {
        lastADC = v&1;   
        return;
      }
    } 
    
    buffer[cindex] = v;
    computeBuffer();
  }
}


