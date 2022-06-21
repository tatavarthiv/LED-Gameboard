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
const char UP_PIN = 12;
const char DOWN_PIN = 11;
const char LEFT_PIN = 10;
const char RIGHT_PIN = 9;
const char PLACE_PIN = 8;


// Directions
#define DIR_UP    0
#define DIR_DOWN  1
#define DIR_LEFT  2
#define DIR_RIGHT 3

// Positions
int x_pos = 0;
int y_pos = 0;

// Gameboard
const char PIECE_X = 'x';
const char PIECE_O = 'o';
const char PIECE_EMPTY = '-';
int game[3][3];
int markers = 0;
bool winner;
char who;


// --#--#--
// --#--#--
// ########
// --#--#--
// --#--#--
// ########
// --#--#--
// --#--#--

// LED grid mappings 
int board1[3][3][4] = { {{14, 15, 16, 17}, {32, 33, 46, 47}, {62, 63, 64, 65}},
                {{11, 12, 19, 20}, {35, 36, 43, 44}, {59, 60, 67, 68}}, 
                {{8, 9, 22, 23}, {38, 39, 40, 41}, {56, 57, 70, 71}} };
int board2[3][3][4] = { {{134, 135, 121, 120}, {104, 105, 102, 103}, {86, 87, 73, 72}},
                        {{131, 132, 124, 123}, {107, 108, 99, 100}, {83, 84, 75, 76}},
                        {{128, 129, 126, 127}, {111, 110, 96, 97}, {80, 81, 79, 78}} };

CRGB oColor = CRGB(255, 0, 0); // red
CRGB xColor = CRGB(0, 0, 255); // blue
CRGB wColor = CRGB(255, 255, 255); // white
CRGB eColor = CRGB(0, 0, 0); // clear/empty
CRGB gColor = CRGB(0, 255, 0); //green
CRGB tieColor = CRGB(255, 215, 0); //gold

