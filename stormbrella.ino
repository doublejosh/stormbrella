/**
 * Stormbrella
 */

#include <Adafruit_NeoPixel.h>

// Hardware.
unsigned int RELAYS[][2] = {
               {6, 0},
               {7, 0},
               {8, 0},
               {9, 0},
             },
             array_size = 4;

// Rain.
const int RAIN_PIN = 3,
          NUM_LIGHTS = 35;

unsigned int COLOR_CYCLE = 0;

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUM_LIGHTS, RAIN_PIN, NEO_GRB + NEO_KHZ800);

// Animation.
const unsigned int DELAY = 50,
                   LIGHTNING_CHANCE = 10, // Out of 10000.
                   LIGHTNING_CHANCE_BOOST = 250; // Increase while active.

boolean LIGHTNING_ACTIVE = 0,
        LIGHTNING_CALM = 1,
        DEBUG = true;

/**
 * Startup.
 */
void setup () {
  randomSeed(analogRead(0));

  // Rain.
  strip1.begin();
  strip1.show();
  pinMode(RAIN_PIN, OUTPUT);

  // Initialize lightning pins for output.
  for (int i = 0; i < array_size; i++) {
    pinMode(RELAYS[i][0], OUTPUT);
    digitalWrite(RELAYS[i][0], LOW);    
  }

  if (DEBUG) {
    Serial.begin(9600);
  }
}

/**
 * Main.
 */
void loop () {
  lightning();
  rainbowCycle();
  delay(DELAY);
}


/**
 * Rainbow test pattern.
 */
void rainbowCycle () {
  uint16_t i, j;

  if (COLOR_CYCLE > 256) {
    COLOR_CYCLE = 0;
  }

  for (i = 0; i < strip1.numPixels(); i++) {
    strip1.setPixelColor(i, Wheel(((i * 256 / strip1.numPixels()) + COLOR_CYCLE) & 255));
  }
  strip1.show();
  
  COLOR_CYCLE++;
}


/**
 * Like a cloud in a storm.
 */
void lightning () {
  int chance = LIGHTNING_CHANCE;
  boolean anyOn = false;

  // All bulbs.
  for (int i = 0; i < array_size; i++) {
    // Turn off first.
    digitalWrite(RELAYS[i][0], LOW);

    // Increase chances for clusters.
    if (LIGHTNING_ACTIVE) {
      chance = LIGHTNING_CHANCE + LIGHTNING_CHANCE_BOOST;
    }
    else {
      chance = LIGHTNING_CHANCE;
    }

    // Determine if flashing.
    if (random(0, 1000) < chance) {
      // Pick a bulb.
      int bulb = random(0, array_size);
      // Switch it on and keep track.
      digitalWrite(RELAYS[bulb][0], HIGH);
      LIGHTNING_ACTIVE = 1;
      anyOn = true;

      if (DEBUG) {
        Serial.print(bulb);
        Serial.print(" - ");
        Serial.println(chance);
      }
    }
  }
  
  if (!anyOn) {
    LIGHTNING_ACTIVE = 0;
  }  
}

/**
 * Get color from 0 to 255 input.
 */
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
   return strip1.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170) {
   WheelPos -= 85;
   return strip1.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else {
   WheelPos -= 170;
   return strip1.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
