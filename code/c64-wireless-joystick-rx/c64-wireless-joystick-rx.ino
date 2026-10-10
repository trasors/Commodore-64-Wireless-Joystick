/*
// ATMEL ATTINY1634
//
//                   +-\/-+
// TX   (D  0) PB0  1|    |20  PB1 (D  16)
// RX   (D  1) PA7  2|    |19  PB2 (D  15)
//    * (D  2) PA6  3|    |18  PB3 (D  14) *
//    * (D  3) PA5  4|    |17  PC0 (D  13) *
//      (D  4) PA4  5|    |16  PC1 (D  12)
//      (D  5) PA3  6|    |15  PC2 (D  11)
//      (D  6) PA2  7|    |14  PC3/RESET (D 17)
//      (D  7) PA1  8|    |13  PC4 (D  10)
//      (D  8) PA0  9|    |12  PC5 (D  9)
//             GND 10|    |11  VCC
//                   +----+
//
// Reminder: Pins 6 - 1 & 20 - 15 are ADC pins
// * indicates PWM port
// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
 *
 * NRF24L01 / ATTINY1634
 * PIN 1  GND  -> GND
 * PIN 2  VCC  -> 3V3
 * PIN 3  CE   -> 4  / PA5 / D3
 * PIN 4  CSN  -> 3  / PA6 / D2
 * PIN 5  SCK  -> 16 / PC1
 * PIN 6  MOSI -> 19 / PB2
 * PIN 7  MISO -> 20 / PB1
 * PIN 8  IRQ  -> N/C
 * 
 * Notes
 * http://atariage.com/forums/topic/232752-5v-to-atari-joystick-ports/
 * https://www.c64-wiki.com/wiki/64JPX
 * https://github.com/svofski/mouse1351/blob/master/c1351.c
 * http://atariage.com/forums/blog/572/entry-11529-nunchuk-arduino-atari-joystick/ (photocoupler)
 * 
 */

// #define DEBUG // must uncomment #define SERIAL_DEBUG, line 19 RF24_config.h
#define RADIO

#if defined( DEBUG )
  #define DEBUG_STREAM  Serial
#endif

// nrf setup
#if defined( RADIO )
  #define CE_PIN 3  // PA5
  #define CSN_PIN 2 // PA6
  #include "RF24.h"
  RF24 radio(CE_PIN, CSN_PIN);
  byte addresses[][6] = { "j1tx","j1rx" };
#endif

/*
 * Digispark Joystick
 * 0 - X Axis (0=L, 255=R)
 * 1 - Y Axis (0=U, 255=D)
 * 2 - Analog 1
 * 3 - Analog 2
 * 4 - 6 Not used
 * 7 - Buttons (1B, 2P, 4B, 8P, 16Y, 32Y, 64B, 128P) B=Blue, P=Pink, Y=Yellow
 * 
 * C64 Joystick Port
 * Pin  Direction   Port 1  Port 2
 * 1    Up          PA3     PC4
 * 2    Down        PA2     PC2
 * 3    Left        PA1     PC0
 * 4    Right       PA0     PB3
 * 5    Pot A
 * 6    Button      PA4     PC5
 * 7    +5V 100Ma
 * 8    GND
 * 9    Pot B
 *
 * DB9 header
 * Pin  Direction
 * 1    GND
 * 2    +5V
 * 3    Button
 * 4    Up
 * 5    Down
 * 6    Left
 * 7    Right
 * 
 */

// PORT A
#define JOY1_L 1
#define JOY1_R 0
#define JOY1_U 3
#define JOY1_D 2
#define JOY1_B 4

// PORT B
#define JOY2_R 3

// PORT C
#define JOY2_L 0
#define JOY2_U 4
#define JOY2_D 2
#define JOY2_B 5

#define LED_PIN 1

byte receive_payload[32];

bool joy_toggle = false; // default to joy2

void reset_joy_1() {
  // we want the pins to be reset
  DDRA &= ~( (1 << JOY1_L) | (1 << JOY1_R) | (1 << JOY1_U) | (1 << JOY1_D) | (1 << JOY1_B) ); // input
}

void reset_joy_2() {
  // we want the pins to be reset
  DDRB &= ~  (1 << JOY2_R);
  DDRC &= ~( (1 << JOY2_L) | (1 << JOY2_U) | (1 << JOY2_D) | (1 << JOY2_B) ); // input
}

