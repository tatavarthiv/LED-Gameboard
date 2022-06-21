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

//Buttons
const char LEFT1 = 10;
const char RIGHT1 = 9;
const char LEFT2 = 5;
const char RIGHT2 = 4;
const char PLACE1 = 8;
const char PLACE2 = 3;

// Directions
#define DIR_UP    0
#define DIR_DOWN  1
#define DIR_LEFT  2
#define DIR_RIGHT 3

// Positions
int x_pos = 0;
int y_pos = 7;

// Gameboard
const char PIECE_X = 'x';
const char PIECE_O = 'o';
const char PIECE_EMPTY = '-';
int game[9][8];
bool winner;
char who;
int markers = 0;


// --#--#--
// --#--#--
// ########
// --#--#--
// --#--#--
// ########
// --#--#--
// --#--#--

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

CRGB oColor = CRGB(255, 0, 0); // red
CRGB xColor = CRGB(0, 0, 255); // blue
CRGB wColor = CRGB(255, 255, 255); // white
CRGB eColor = CRGB(0, 0, 0); // clear/empty
CRGB gColor = CRGB(0, 255, 0); //green

void setup() {
    // reset positions
    x_pos = 0;
    y_pos = 7;
    winner = false;
    who = PIECE_X;

    // setup buttons
    pinMode(LEFT1, INPUT_PULLUP);
    pinMode(RIGHT1, INPUT_PULLUP);
    pinMode(LEFT2, INPUT_PULLUP);
    pinMode(RIGHT2, INPUT_PULLUP);
    pinMode(PLACE1, INPUT_PULLUP);
    pinMode(PLACE2, INPUT_PULLUP);

    // setup ws2812b leds
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

    // reset led board to be empty
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 8; j++) {
            leds[board_side_1[i][j]] = eColor;
            leds[board_side_2[i][j]] = eColor;
        }
    }

    // reset game to be empty
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 8; j++) {
            game[i][j] = PIECE_EMPTY;
        }
    }
}

char opposite(char piece) {
// Return opposite of PIECE

    if (piece == PIECE_X) {
        return PIECE_O;
    }
    return PIECE_X;
}

void place() {
// Place WHO at X_POS
  bool placed = false;
  for (int i = 0; i < 8; i++) {
      if (game[x_pos][i] == PIECE_EMPTY) {
          if (who == PIECE_X) {
              leds[board_side_1[x_pos][i]] = xColor;
              leds[board_side_2[x_pos][i]] = xColor;
              game[x_pos][i] = PIECE_X;
          } else {
              leds[board_side_1[x_pos][i]] = oColor;
              leds[board_side_2[x_pos][i]] = oColor;
              game[x_pos][i] = PIECE_O;
          }
          FastLED.show();
          placed = true;
          break;
      }
  }

  if (!placed) {
      if (who == PIECE_X) {
          bluecursor();
      }
      else {
          redcursor();
      }
  }

  //add check for win here later
  if (checkForWin(who, game)) {
      winner = true;
  } else {
      who = opposite(who);
  }
  return;
}

bool checkForWin(char player, int tmp[9][8]) {
// Check if there is a win for player.
  // horizontalCheck 
  for (int i = 0; i < 6; i++){
      for (int j = 0; j < 8; j++){
          if (tmp[i][j] == player && tmp[i][j+1] == player && tmp[i][j+2] == player && tmp[i][j+3] == player){
              return true;
          }           
      }
  }
  // verticalCheck
  for (int i = 0; i < 9; i++){
      for (int j = 0; j < 5; j++){
          if (tmp[i][j] == player && tmp[i+1][j] == player && tmp[i+2][j] == player && tmp[i+3][j] == player){
              return true;
          }           
      }
  }
  // ascendingDiagonalCheck 
  for (int i=3; i < 9; i++){
      for (int j=0; j < 5; j++){
          if (tmp[i][j] == player && tmp[i-1][j+1] == player && tmp[i-2][j+2] == player && tmp[i-3][j+3] == player)
              return true;
      }
  }
  // descendingDiagonalCheck
  for (int i=3; i < 9; i++){
      for (int j=3; j < 8; j++){
          if (tmp[i][j] == player && tmp[i-1][j-1] == player && tmp[i-2][j-2] == player && tmp[i-3][j-3] == player)
              return true;
      }
  }
  return false;
}


void winnerblink() {
  //modify to blink only winning squares later
  if (winner) {
    for (int i = 0; i < NUM_LEDS; i++) {
      if (who == PIECE_X) {
        leds[i] = xColor;
      }
      else {
        leds[i] = oColor;
      }
    }
    FastLED.show();
  }
  return;
}

void redcursor() {
  findBestMove();
  delay(1000);
}

