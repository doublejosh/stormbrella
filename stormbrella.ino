/**
 * Stormbrella
 * 
 * A weather related light show inside your umbrella,
 * half-written in tents under duress.
 */

#include <Adafruit_NeoPixel.h>

const unsigned int LIGHTNING_BULBS = 4,
                   RAIN_COLS = 8,
                   RAIN_ROWS = 36,
                   RAIN_PINS[] = {2, 3, 4, 5, 6, 7, 8, 9},
                   DELAY = 40,
                   LIGHTNING_CHANCE = 13, // Out of 10000.
                   LIGHTNING_CHANCE_BOOST = 3800, // Increase while active.
                   LIGHTNING_MIN_LEN = 18,
                   MAX_BRIGHT = 30;

unsigned int COLOR_CYCLE = 0,
             chance_of_rain = 2, // Less is more.
             chance_of_rain_min = 2,
             rainbow_cycle = 0,
             spiral_position = 0;

uint8_t RELAYS[][2] = {{10, 0}, {11, 0}, {12, 0}, {13, 0}},
        rain_matrix[RAIN_COLS][RAIN_ROWS] = {0};

boolean LIGHTNING_ACTIVE = 0,
        INVERT_RAIN = true,
        RAINBOW_MODE = true,
        DEBUG = true;

uint32_t bg_color = Color(0, 0, 0),
         rain_color = Color(50, 100, 240),
         lightning_color = Color(255, 240, 50),
         rainbow_cycle_colors[] = {
           Color(255, 0 , 0),
           Color(255, 127, 0),
           Color(255, 255, 0),
           Color(0, 255, 0),
           Color(0, 0, 255),
           Color(75, 0, 130),
           Color(0, 0, 0)
         };

