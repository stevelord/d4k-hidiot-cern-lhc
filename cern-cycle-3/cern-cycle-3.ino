// Large Hadron Collider Simulator
//
// Press S1 to drop in a clockwise proton (blue)
// Press S2 to drop in an anticlockwise proton (red)
// Press both buttons to trigger the magnet

#include <Adafruit_NeoPixel.h>
#include <stdlib.h> //rand
#include <avr/eeprom.h>

#define S1   PB0  // S1 Tact switch
#define S2   PB2  // S2 Tact switch
#define PIXEL_PIN PB1  // PB1 is connected to Neopixels

#define PIXELS 16

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

bool oS1 = HIGH;
bool oS2 = HIGH;
bool magnets = LOW;
//uint32_t layer0[PIXELS]; // Our combination of all layers.
uint32_t layer1[PIXELS]; // 32-bit colour on each spot, anti-clockwise
uint32_t layer2[PIXELS]; // 32-bit colour on each spot, clockwise
uint32_t layer3[PIXELS]; // 32-bit colour on each spot, magnets
uint32_t wait = 360;

void setup() {
  pinMode(S1, INPUT_PULLUP);
  pinMode(S2, INPUT_PULLUP);
  initRand();
  for (uint8_t i = 0; i < PIXELS - 1; i++){
    layer1[i] = 0;
    layer2[i] = 0;
    layer3[i] = 0;
  }
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  // Get current button state.
  bool nS1 = digitalRead(S1);
  bool nS2 = digitalRead(S2);
  uint32_t colour = 0;

  // Check if state changed from high to low (S1 press).
  if ((nS1 == LOW && oS1 == HIGH)||(nS2 == LOW && oS2 == HIGH)) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    nS1 = digitalRead(S1);
    nS2 = digitalRead(S2);
    colour = getColour();
    //layer1[0] = 0;
    if (nS1 == LOW) {
      layer2[0] = 0x00001FUL;
    }
    if (nS2 == LOW) {
      layer1[0] = 0x1F0000UL;
    }

    if (wait > 30) wait = wait - 10;
  }

  // Prime magnets
  if (nS1 == LOW && nS2 == LOW){
    layer3[1] = 0x001F00UL;
    layer3[2] = 0x001F00UL;
    magnets = HIGH;
  } else {
    layer3[1] = 0x000000UL;
    layer3[2] = 0x000000UL;
    magnets = LOW;
  }
  // Set the last button state to the old state.
  oS1 = nS1;
  oS2 = nS2;

  drawDisplay();
  rotateRing();
}

void drawDisplay(){
  for (uint8_t i=0; i< PIXELS; i++){
    uint32_t c = layer1[i] + layer2[i] + layer3[i];
    uint8_t r = splitColor(c,'r');
    uint8_t g = splitColor(c,'g');
    uint8_t b = splitColor(c,'b');
    strip.setPixelColor(i,r,g,b);
    if (magnets == HIGH){
      strip.setPixelColor(1,0,0x1f,0);
      strip.setPixelColor(2,0,0x1f,0);
    }
    /*if (c > 0){
      strip.setPixelColor(i,r,g,b);
    } else {
      strip.setPixelColor(i,0,0,0);
    }*/
  }
  strip.show();
  delay(wait);
}

void rotateRing(){
  uint32_t tmpbuf = 0; // holds the value of layer1
  // Start with anti-clockwise layers
  tmpbuf = layer1[PIXELS - 1];
  for (uint8_t i=PIXELS - 1; i>0; i--){
      layer1[i] = layer1[i - 1];
  }
  layer1[0] = tmpbuf;
  // Clockwise layers
  tmpbuf = layer2[0];
  for (uint8_t i=0; i < PIXELS - 1; i++){
      layer2[i] = layer2[i + 1];
  }
  layer2[PIXELS -1] = tmpbuf;
}

uint8_t splitColor ( uint32_t c, char value )
{
  switch ( value ) {
    case 'r': return (uint8_t)(c >> 16);
    case 'g': return (uint8_t)(c >>  8);
    case 'b': return (uint8_t)(c >>  0);
    default:  return 0;
  }
}

uint32_t getColour(){
  uint32_t r = (rand() / (RAND_MAX / 0x3FUL + 1)) << 16;//(uint8_t)( 63UL << 16);
  uint16_t g = (rand() / (RAND_MAX / 0x1F + 1)) << 8;
  uint8_t b = (rand() / (RAND_MAX / 0x1F + 1)) << 0;
  //uint32_t red   = (uint32_t)(63UL << 16);//4128768; //63 << 16;
  //uint16_t green = (uint8_t)(63 <<  8);
  //uint8_t  blue  = (uint8_t)(63 <<  0);
  return r + g + b;//+green+blue;
}

void initRand(void)
{
        uint32_t state;
        static uint32_t EEMEM sstate;

        state = eeprom_read_dword(&sstate);

        // Check if it's unwritten EEPROM (first time).
        if (state == 0xffffffUL)
                state = 0xC0D3CAFE;
        srand(state);
        eeprom_write_dword(&sstate, rand());
} 

