/**
 * Stormbrella
 */

// Hardware.
unsigned int RELAYS[][2] = {
  {6, 0},
  {7, 0},
  {8, 0},
  {9, 0},
};
int array_size = 4;

// Animation.
const unsigned int DELAY = 50;
const unsigned int LIGHTNING_CHANCE = 2; // Out of 10000.
const unsigned int LIGHTNING_CHANCE_BOOST = 250; // Increase while active.
boolean LIGHTNING_ACTIVE = 0;
boolean LIGHTNING_CALM = 1;

/**
 * Startup.
 */
void setup () {
  randomSeed(analogRead(0));

  // Initialise the Arduino data pins for output.
  for (int i = 1; i < array_size; i++) {
    pinMode(RELAYS[i][0], OUTPUT);
    digitalWrite(RELAYS[i][0], LOW);    
  }
}

/**
 * Main.
 */
void loop () {
  lightning();

  delay(DELAY);
}


/**
 * Like a cloud in a storm.
 */
void lightning () {
  int chance = LIGHTNING_CHANCE;
  boolean anyOn = false;

  // Turn off all lights.
  for (int i = 0; i < array_size; i++) {
    digitalWrite(RELAYS[i][0], HIGH);

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
      int bulb = random(1, array_size);
      // Switch it on and keep track.
      digitalWrite(RELAYS[bulb][0], LOW);
      LIGHTNING_ACTIVE = 1;
      anyOn = true;
    }
  }
  
  if (!anyOn) {
    LIGHTNING_ACTIVE = 0;
  }  
}
