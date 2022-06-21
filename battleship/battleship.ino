// python-build-start
// /opt/arduino-1.8.19/arduino
// arduino:avr:uno
// /dev/ttyACM0
// python-build-end
  
//LEDS
#include <FastLED.h>
#define NUM_LEDS    144
#define LED_PIN     13
long unsigned int prevTime = 0;
int resetDelay = 250;
CRGB leds[NUM_LEDS];

//Buttons P1
const char UP = 12;
const char DOWN = 11;
const char LEFT = 10;
const char RIGHT = 9;
const char PLACE = 8;

//Buttons P2
const char UP2 = 7;
const char DOWN2 = 6;
const char LEFT2 = 5;
const char RIGHT2 = 4;
const char PLACE2 = 3;

// Positions
int x_pos = 4;
int y_pos = 4;

// Gameboard
const char PIECE_X = 'x';
const char PIECE_O = 'o';
const int PIECE_EMPTY = 0;
const int NUM_SHIPS = 4;
int game1[9][8]; // player 2 shots
int game2[9][8]; // player 1 shots
bool winner;
char who;
int hit1;
int hit2;

// 4 ships, (xdim, ydim, id)
int ships[4][3] = { {2, 1, 1}, {1, 2, 2}, {3, 1, 3}, {1, 3, 4} };


// LED grid mappings 
int board_side_1[9][8] = { {0, 1, 2, 3, 4, 5, 6, 7},
                           {15, 14, 13, 12, 11, 10, 9, 8},
                           {16, 17, 18, 19, 20, 21, 22, 23},
                           {31, 30, 29, 28, 27, 26, 25, 24},
                           {32, 33, 34, 35, 36, 37, 38, 39},
                           {47, 46, 45, 44, 43, 42, 41, 40},
                           {48, 49, 50, 51, 52, 53, 54, 55},
                           {63, 62, 61, 60, 59, 58, 57, 56},
                           {64, 65, 66, 67, 68, 69, 70, 71} };

int board_side_2[9][8] = { {136, 137, 138, 139, 140, 141, 142, 143},
                           {135, 134, 133, 132, 131, 130, 129, 128},
                           {120, 121, 122, 123, 124, 125, 126, 127},
                           {119, 118, 117, 116, 115, 114, 113, 112},
                           {104, 105, 106, 107, 108, 109, 110, 111},
                           {103, 102, 101, 100, 99, 98, 97, 96},
                           {88, 89, 90, 91, 92, 93, 94, 95},
                           {87, 86, 85, 84, 83, 82, 81, 80},
                           {72, 73, 74, 75, 76, 77, 78, 79} };

CRGB hitColor = CRGB(171, 35, 40); // dark red
CRGB shipColor = CRGB(255, 215, 0);  // gold
CRGB waterColor = CRGB(0, 0, 255);  // blue
CRGB deadColor = CRGB(12, 4, 4); // black-ish
CRGB missColor = CRGB(255, 255, 255); // white
CRGB eColor = CRGB(0, 0, 0);        // clear/empty

CRGB winnerColor = CRGB(0, 255, 0); // green
CRGB loserColor = CRGB(255, 0, 0); // red

