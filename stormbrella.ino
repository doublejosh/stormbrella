/**
 * Stormbrella
 */

#include <Adafruit_NeoPixel.h>

const unsigned int LIGHTNING_BULBS = 4,
                   RAIN_COLS = 4,
                   RAIN_ROWS = 35,
                   RAIN_PINS[] = {
                     2,
                     3,
                     4,
                     5
                   },
                   DELAY = 30,
                   LIGHTNING_CHANCE = 2, // Out of 10000.
                   LIGHTNING_CHANCE_BOOST = 400, // Increase while active.
                   MAX_BRIGHT = 50;

unsigned int RELAYS[][2] = {
               {6, 0},
               {7, 0},
               {8, 0},
               {9, 0},
             },
             COLOR_CYCLE = 0,
             rain_matrix[RAIN_COLS][RAIN_ROWS] = {0},
             chance_of_rain = 2,
             chance_of_rain_min = 2;

boolean LIGHTNING_ACTIVE = 0,
        LIGHTNING_CALM = 1,
        INVERT_RAIN = true,
        DEBUG = false;

uint32_t bg_color = Color(0, 0, 0),
         rain_color = Color(50, 100, 240),
         lightning_color = Color(255, 240, 50);

Adafruit_NeoPixel strands[] = {
  Adafruit_NeoPixel(RAIN_ROWS, RAIN_PINS[0], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(RAIN_ROWS, RAIN_PINS[1], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(RAIN_ROWS, RAIN_PINS[2], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(RAIN_ROWS, RAIN_PINS[3], NEO_GRB + NEO_KHZ800)
};


/**
 * Startup.
 */
void setup () {
  randomSeed(analogRead(0));

  // Rain.
  for (int i = 0; i < RAIN_COLS; i++) {
    pinMode(RAIN_PINS[i], OUTPUT);
    strands[i].begin();
  }

  // Initialize lightning pins for output.
  for (int i = 0; i < LIGHTNING_BULBS; i++) {
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
  rain();
  //rainbowCycle();
  delay(DELAY);
}


/**
 * Drops falling.
 */
void rain() {
  int r, c;

  make_drops();

  // Display drops.
  for (c = 0; c < RAIN_COLS; c++) {
    for (r = 0; r < RAIN_ROWS; r++) {
      if (rain_matrix[c][r] > 0) {
        draw_drop(c, r, true);
        move_drop(c, r);
      }
      else {
        draw_drop(c, r, false);
      }
    } 
  }
  for (int i = 0; i < RAIN_COLS; i++) {
    strands[i].show();
  }
}

/**
 * Rainbow test pattern.
 */
void rainbowCycle () {
  uint16_t i, j;

  if (COLOR_CYCLE > 256) {
    COLOR_CYCLE = 0;
  }

  for (i = 0; i < strands[0].numPixels(); i++) {
    strands[0].setPixelColor(i, Wheel(((i * 256 / strands[0].numPixels()) + COLOR_CYCLE) & 255));
  }
  strands[0].show();
}


/**
 * Generate new rain drops.
 **/
void make_drops () {
  // Change the weather?
  switch (random(0, 3)) {
    case 1:
      chance_of_rain += 1;
      break;
      
    case 2:
      chance_of_rain -= 1;
      break;
      
    case 3:
    default:
      break;
  }

  // Protect boundaries.
  if (chance_of_rain <= 1) {
   chance_of_rain = 2;
  }
  else if (chance_of_rain > chance_of_rain_min) {
    chance_of_rain = chance_of_rain_min;
  }

  // Add a new drop?
  if (random(0, chance_of_rain) == 1) {
    // Pick a column
    int c = random(0, RAIN_COLS);
    int r = random(0, RAIN_ROWS);
    
     rain_matrix[c][r] = 1;

     if (DEBUG) {
       Serial.print(c);
       Serial.println(" - 0");
     }
  }

}

/**
 * Shift known drops to new position.
 */
void move_drop (int c, int r) {  
  int save = rain_matrix[c][r];
  // Out with the old.
  rain_matrix[c][r] = 0;
  // Can't move it on the ground.
  if (r > 0) {
    rain_matrix[c][r - 1] = save;

    if (DEBUG) {
      Serial.print(c);
      Serial.print(" - ");
      Serial.println(r);
    }
  }
}


/**
 * Translate drop positions into data output.
 */
void draw_drop (int c, int r, boolean rain) {
  if (INVERT_RAIN) {
    r = RAIN_ROWS - r;
  }
  
  if (rain) {
    strands[c].setPixelColor(r, rain_color);
  }
  else {
    strands[c].setPixelColor(r, bg_color);
  }
}


/**
 * Like a cloud in a storm.
 */
void lightning () {
  int chance = LIGHTNING_CHANCE;
  boolean anyOn = false;

  // All bulbs.
  for (int i = 0; i < LIGHTNING_BULBS; i++) {
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
      int bulb = random(0, LIGHTNING_BULBS);
      // Switch it on and keep track.
      digitalWrite(RELAYS[bulb][0], HIGH);
      LIGHTNING_ACTIVE = 1;
      anyOn = true;

      // Pick a column.
      int c = random(0, RAIN_COLS);
      for (int r = 0; r < strands[0].numPixels(); r++) {
        strands[c].setPixelColor(r, lightning_color);
      }
      strands[c].show();

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
   return strands[0].Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170) {
   WheelPos -= 85;
   return strands[0].Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else {
   WheelPos -= 170;
   return strands[0].Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

/**
 * Create a 24 bit color value from R, G, B.
 */
uint32_t Color(byte r, byte g, byte b) {
  r = map(r, 0, 255, 0, MAX_BRIGHT);
  g = map(g, 0, 255, 0, MAX_BRIGHT);
  b = map(b, 0, 255, 0, MAX_BRIGHT);
  
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