Adafruit_NeoPixel strands[] = {
  Adafruit_NeoPixel(RAIN_ROWS, RAIN_PINS[0], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(RAIN_ROWS, RAIN_PINS[1], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(RAIN_ROWS, RAIN_PINS[2], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(RAIN_ROWS, RAIN_PINS[3], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(RAIN_ROWS, RAIN_PINS[4], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(RAIN_ROWS, RAIN_PINS[5], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(RAIN_ROWS, RAIN_PINS[6], NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(RAIN_ROWS, RAIN_PINS[7], NEO_GRB + NEO_KHZ800)
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
  //spiral();
  //rainbowCycle();
  delay(DELAY);
}


/**
 * Drops falling.
 */
void rain() {
  int r, c;

  if (DEBUG) {
    Serial.println("RAINING");
  }

  make_drops();

  // Display drops.
  for (c = 0; c < RAIN_COLS; c++) {
    for (r = 0; r < RAIN_ROWS; r++) {     
      draw_pixel(c, r);
      if (rain_matrix[c][r] > 0) {
        move_drop(c, r);
      }
    }
    strands[c].show();
  }
}

/**
 * Rainbow test pattern.
 */
void rainbowCycle () {
  uint16_t i, s;

  if (COLOR_CYCLE > 256) {
    COLOR_CYCLE = 0;
  }

  for (s = 0; s < RAIN_COLS; s++) {
    for (i = 0; i < strands[0].numPixels(); i++) {
      strands[s].setPixelColor(i, Wheel(((i * 256 / strands[s].numPixels()) + COLOR_CYCLE) & 255));
    }
    strands[s].show();
  }
}


/**
 * Generate new rain drops.
 **/
void make_drops () {
  int column,
      row;

  if (DEBUG) {
    Serial.println("Making");
  }

  // Change the weather?
//  switch (random(0, 3)) {
//    case 1:
//      chance_of_rain += 1;
//      break;
//      
//    case 2:
//      chance_of_rain -= 1;
//      break;
//      
//    case 3:
//    default:
//      break;
//  }

  // Protect boundaries.
//  if (chance_of_rain < chance_of_rain_min) {
//    chance_of_rain = chance_of_rain_min;
//  }

  if (DEBUG) {
    Serial.print("CHANCE OF RAIN: ");
    Serial.println(chance_of_rain);
  }

  // Add a new drop?
  if (random(0, chance_of_rain) == 1) {
    // Pick a spot.
    column = random(0, RAIN_COLS),
    row = random(0, RAIN_ROWS);
    rain_matrix[column][row] = rainbow_cycle; //1;

    if (DEBUG) {
      Serial.print("New Drop: ");
      Serial.print(column);
      Serial.print(" - ");
      Serial.println(row);
    }

    // Next color.
    rainbow_cycle++;
    if (rainbow_cycle > 6) {
      rainbow_cycle = 0;
    }
  
//    if (DEBUG) {
//      Serial.print(column);
//      Serial.print(" - ");
//      Serial.println(rainbow_cycle);
//    }
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

//    if (DEBUG) {
//      Serial.print(c);
//      Serial.print(" - ");
//      Serial.println(r);
//    }
  }
}


/**
 * Translate drop positions into data output.
 */
void draw_pixel (int c, int r) {
  uint32_t drop_color = bg_color;

  if (DEBUG && rain_matrix[c][r] == 1) {
    Serial.print(c);
    Serial.print(" - ");
    Serial.print(r);
    Serial.print(" - ");
    Serial.println(rain_matrix[c][r]);
  }

  if (rain_matrix[c][r] > 0) {
    if (RAINBOW_MODE) {
      drop_color = rainbow_cycle_colors[rain_matrix[c][r]];
    }
    else {
      drop_color = rain_color;
    }
  }
  
  if (INVERT_RAIN) {
    r = RAIN_ROWS - (r + 1);
  }
  strands[c].setPixelColor(r, drop_color);
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
    if (random(0, 10000) < chance) {
      // Pick a bulb.
      int bulb = random(0, LIGHTNING_BULBS);
      // Switch it on and keep track.
      digitalWrite(RELAYS[bulb][0], HIGH);
      LIGHTNING_ACTIVE = 1;
      anyOn = true;

      // Pick a column.
      int c = random(0, RAIN_COLS),
          length = random(LIGHTNING_MIN_LEN, strands[0].numPixels());
      for (int r = 0; r < length; r++) {
        strands[c].setPixelColor(r, lightning_color);
      }
      strands[c].show();

      if (DEBUG) {
        Serial.println("-----");
        Serial.print(bulb);
        Serial.println(" -- ");
        Serial.println(chance);
      }
    }
  }
  
  if (!anyOn) {
    LIGHTNING_ACTIVE = 0;
  }  
}


/**
 * Round and round.
 */
void spiral() {
  int column = spiral_position % RAIN_COLS,
      color = spiral_position % 7;
      //row = (int) spiral_position / RAIN_COLS,
      //prev_col = (spiral_position - 1) % RAIN_COLS,
      //prev_row = ((int) spiral_position - 1) / RAIN_COLS;
//      prev_col2 = (spiral_position - 2) % RAIN_COLS,
//      prev_row2 = ((int) spiral_position - 2) / RAIN_COLS;

//  for (int c = 0; c < RAIN_COLS; c++) {
//    for (int r = 0; r < RAIN_ROWS; r++) {
//      strands[c].setPixelColor(r, bg_color);
//    }
//    strands[c].show();
//  }

  for (int r = 0; r < RAIN_ROWS; r++) {
    strands[column].setPixelColor(r, rainbow_cycle_colors[color]);
  }

  //strands[column].setPixelColor(row, rain_color);
  //strands[prev_col].setPixelColor(prev_row, rain_color);
  //strands[prev_col2].setPixelColor(prev_row2, bg_color);
  strands[column].show();
  //strands[prev_col].show();
  //strands[prev_col2].show();
//  
  if (DEBUG) {
    Serial.print(column);
    Serial.print(" - ");
    Serial.println(color);
//    Serial.print(" ----- ");
//    Serial.print(prev_col);
//    Serial.print(" - ");
//    Serial.println(prev_row);
  }
  
  spiral_position++;
  
  if (column > RAIN_COLS) {
    spiral_position = 0;
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