void setup() {
    // reset positions
    x_pos = 4;
    y_pos = 4;
    winner = false;
    who = PIECE_X;
    hit1 = 0;
    hit2 = 0;

    // setup buttons
    pinMode(UP, INPUT_PULLUP);
    pinMode(DOWN, INPUT_PULLUP);
    pinMode(LEFT, INPUT_PULLUP);
    pinMode(RIGHT, INPUT_PULLUP);
    pinMode(PLACE, INPUT_PULLUP);

    pinMode(UP2, INPUT_PULLUP);
    pinMode(DOWN2, INPUT_PULLUP);
    pinMode(LEFT2, INPUT_PULLUP);
    pinMode(RIGHT2, INPUT_PULLUP);
    pinMode(PLACE2, INPUT_PULLUP);
    
    // setup ws2812b leds
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

    // reset led board to be water
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 8; j++) {
            leds[board_side_1[i][j]] = waterColor;
            leds[board_side_2[i][j]] = waterColor;
        }
    }
    FastLED.show();

    // reset game to be empty
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 8; j++) {
            game1[i][j] = PIECE_EMPTY;
            game2[i][j] = PIECE_EMPTY;
        }
    }

    // player 1 places ships
    for (int i = 0; i < NUM_SHIPS; i++) {
      int dimx = ships[i][0];
      int dimy = ships[i][1];
      int id = ships[i][2];
      setupcursor(dimx, dimy, id);
      x_pos = 4;
      y_pos = 4;
    }

    // player 2 places ships
    who = opposite(who);
    for (int i = 0; i < NUM_SHIPS; i++) {
      int dimx = ships[i][0];
      int dimy = ships[i][1];
      int id = ships[i][2];
      setupcursor2(dimx, dimy, id);
      x_pos = 4;
      y_pos = 4;
    }

    // reset led board to be water
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 8; j++) {
            leds[board_side_1[i][j]] = waterColor;
            leds[board_side_2[i][j]] = waterColor;
        }
    }
    FastLED.show();
    
    //switch back to player X
    who = opposite(who);
}

char opposite(char piece) {
// Return opposite of PIECE
    if (piece == PIECE_X) {
        return PIECE_O;
    }
    return PIECE_X;
}

void place(int dimx, int dimy, int x, int y, int id) {
// Place WHO ship at (x, y) with dimensions (dimx, dimy), assume placeable
  if (who == PIECE_X)
  {
    for (int i = x; i < x + dimx; i++) {
      for (int j = y; j < y + dimy; j++) {
        leds[board_side_1[i][j]] = shipColor;
        game1[i][j] = id;
      }
    }
  } else {
    for (int i = x; i < x + dimx; i++) {
      for (int j = y; j < y + dimy; j++) {
        leds[board_side_2[i][j]] = shipColor;
        game2[i][j] = id;
      }
    }
  }
  FastLED.show();
}

bool placeable(int dimx, int dimy, int x, int y) {
// Place WHO ship at (x, y) with dimensions (dimx, dimy), assume placeable
  if (who == PIECE_X) {
    for (int i = x; i < x + dimx; i++) {
      for (int j = y; j < y + dimy; j++) {
        if ((i < 0 || i >= 9 || j < 0 || j >= 8) || game1[i][j] != PIECE_EMPTY) {
          return false; 
        }
      }
    }
    return true;
  } else {
    for (int i = x; i < x + dimx; i++) {
      for (int j = y; j < y + dimy; j++) {
        if ((i < 0 || i >= 9 || j < 0 || j >= 8) || game2[i][j] != PIECE_EMPTY) {
          return false;
        }
      }
    }
    return true;
  }
}

bool fireable() {
  if (who == PIECE_X) {
    return game2[x_pos][y_pos] == PIECE_EMPTY;
  } 
  return game1[x_pos][y_pos] == PIECE_EMPTY;
}

void winnerblink() {
  // modify to blink only winning squares later
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 8; j++) {
      if (who == PIECE_X) {
        leds[board_side_1[i][j]] = winnerColor;
        leds[board_side_2[i][j]] = loserColor;
      } else {
        leds[board_side_1[i][j]] = loserColor;
        leds[board_side_2[i][j]] = winnerColor;          
      }
    }
  }
  FastLED.show();
}

void setupcursor(int x, int y, int id) {
// the cursor to place ships, blink potential ship placement
  x_pos = 4;
  y_pos = 4;
  bool placed = false;
  while (!placed) {
    cursorblink(x, y);
    if (digitalRead(LEFT) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            x_pos = max(x_pos-1, 0);
        }
        prevTime = currTime;
    }
    else if (digitalRead(RIGHT) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            x_pos = min(x_pos+1, 9-x);
        }
        prevTime = currTime;
    }
    else if (digitalRead(DOWN) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            y_pos = max(y_pos-1, 0);
        }
        prevTime = currTime;
    }
    else if (digitalRead(UP) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            y_pos = min(y_pos+1, 8-y);
        }
        prevTime = currTime;
    }
    else if (digitalRead(PLACE) == LOW && placeable(x, y, x_pos, y_pos)) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            place(x, y, x_pos, y_pos, id);
        }
        placed = true;
        prevTime = currTime;
    }
  }
}

