/*
 * ATMEGA328PB
 *
 * NRF24L01
 * CE   -> PB1, D9
 * CSN  -> PB2, D10
 * MOSI -> PB3, D11
 * SCK  -> PB5, D12
 * MISO -> PB4, D13
 * 
 * MCP23008
 * Address 0x20
 * 
 * LED -> PD5, D5
 * 
 * 
 */
 
#define DEBUG
#define RADIO
#define PIN_LED 5
#define NO_GPIO_PINS 8

// Adafruit MCP23008 library
#include "Adafruit_MCP23008.h"
Adafruit_MCP23008 mcp;

#if defined(RADIO)
#include "RF24.h"
#define CE_PIN 9
#define CSN_PIN 10
RF24 radio(CE_PIN, CSN_PIN);
byte addresses[][6] = { "j1tx","j1rx" };
#endif

#if defined(DEBUG)
#define SERIAL_DEBUG
#include <printf.h>
#endif

// PORTD Joystick
#define JOY_LEFT 2
#define JOY_RIGHT 7
#define JOY_DOWN 4
#define JOY_UP 3

// PORTC Buttons
//#define BUT_1 2
//#define BUT_2 3

// Analog pins
#define ANALOG1 A0
#define ANALOG2 A1
int analogOldVal1, analogOldVal2, analogNewVal1, analogNewVal2;

int timeout = 0;
int threshold = 5;

byte switchVar1 = 72;  //01001000
unsigned int  btnBits = 0;

// joystick buffer
byte joystick[] = { 127, 127, 127, 127, 127, 127, 0, 0 };
byte joystick_old[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int sz = sizeof(joystick);
#define JOY_X 0
#define JOY_Y 1
#define ROT_X 2
#define ROT_Y 3
#define ROT_Z 4
#define SLIDER 5
#define BUTTONS_1 6
#define BUTTONS_2 7

void setup() {
  
  #if defined(DEBUG)
  Serial.begin( 115200 );
  Serial.println("Starting...");
  printf_begin();
  #endif  

  // MCP23008 setup
  mcp.begin();
  for(int i=0;i<NO_GPIO_PINS;i++) {
    mcp.pinMode(i, INPUT);
    mcp.pullUp(i, HIGH);  // turn on a 100K pullup internally
  }
  
  // Joystick pins setup
  // Configure Joystick pins for input
  DDRD &= ~( (1 << JOY_LEFT) | (1 << JOY_RIGHT) | (1 << JOY_UP) | (1 << JOY_DOWN) );
  // Confgure Joystick pins as input_pullup
  PORTD |= ( (1 << JOY_LEFT) | (1 << JOY_RIGHT) | (1 << JOY_UP) | (1 << JOY_DOWN) );

  // Buttons pins setup
  // Configure button pins for input
  //DDRC &= ~( (1 << BUT_1) | (1 << BUT_2) );
  // Confgure Joystick pins as input_pullup
  //PORTC |= ( (1 << BUT_1) | (1 << BUT_2) );

  // setup nrf24
  #if defined(RADIO)
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setAutoAck(1); 
  radio.setChannel(95);
  radio.setRetries(15,15);
  radio.enableDynamicPayloads();
  //radio.setCRCLength(RF24_CRC_16);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);
  #endif
  #if defined(DEBUG)
  #if defined(RADIO)
  radio.printDetails();
  #endif
  #endif
  delay(1000);
}

void loop() {
  
  // status
  int pressed = 0;

  // Read MCP23008 GPIO ports (note 1=open)
  switchVar1 = 255 - mcp.readGPIO();

  // Read GPIO buttons
  btnBits = switchVar1;
  if(btnBits) {
    joystick[BUTTONS_2] = btnBits;
  } else {
    joystick[BUTTONS_2] = 0;
  } // end if
  
  // Read joystick
  /*
  Serial.println(!(PIND & (1 << JOY_LEFT)));
  Serial.println(!(PIND & (1 << JOY_RIGHT)));
  Serial.println(!(PIND & (1 << JOY_UP)));
  Serial.println(!(PIND & (1 << JOY_DOWN)));
  */
  !(PIND&(1 << JOY_LEFT)) ? joystick[JOY_X] = 0 : (!(PIND&(1 << JOY_RIGHT)) ? joystick[JOY_X] = 255 : joystick[JOY_X] = 127);
  !(PIND&(1 << JOY_UP)) ? joystick[JOY_Y] = 0 : (!(PIND&(1 << JOY_DOWN)) ? joystick[JOY_Y] = 255 : joystick[JOY_Y] = 127);

  // Read PORTC buttons
  //Serial.println( (PINC & (1 << BUT_1)) );
  //Serial.println( (PINC & (1 << BUT_2)) );

  // Read analog inputs
  analogNewVal1 = analogRead(ANALOG1)>>2;
  analogNewVal2 = analogRead(ANALOG2)>>2;
  if (analogNewVal1 > analogOldVal1 + 2 || analogNewVal1 < analogOldVal1 - 2) {
    joystick[ROT_X] = analogNewVal1;
    analogOldVal1 = analogNewVal1;
  } // end if
  if (analogNewVal2 > analogOldVal2 + 2 || analogNewVal2 < analogOldVal2 - 2) {
    joystick[ROT_Y] = analogNewVal2;
    analogOldVal2 = analogNewVal2;
  } // end if

  // compare with old values
  for(int i=0; i<sz; i++) {
    if( joystick_old[i] != joystick[i] )
      pressed = 1;
    joystick_old[i] = joystick[i];
  } // end for

  // send data only if change
  if( pressed ) {
    #if defined(RADIO)
    if( !radio.write( &joystick, sz ) ) {
      #if defined(DEBUG)
      Serial.println(F("send failed"));
      #endif
    } else {
      #if defined(DEBUG)
      Serial.println(F("sent"));
      #endif
    } // end if
    #endif*/
    #ifdef DEBUG
    for(int i=0; i<sz; i++) {
      Serial.print(joystick[i]);
      Serial.print(" ");
    } // end for
    Serial.println("");
    #endif
  } // end if

  delay(20);
}