void setup() {
    // reset positions
    x_pos = 0;
    y_pos = 0;
    winner = false;
    markers = 0;
    who = PIECE_X;
    Serial.begin(9600);
    // setup buttons
    pinMode(UP_PIN, INPUT_PULLUP);
    pinMode(DOWN_PIN, INPUT_PULLUP);
    pinMode(LEFT_PIN, INPUT_PULLUP);
    pinMode(RIGHT_PIN, INPUT_PULLUP);
    pinMode(PLACE_PIN, INPUT_PULLUP);

    // setup ws2812b leds
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    
    // setup grid lines 
    int grid[56] = {10, 13, 18, 21, 24, 25, 26, 27, 28, 29, 30, 31, 34, 37, 42, 45, 48, 49, 50, 51, 52, 53, 54, 55, 58, 61, 66, 69, 130, 125, 109, 98, 82,
                     77, 133, 122, 106, 101, 90, 85, 74, 112, 113, 114, 115, 116, 117, 118, 119, 95, 94, 93, 92, 91, 89, 88};
    for (int i = 0; i < 56; i++) {
        leds[grid[i]] = wColor;
        FastLED.show();
    }

    // reset led board to be empty
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                leds[board1[i][j][k]] = eColor;
                leds[board2[i][j][k]] = eColor;
            }
        }
    }

    // reset game to be empty
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
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
// Place WHO at X_POS, Y_POS
    if (game[x_pos][y_pos] != PIECE_EMPTY) {
        return;
    }
    game[x_pos][y_pos] = who;
    markers += 1;
    for (int i = 0; i < 4; i++) {
        if (who == PIECE_X) {
            leds[board1[x_pos][y_pos][i]] = xColor;
            leds[board2[x_pos][y_pos][i]] = xColor;
        } else {
            leds[board1[x_pos][y_pos][i]] = oColor;
            leds[board2[x_pos][y_pos][i]] = oColor;
        }
    }
    FastLED.show();

    if (checkForWin(who, game)) { // win
        winner = true;
    } else if (markers == 9) { // tie
      delay(2000);
      for (int i = 8; i < 136; i++) {
        leds[i] = tieColor;
      }
      FastLED.show();
      delay(3000);
      reset();
      bluecursor();
    } else { // keep playing
        who = opposite(who);
    }

    // switch cursor to available spot
    for(int i = 0; i < 3; i++) {
      for(int j = 0; j < 3; j++) {
        if(game[i][j] == PIECE_EMPTY) {
          x_pos = i;
          y_pos = j; 
          return;
        }
      }
    }
}
void findBestMove() {
  int bestX = -1;
  int bestY = -1;
  if (who == PIECE_X) {
    int bestMove = -2;
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (game[i][j] == PIECE_EMPTY){
          int tmp2[3][3];
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
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (game[i][j] == PIECE_EMPTY){
          int tmp2[3][3];
          memcpy(tmp2, game, 18);
          tmp2[i][j] = who;
          int value = minimax(tmp2, markers+1, opposite(who), true);
//            // DEBUGGING
//            Serial.write("Value: ");
//            Serial.println(value);
//            Serial.write("Coord: ");
//            Serial.print(i);
//            Serial.write(" ");
//            Serial.println(j);
//            Serial.write("================");
//            Serial.println();
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
}


int minimax(int tmp[3][3], int mark, char player, bool optimizer) {
  if (checkForWin(opposite(player), tmp)) {
    if (player == PIECE_X) {
      return -1;
    } else {
      return 1;
    }
  } else if (mark == 9) {
    return 0;
  }
  if (optimizer) {
    int bestVal = -2;
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (tmp[i][j] == PIECE_EMPTY) {
          int tmp2[3][3];
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
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (tmp[i][j] == PIECE_EMPTY) {
          int tmp2[3][3];
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

bool checkLine(int line[], char player) {
// Check if line is completed by one player
    for(int i = 0; i < 3; i++) {
        if (line[i] != player) {
            return false;
        }
    }
    return true;
}

bool checkForWin(char player, int tmp[3][3]) {
// Check if there is a win for player.

    // check row
    int row1[3] = {tmp[0][0], tmp[0][1], tmp[0][2]};
    int row2[3] = {tmp[1][0], tmp[1][1], tmp[1][2]};
    int row3[3] = {tmp[2][0], tmp[2][1], tmp[2][2]};
    if (checkLine(row1, player) || checkLine(row2, player) || checkLine(row3, player)) {
        return true;
    }
    
    int col1[3] = {tmp[0][0], tmp[1][0], tmp[2][0]};
    int col2[3] = {tmp[0][1], tmp[1][1], tmp[2][1]};
    int col3[3] = {tmp[0][2], tmp[1][2], tmp[2][2]};
    if (checkLine(col1, player) || checkLine(col2, player) || checkLine(col3, player)) {
        return true;
    }

    // check diagonal
    int dia_lr[3] = {tmp[0][0], tmp[1][1], tmp[2][2]};
    int dia_rl[3] = {tmp[2][0], tmp[1][1], tmp[0][2]};
    if (checkLine(dia_lr, player) || checkLine(dia_rl, player)) {
        return true;
    }
    return false;
}


void winnerblink() {
  //modify to blink only winning squares later
  if (winner) {
    for (int i = 8; i < 72; i++) {
      if (who == PIECE_X) {
        leds[i] = xColor;
      }
      else if (who == PIECE_O) {
        leds[i] = oColor;
      }
    }
    for (int i = 72; i < 136; i++) {
      if (who == PIECE_X) {
        leds[i] = xColor;
      }
      else if (who == PIECE_O) {
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

void bluecursor() {
    //findBestMove();
    //delay(1000);
    bool placed = false;
    while (!placed) {
      blueblink();
      if (digitalRead(UP_PIN) == LOW) {
          long unsigned int currTime = millis();
          if (currTime - prevTime > 10) {
              int prev_x = x_pos;
              x_pos = min(x_pos+1, 2);
              while(game[x_pos][y_pos] != PIECE_EMPTY && x_pos < 2) {
                x_pos = min(x_pos+1, 2);
              }
              if (game[x_pos][y_pos] != PIECE_EMPTY) {
                bool em = true;
                for(int i = prev_x+1; i < 3 && em; i++) {
                  for(int j = 0; j < 3; j++) {
                    if (game[i][j] == PIECE_EMPTY) {
                      x_pos = i;
                      y_pos = j;
                      em = false;
                      break;
                    }
                  }
                }
              }
              if (game[x_pos][y_pos] != PIECE_EMPTY) {
                x_pos = prev_x;
              }
          }
          prevTime = currTime;
      }
      else if (digitalRead(DOWN_PIN) == LOW) {
          long unsigned int currTime = millis();
          if (currTime - prevTime > 10) {
              int prev_x = x_pos;
              x_pos = max(x_pos-1, 0);
              while(game[x_pos][y_pos] != PIECE_EMPTY && x_pos > 0) {
                x_pos = max(x_pos-1, 0);
              }
              if (game[x_pos][y_pos] != PIECE_EMPTY) {
                bool em = true;
                for(int i = prev_x-1; i >= 0 && em; i--) {
                  for(int j = 0; j < 3; j++) {
                    if (game[i][j] == PIECE_EMPTY) {
                      em = false;
                      x_pos = i;
                      y_pos = j;
                      break;
                    }
                  }
                }
              }
              if (game[x_pos][y_pos] != PIECE_EMPTY) {
                x_pos = prev_x;
              }
          }
          prevTime = currTime;
      }
      else if (digitalRead(LEFT_PIN) == LOW) {
          long unsigned int currTime = millis();
          if (currTime - prevTime > 10) {
              int prev_y = y_pos;
              y_pos = max(y_pos-1, 0);
              while(game[x_pos][y_pos] != PIECE_EMPTY && y_pos > 0) {
                y_pos = max(y_pos-1, 0);
              }
              if (game[x_pos][y_pos] != PIECE_EMPTY) {
                bool em = true;
                for(int i = 0; i < 3; i++) {
                  for(int j = prev_y-1; j >= 0 && em; j--) {
                    if (game[i][j] == PIECE_EMPTY) {
                      x_pos = i;
                      y_pos = j;
                      em = false;
                      break;
                    }
                  }
                }
              }
              if (game[x_pos][y_pos] != PIECE_EMPTY) {
                y_pos = prev_y;
              }            
          }
          prevTime = currTime;
      }
      else if (digitalRead(RIGHT_PIN) == LOW) {
          long unsigned int currTime = millis();
          if (currTime - prevTime > 10) {
              int prev_y = y_pos;
              y_pos = min(y_pos+1, 2);
              while(game[x_pos][y_pos] != PIECE_EMPTY && y_pos < 2) {
                y_pos = min(y_pos+1, 2);
              }
              if (game[x_pos][y_pos] != PIECE_EMPTY) {
                bool em = true;
                for(int i = 0; i < 3 && em; i++) {
                  for(int j = prev_y+1; j < 3; j++) {
                    if (game[i][j] == PIECE_EMPTY) {
                      em = false;
                      x_pos = i;
                      y_pos = j;
                      break;
                    }
                  }
                }
              }
              if (game[x_pos][y_pos] != PIECE_EMPTY) {
                y_pos = prev_y;
              }
          }
          prevTime = currTime;
      }
      else if (digitalRead(PLACE_PIN) == LOW) {
          long unsigned int currTime = millis();
          if (currTime - prevTime > 10) {
              placed = true;
              place();
          }
          prevTime = currTime;
      }
  }
  return;
}

void redblink() {
    if (game[x_pos][y_pos] == PIECE_EMPTY) {
      for (int i = 0; i < 4; i++) {
          leds[board2[x_pos][y_pos][i]] = gColor;
      }
      FastLED.show();
      delay(500);
      for (int i = 0; i < 4; i++) {
          leds[board2[x_pos][y_pos][i]] = eColor;
      }
      FastLED.show();
      delay(100);
    }
    return;
}

void blueblink() {
    if (game[x_pos][y_pos] == PIECE_EMPTY) {
      for (int i = 0; i < 4; i++) {
          leds[board1[x_pos][y_pos][i]] = gColor;
      }
      FastLED.show();
      delay(500);
      for (int i = 0; i < 4; i++) {
          leds[board1[x_pos][y_pos][i]] = eColor;
      }
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
          bluecursor();
      }
    }   
}

void reset() {
    x_pos = 0;
    y_pos = 0;
    markers = 0;
    winner = false;
    who = PIECE_X;
    // setup grid lines 
    int grid[56] = {10, 13, 18, 21, 24, 25, 26, 27, 28, 29, 30, 31, 34, 37, 42, 45, 48, 49, 50, 51, 52, 53, 54, 55, 58, 61, 66, 69, 130, 125, 109, 98, 82,
                     77, 133, 122, 106, 101, 90, 85, 74, 112, 113, 114, 115, 116, 117, 118, 119, 95, 94, 93, 92, 91, 89, 88};
    for (int i = 0; i < 56; i++) {
        leds[grid[i]] = wColor;
    }

    // reset led board to be empty
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                leds[board1[i][j][k]] = eColor;
                leds[board2[i][j][k]] = eColor;
            }
        }
    }
    FastLED.show();

    // reset game to be empty
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            game[i][j] = PIECE_EMPTY;
        }
    }
}