void setupcursor2(int x, int y, int id) {
// the cursor to place ships, blink potential ship placement
  x_pos = 4;
  y_pos = 4;
  bool placed = false;
  while (!placed) {
    cursorblink(x, y);
    if (digitalRead(LEFT2) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            x_pos = max(x_pos-1, 0);
        }
        prevTime = currTime;
    }
    else if (digitalRead(RIGHT2) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            x_pos = min(x_pos+1, 9-x);
        }
        prevTime = currTime;
    }
    else if (digitalRead(DOWN2) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            y_pos = max(y_pos-1, 0);
        }
        prevTime = currTime;
    }
    else if (digitalRead(UP2) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            y_pos = min(y_pos+1, 8-y);
        }
        prevTime = currTime;
    }
    else if (digitalRead(PLACE2) == LOW && placeable(x, y, x_pos, y_pos)) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            place(x, y, x_pos, y_pos, id);
        }
        placed = true;
        prevTime = currTime;
    }
  }
}


void cursorblink(int dimx, int dimy) {
    CRGB tmp[dimx][dimy];
    // blink for placement of ship
    for (int i = x_pos; i < x_pos + dimx; i++) {
      for (int j = y_pos; j < y_pos + dimy; j++) {
        if (who == PIECE_X) {
          tmp[i-x_pos][j-y_pos] = leds[board_side_1[i][j]];
          leds[board_side_1[i][j]] = shipColor;
        } else {
          tmp[i-x_pos][j-y_pos] = leds[board_side_2[i][j]];
          leds[board_side_2[i][j]] = shipColor;
        }
      }
    }
    FastLED.show();
    delay(500);
    for (int i = x_pos; i < x_pos + dimx; i++) {
      for (int j = y_pos; j < y_pos + dimy; j++) {
        if (who == PIECE_X) {
          leds[board_side_1[i][j]] = tmp[i-x_pos][j-y_pos];
        } else {
          leds[board_side_2[i][j]] = tmp[i-x_pos][j-y_pos];
        }
      }
    }
    FastLED.show();
    delay(100);
}

// check if there are enough hits for a player to have won
bool win() {
  if (hit1 == 10) {
    winner = PIECE_X;
    return true;
  } else if (hit2 == 10) {
    winner = PIECE_O;
    return true;
  } else {
    return false;
  }
}

// check if any ships are sunk, if any are, change that ship's color to dead
void checkForSink(int id) {
  if (who == PIECE_X) {
    for (int i = 0; i < 9; i++) { 
      for (int j = 0; j < 8; j++) {
        if (game2[i][j] == id) {
          return;
        }
      }
    }
    for (int i = 0; i < 9; i++) { 
      for (int j = 0; j < 8; j++) {
        if (game2[i][j] == -id) {
          leds[board_side_1[i][j]] = deadColor;
        }
      }
    }
  } else {
    for (int i = 0; i < 9; i++) { 
      for (int j = 0; j < 8; j++) {
        if (game1[i][j] == id) {
          return;
        }
      }
    }
    for (int i = 0; i < 9; i++) { 
      for (int j = 0; j < 8; j++) {
        if (game1[i][j] == -id) {
          leds[board_side_2[i][j]] = deadColor;
        }
      }
    }
  }
}