void setup() {

/*
 * DDR PORT
 * 0    0  - input, high impedance
 * 0    1  - input, pulled up
 * 1    0  - output, driven low
 * 1    1  - output, driven high
 * 
 * INPUT:  DDRx &= ~( pin | pin ) 
 * OUTPUT: DDRx |= pin | pin
 * LOW:  PORTx &= ~
 * HIGH: PORTx |=
 * 
 */
  DDRA &= ~( (1 << JOY1_L) | (1 << JOY1_R) | (1 << JOY1_U) | (1 << JOY1_D) | (1 << JOY1_B) ); // input
  PORTA &= ~( (1 << JOY1_L) | (1 << JOY1_R) | (1 << JOY1_U) | (1 << JOY1_D) | (1 << JOY1_B) );// high impedance

  DDRC &= ~( (1 << JOY2_L) | (1 << JOY2_U) | (1 << JOY2_D) | (1 << JOY2_B) ); // input
  PORTC &= ~( (1 << JOY2_L) | (1 << JOY2_U) | (1 << JOY2_D) | (1 << JOY2_B) );// high impedance
  DDRB &= ~(1 << JOY2_R);
  PORTB &= ~(1 << JOY2_R);

  pinMode(LED_PIN, OUTPUT);
  #if defined(DEBUG)
  pinMode(LED_PIN, OUTPUT);
  DEBUG_STREAM.begin(9600);
  #endif
  
  // setup nrf24
  #if defined( RADIO )
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setAutoAck(1); 
  radio.setChannel(95);
  radio.setRetries(15,15);
  radio.enableDynamicPayloads();
  //radio.setCRCLength(RF24_CRC_16);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1,addresses[1]);
  radio.startListening();
  #endif
  
  #if defined( DEBUG )
  DEBUG_STREAM.print(F("C64JX Debug"));
  //radio.printDetails();
  #endif
while(0);
}

void loop() {
  
  uint8_t len = 0;
  #if defined( RADIO )
  if ( radio.available() ) {
    while (radio.available()) {
      len = radio.getDynamicPayloadSize();
      if( !len ) return;
      radio.read( &receive_payload, len );
      #if defined( DEBUG )
      for(int i=0;i<len;i++) {
        DEBUG_STREAM.print(receive_payload[i]);
      } // end for
      digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(25);
      digitalWrite(LED_PIN, LOW);
      #endif

      if( receive_payload[7] == 16 ) {
        digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(250);
        digitalWrite(LED_PIN, LOW);
        joy_toggle = !joy_toggle;
        reset_joy_1();
        reset_joy_2();
      }

      if( joy_toggle ) {
      (receive_payload[0] < 127) ? DDRA |= (1 << JOY1_L) : DDRA &= ~( (1 << JOY1_L) ); // LEFT?
      (receive_payload[0] > 127) ? DDRA |= (1 << JOY1_R) : DDRA &= ~( (1 << JOY1_R) ); // RIGHT?
      (receive_payload[1] < 127) ? DDRA |= (1 << JOY1_U) : DDRA &= ~( (1 << JOY1_U) ); // UP?
      (receive_payload[1] > 127) ? DDRA |= (1 << JOY1_D) : DDRA &= ~( (1 << JOY1_D) ); // DOWN?
      (receive_payload[7] > 0  ) ? DDRA |= (1 << JOY1_B) : DDRA &= ~( (1 << JOY1_B) ); // BUTTON?
      } else {
      (receive_payload[0] < 127) ? DDRC |= (1 << JOY2_L) : DDRC &= ~( (1 << JOY2_L) ); // LEFT?
      (receive_payload[0] > 127) ? DDRB |= (1 << JOY2_R) : DDRB &= ~( (1 << JOY2_R) ); // RIGHT?
      (receive_payload[1] < 127) ? DDRC |= (1 << JOY2_U) : DDRC &= ~( (1 << JOY2_U) ); // UP?
      (receive_payload[1] > 127) ? DDRC |= (1 << JOY2_D) : DDRC &= ~( (1 << JOY2_D) ); // DOWN?
      (receive_payload[7] > 0  ) ? DDRC |= (1 << JOY2_B) : DDRC &= ~( (1 << JOY2_B) ); // BUTTON?
      } // end if
 
    } // end while
  } // end if
  #endif
//delay(1000);
//DEBUG_STREAM.println(".");

}

