// PPM Generation with Timer1, Pin D10.
// written by David Wheeler
// 
// The code initializes the Timer1 peripheral, then services its interrupt cyclically.  
// A PPM signal of 50Hz is generated on pin D10.
// Currently, all servo positions are centered (1.5ms servo times)
// To move the servos, modify the program to write values in ppmTime[1] through ppmTime[6].
// Values should range from 2000 (1ms) to 4000 (2ms).

int debugPin = 13;

#define PPM_CHANNELS (6)
#define PPM_PHASES (PPM_CHANNELS+1)
#define PPM_RESYNC_PHASE (0)
#define PPM_PULSE_WIDTH (800)  // 400uS
#define PPM_CENTER (3000)      // 1.5mS
#define PPM_FRAME_LEN (64000)  // 32mS -> ~30Hz

// Values for Timer1 Config registers
#define WGM_15_1A (0x03)
#define WGM_15_1B (0x03 << 3)
#define COM1A_00 (0x00 << 6)
#define COM1B_10 (0x02 << 4)
#define COM1C_00 (0x00 << 2)
#define CS1_DIV8 (0x02)
#define TIMSK1_TOIE (0x01)

int ppmPin = 10;
volatile int tcycles = 0;
int ppmPhase = 0; // which phase of PPM we are in, 0-6.  6 is long resync phase. 
int ppmTime[PPM_PHASES];  // width of each PPM phase (1 = 0.5uS)


void setup()
{
  // pinMode(RXLED, OUTPUT);  // Set RX LED as an output
  pinMode(debugPin, OUTPUT);
  // TX LED is set as an output behind the scenes


  Serial.begin(9600); //This pipes to the serial monitor
  // Serial1.begin(9600); //This is the UART, pipes to sensors attached to board
 
  // Init PPM phases
  int remainder = PPM_FRAME_LEN;
  for (ppmPhase = 1; ppmPhase <= PPM_CHANNELS; ppmPhase++) {
    ppmTime[ppmPhase] = PPM_CENTER;  // 1.5mS
    remainder -= ppmTime[ppmPhase];
  }
  ppmTime[PPM_RESYNC_PHASE] = remainder;
  ppmPhase = 0;
 
  // Disable Interrupts
  cli();
 
  // Register interrupt handler
  // Init for PPM Generation
  pinMode(ppmPin, OUTPUT);
  TCCR1A = WGM_15_1A | COM1A_00 | COM1B_10 | COM1C_00; // Fast PWM with OCR1A defining TOP
  TCCR1B = WGM_15_1B | CS1_DIV8;                       // Prescaler : system clock / 8.
  TCCR1C = 0;                                          // Not used
  OCR1A = ppmTime[ppmPhase];                           // Length of current phase
  OCR1B = PPM_PULSE_WIDTH;                             // Width of pulses
  // OCR1C = 0;                                           // Not used
  TIMSK1 = TIMSK1_TOIE;                                // interrupt on overflow (end of phase)
  
  // Enable interrupts
  sei();
}

ISR(TIMER1_OVF_vect) 
{
    static unsigned remainder = PPM_FRAME_LEN;
    
    // Handle Timer 1 overflow: Start of PPM interval
    // Program total length of this phase in 0.5uS units.
    ppmPhase += 1;
    if (ppmPhase > PPM_CHANNELS) {
        ppmPhase = 0;
        tcycles++;
    }
    
    // Remainder computation
    if (ppmPhase == 0) {
        // Store remainder
        ppmTime[ppmPhase] = remainder;
        remainder = PPM_FRAME_LEN;
    }
    else {
        remainder -= ppmTime[ppmPhase];
    }
    
    // Program next interval time.
    OCR1A = ppmTime[ppmPhase];
}

void loop()
{
  char s[128];
  
  // sprintf(s, "Timer cycles: %d, TCNT1=%04x, TIFR1=%02x, TIMSK1=%02x (TOIE1=%02x)", tcycles, TCNT1, TIFR1, TIMSK1, TOIE1);
  // Serial.println(s);
 // Serial1.println("Hello!");  // Print "Hello!" over hardware UART

 digitalWrite(debugPin, LOW);   // set the LED on
 // TXLED0; //TX LED is not tied to a normally controlled pin
 delay(500);              // wait for a second
 digitalWrite(debugPin, HIGH);    // set the LED off
 // TXLED1;
 delay(500);              // wait for a second
}