// fire at (x, y)
void fire(int x, int y) {
  if (who == PIECE_X) {
    if (game2[x][y] > 0) {
      int id = game2[x][y];
      game2[x][y] = game2[x][y] * -1;
      hit1++;
      leds[board_side_1[x][y]] = hitColor;
      checkForSink(id);
    } else if (game2[x][y] == 0) {
      leds[board_side_1[x][y]] = missColor;
      game2[x][y] = -10;
    }
  } else {
    if (game1[x][y] > 0) {
      int id = game1[x][y];
      game1[x][y] = game1[x][y] * -1;
      hit2++;
      leds[board_side_2[x][y]] = hitColor;
      checkForSink(id);
    } else if (game1[x][y] == 0) {
      leds[board_side_2[x][y]] = missColor;
      game1[x][y] = -10;
    }
  }
  FastLED.show();
}


void gamecursor() {
  x_pos = 4;
  y_pos = 4;
  bool placed = false;
  while (!placed) {
    cursorblink(1, 1);
    if (digitalRead(LEFT) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            x_pos = max(x_pos-1, 0);
        }
        prevTime = currTime;
    }
    else if (digitalRead(RIGHT) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            x_pos = min(x_pos+1, 8);
        }
        prevTime = currTime;
    }
    else if (digitalRead(DOWN) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            y_pos = max(y_pos-1, 0);
        }
        prevTime = currTime;
    }
    else if (digitalRead(UP) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            y_pos = min(y_pos+1, 7);
        }
        prevTime = currTime;
    }
    else if (digitalRead(PLACE) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            fire(x_pos, y_pos);
        }
        winner = win();
        placed = true;
        prevTime = currTime;
    }
  }
  if (!winner) {
    who = opposite(who);
  }
}
void gamecursor2() {
  x_pos = 4;
  y_pos = 4;
  bool placed = false;
  while (!placed) {
    cursorblink(1, 1);
    if (digitalRead(LEFT2) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            x_pos = max(x_pos-1, 0);
        }
        prevTime = currTime;
    }
    else if (digitalRead(RIGHT2) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            x_pos = min(x_pos+1, 8);
        }
        prevTime = currTime;
    }
    else if (digitalRead(DOWN2) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            y_pos = max(y_pos-1, 0);
        }
        prevTime = currTime;
    }
    else if (digitalRead(UP2) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            y_pos = min(y_pos+1, 7);
        }
        prevTime = currTime;
    }
    else if (digitalRead(PLACE2) == LOW) {
        long unsigned int currTime = millis();
        if (currTime - prevTime > 10) {
            fire(x_pos, y_pos);
        }
        winner = win();
        placed = true;
        prevTime = currTime;
    }
  }
  if (!winner) {
    who = opposite(who);
  }
}

void loop() {
    if (!winner) {
      if (who == PIECE_X)
      {
        gamecursor();
      }
      else {
        gamecursor2();
      }
    } else {
      winnerblink();
      delay(10000);
      reset();
    }
}

void reset() {
    x_pos = 4;
    y_pos = 4;
    winner = false;
    who = PIECE_X;
    hit1 = 0;
    hit2 = 0;

    // reset led board to be water
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 8; j++) {
            leds[board_side_1[i][j]] = waterColor;
            leds[board_side_2[i][j]] = waterColor;
        }
    }
    FastLED.show();

    // reset game to be empty
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 8; j++) {
            game1[i][j] = PIECE_EMPTY;
            game2[i][j] = PIECE_EMPTY;
        }
    }

    // player 1 places ships
    for (int i = 0; i < NUM_SHIPS; i++) {
      int dimx = ships[i][0];
      int dimy = ships[i][1];
      int id = ships[i][2];
      setupcursor(dimx, dimy, id);
      x_pos = 4;
      y_pos = 4;
    }

    // player 2 places ships
    who = opposite(who);
    for (int i = 0; i < NUM_SHIPS; i++) {
      int dimx = ships[i][0];
      int dimy = ships[i][1];
      int id = ships[i][2];
      setupcursor2(dimx, dimy, id);
      x_pos = 4;
      y_pos = 4;
    }

    // reset led board to be water
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 8; j++) {
            leds[board_side_1[i][j]] = waterColor;
            leds[board_side_2[i][j]] = waterColor;
        }
    }
    FastLED.show();

    //switch back to player X
    who = opposite(who);
}