void findBestMove() {
  int bestX = -1;
  int bestY = -1;
  if (who == PIECE_X) {
    int bestMove = -2;
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 8; j++) {
        if (game[i][j] == PIECE_EMPTY){
          int tmp2[9][8];
          memcpy(tmp2, game, 18);
          tmp2[i][j] = who;
          int value = minimax(tmp2, markers+1, opposite(who), false);
          if (value >= bestMove) {
            bestMove = value;
            bestX = i;
            bestY = j;
          }
        }
      }
    }
  } else {
    int bestMove = 2;
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 8; j++) {
        if (game[i][j] == PIECE_EMPTY){
          int tmp2[9][8];
          memcpy(tmp2, game, 18);
          tmp2[i][j] = who;
          Serial.write("minimax");
          int value = minimax(tmp2, markers+1, opposite(who), true);
          // DEBUGGING
          Serial.write("Value: ");
          Serial.println(value);
          Serial.write("Coord: ");
          Serial.print(i);
          Serial.write(" ");
          Serial.println(j);
          Serial.write("================");
          Serial.println();
          if (value <= bestMove) {
            bestMove = value;
            bestX = i;
            bestY = j;
          }
        }
      }
    }
  }
  x_pos = bestX;
  y_pos = bestY;
  place();
  // DEBUGGING
  Serial.write("================");
  Serial.println();
  Serial.write("================");
  Serial.println();
}


int minimax(int tmp[9][8], int mark, char player, bool optimizer) {
  if (checkForWin(opposite(player), tmp)) {
    if (player == PIECE_X) {
      return -1;
    } else {
      return 1;
    }
  } else if (mark == 72) {
    return 0;
  }
  if (optimizer) {
    int bestVal = -2;
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 8; j++) {
        if (tmp[i][j] == PIECE_EMPTY) {
          int tmp2[9][8];
          memcpy(tmp2, tmp, 18);
          tmp2[i][j] = player;
          int value = minimax(tmp2, mark+1, opposite(player), !optimizer);
          bestVal = max(bestVal, value);
        }
      }
    }
    return bestVal;
  } else {
    int bestVal = 2;
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 8; j++) {
        if (tmp[i][j] == PIECE_EMPTY) {
          int tmp2[9][8];
          memcpy(tmp2, tmp, 18);
          tmp2[i][j] = player;
          int value = minimax(tmp2, mark+1, opposite(player), !optimizer);
          bestVal = min(bestVal, value);
        }
      }
    }
    return bestVal;
  }
}

void bluecursor() {
  bool placed = false;
  while (!placed) {
      blueblink();
      if (digitalRead(LEFT1) == LOW) {
          long unsigned int currTime = millis();
          if (currTime - prevTime > 10) {
              x_pos = max(x_pos-1, 0);
          }
          prevTime = currTime;
      }
      else if (digitalRead(RIGHT1) == LOW) {
          long unsigned int currTime = millis();
          if (currTime - prevTime > 10) {
              x_pos = min(x_pos+1, 8);
          }
          prevTime = currTime;
      }
      else if (digitalRead(PLACE1) == LOW) {
          long unsigned int currTime = millis();
          if (currTime - prevTime > 10) {
              place();
          }
          prevTime = currTime;
          placed = true;
      }
  }
  return;
}
    
void redblink() {
    if (game[x_pos][y_pos] == PIECE_EMPTY) {
      leds[board_side_2[x_pos][y_pos]] = gColor;
      FastLED.show();
      delay(500);
      leds[board_side_2[x_pos][y_pos]] = eColor;
      FastLED.show();
      delay(100);
    }
    return;
}

void blueblink() {
    if (game[x_pos][y_pos] == PIECE_EMPTY) {
      leds[board_side_1[x_pos][y_pos]] = gColor;
      FastLED.show();
      delay(500);
      leds[board_side_1[x_pos][y_pos]] = eColor;
      FastLED.show();
      delay(100);
    }
    return;
}

void loop() {
    if (!winner) {
      bluecursor();
      if (winner) {
          delay(2000);
          winnerblink();
          delay(3000);
          reset();
          bluecursor();
      }
      redcursor();
      if (winner) {
          delay(2000);
          winnerblink();
          delay(3000);
          reset();
      }
    }
}

void reset() {
  winner = false;
  who = PIECE_X;
  x_pos = 0;
  y_pos = 7;
  
  // reset led board to be empty
  for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 8; j++) {
          leds[board_side_1[i][j]] = eColor;
          leds[board_side_2[i][j]] = eColor;
      }
  }

  // reset game to be empty
  for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 8; j++) {
          game[i][j] = PIECE_EMPTY;
      }
  }
}
