#include <stdio.h>
#include "FastLED.h"
#include "stdio.h"
#include "StandardCplusplus.h"
#include "vector"
//#include "ArduinoSTL.h"
#include "new"

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

using namespace std;

#define NUM_LEDS    144
#define LED_PIN     13

// function declarations
void printBoard(vector<vector<int> >&);
int userMove();
void makeMove(vector<vector<int> >&, int, unsigned int);
void errorMessage(int);
int aiMove();
vector<vector<int> > copyBoard(vector<vector<int> >);
bool winningMove(vector<vector<int> >&, unsigned int);
int scoreSet(vector<unsigned int>, unsigned int);
int tabScore(vector<vector<int> >, unsigned int);
vector<int> miniMax(vector<vector<int> >&, unsigned int, int, int, unsigned int);
int heurFunction(unsigned int, unsigned int, unsigned int);
void bluecursor();
void redcursor();
void winnerblink();

unsigned int NUM_COL = 8; // how wide is the board
unsigned int NUM_ROW = 9; // how tall
unsigned int PLAYER = 1; // player number
unsigned int COMPUTER = 2; // AI number
unsigned int MAX_DEPTH = 1; // the default "difficulty" of the computer controlled AI
int INT_MAX = 10000;
int INT_MIN = -10000;

bool gameOver = false; // flag for if game is over
unsigned int turns = 0; // count for # turns
unsigned int currentPlayer = PLAYER; // current player

vector<vector<int>> board(NUM_ROW, vector<int>(NUM_COL)); // the game board


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

CRGB oColor = CRGB(255, 0, 0); // red
CRGB xColor = CRGB(0, 0, 255); // blue
CRGB wColor = CRGB(255, 255, 255); // white
CRGB eColor = CRGB(0, 0, 0); // clear/empty
CRGB gColor = CRGB(0, 255, 0); //green

/**
 * game playing function
 * loops between players while they take turns
 */
void loop() {
  //printBoard(board); // print initial board
  if (!gameOver) {
      bluecursor();
      redcursor();
    } 
} 


void bluecursor() {
  int user_move = userMove();
  makeMove(board, user_move, PLAYER);
}

  
void redcursor() {
  int ai_move = aiMove();
  makeMove(board, ai_move, COMPUTER);
}

/**
 * function that makes the move for the player
 * @param b - the board to make move on
 * @param c - column to drop piece into
 * @param p - the current player
 */
void makeMove(vector<vector<int> >& b, int c, unsigned int p) {
  // start from bottom of board going up
  for (unsigned int r = 0; r < NUM_ROW; r++) {
    if (b[r][c] == 0) { // first available spot
      b[r][c] = p; // set piece
      
      if (p == PLAYER) {
          leds[board_side_1[c][r]] = xColor;
          leds[board_side_2[c][r]] = xColor;
          b[r][c] = 1;
          currentPlayer = 2;
      } else { //player == COMPUTER
          leds[board_side_1[c][r]] = oColor;
          leds[board_side_2[c][r]] = oColor;
          b[r][c] = 2;
          currentPlayer = 1;
      }
      turns++; // iterate number of turns
      FastLED.show();
      
      gameOver = winningMove(board, currentPlayer); // check if player won  
      break;
     }
  }
}

/**
 * prompts the user for their move
 * and ensures valid user input
 * @return the user chosen column
 */
int userMove() {
  int x_pos = 0; // temporary
  bool placed = false;
  while (!placed) {
    blueblink(x_pos);
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
            placed = true;
            break;
        }
    }
  }
  return x_pos;
}

void blueblink(int x_pos) {
    if (board[x_pos][7] == 0) {
      leds[board_side_1[x_pos][7]] = gColor;
      FastLED.show();
      delay(500);
      leds[board_side_1[x_pos][7]] = eColor;
      FastLED.show();
      delay(100);
    }
    return;
}

/**
 * AI "think" algorithm
 * uses minimax to find ideal move
 * @return - the column number for best move
 */
int aiMove() {
  return miniMax(board, MAX_DEPTH, 0 - INT_MAX, INT_MAX, COMPUTER)[1];
}

/**
 * Minimax implementation using alpha-beta pruning
 * @param b - the board to perform MM on
 * @param d - the current depth
 * @param alf - alpha
 * @param bet - beta
 * @param p - current player
 */
vector<int> miniMax(vector<vector<int> > &b, unsigned int d, int alf, int bet, unsigned int p) {
  /**
   * if we've reached minimal depth allowed by the program
   * we need to stop, so force it to return current values
   * since a move will never (theoretically) get this deep,
   * the column doesn't matter (-1) but we're more interested
   * in the score
   *
   * as well, we need to take into consideration how many moves
   * ie when the board is full
   */
  if (d == 0 || d >= (NUM_COL * NUM_ROW) - turns) {
    // get current score to return
    return vector<int>{tabScore(b, COMPUTER), -1};
  }
  if (p == COMPUTER) { // if AI player
    vector<int> moveSoFar = {INT_MIN, -1}; // since maximizing, we want lowest possible value
    if (winningMove(b, PLAYER)) { // if player about to win
      return moveSoFar; // force it to say it's worst possible score, so it knows to avoid move
    } // otherwise, business as usual
    for (unsigned int c = 0; c < NUM_COL; c++) { // for each possible move
      if (b[NUM_ROW - 1][c] == 0) { // but only if that column is non-full
        vector<vector<int> > newBoard = copyBoard(b); // make a copy of the board
        makeMove(newBoard, c, p); // try the move
        int score = miniMax(newBoard, d - 1, alf, bet, PLAYER)[0]; // find move based on that new board state
        if (score > moveSoFar[0]) { // if better score, replace it, and consider that best move (for now)
          moveSoFar = {score, (int)c};
        }
        alf = max(alf, moveSoFar[0]);
        if (alf >= bet) { break; } // for pruning
      }
    }
    return moveSoFar; // return best possible move
  }
  else {
    vector<int> moveSoFar = {INT_MAX, -1}; // since PLAYER is minimized, we want moves that diminish this score
    if (winningMove(b, COMPUTER)) {
      return moveSoFar; // if about to win, report that move as best
    }
    for (unsigned int c = 0; c < NUM_COL; c++) {
      if (b[NUM_ROW - 1][c] == 0) {
        vector<vector<int> > newBoard = copyBoard(b);
        makeMove(newBoard, c, p);
        int score = miniMax(newBoard, d - 1, alf, bet, COMPUTER)[0];
        if (score < moveSoFar[0]) {
          moveSoFar = {score, (int)c};
        }
        bet = min(bet, moveSoFar[0]);
        if (alf >= bet) { break; }
      }
    }
    return moveSoFar;
  }
}

/**
 * function to tabulate current board "value"
 * @param b - the board to evaluate
 * @param p - the player to check score of
 * @return - the board score
 */
int tabScore(vector<vector<int> > b, unsigned int p) {
  int score = 0;
  vector<unsigned int> rs(NUM_COL);
  vector<unsigned int> cs(NUM_ROW);
  vector<unsigned int> set(4);
  /**
   * horizontal checks, we're looking for sequences of 4
   * containing any combination of AI, PLAYER, and empty pieces
   */
  for (unsigned int r = 0; r < NUM_ROW; r++) {
    for (unsigned int c = 0; c < NUM_COL; c++) {
      rs[c] = b[r][c]; // this is a distinct row alone
    }
    for (unsigned int c = 0; c < NUM_COL - 3; c++) {
      for (int i = 0; i < 4; i++) {
        set[i] = rs[c + i]; // for each possible "set" of 4 spots from that row
      }
      score += scoreSet(set, p); // find score
    }
  }
  // vertical
  for (unsigned int c = 0; c < NUM_COL; c++) {
    for (unsigned int r = 0; r < NUM_ROW; r++) {
      cs[r] = b[r][c];
    }
    for (unsigned int r = 0; r < NUM_ROW - 3; r++) {
      for (int i = 0; i < 4; i++) {
        set[i] = cs[r + i];
      }
      score += scoreSet(set, p);
    }
  }
  // diagonals
  for (unsigned int r = 0; r < NUM_ROW - 3; r++) {
    for (unsigned int c = 0; c < NUM_COL; c++) {
      rs[c] = b[r][c];
    }
    for (unsigned int c = 0; c < NUM_COL - 3; c++) {
      for (int i = 0; i < 4; i++) {
        set[i] = b[r + i][c + i];
      }
      score += scoreSet(set, p);
    }
  }
  for (unsigned int r = 0; r < NUM_ROW - 3; r++) {
    for (unsigned int c = 0; c < NUM_COL; c++) {
      rs[c] = b[r][c];
    }
    for (unsigned int c = 0; c < NUM_COL - 3; c++) {
      for (int i = 0; i < 4; i++) {
        set[i] = b[r + 3 - i][c + i];
      }
      score += scoreSet(set, p);
    }
  }
  return score;
}

/**
 * function to find the score of a set of 4 spots
 * @param v - the row/column to check
 * @param p - the player to check against
 * @return - the score of the row/column
 */
int scoreSet(vector<unsigned int> v, unsigned int p) {
  unsigned int good = 0; // points in favor of p
  unsigned int bad = 0; // points against p
  unsigned int empty = 0; // neutral spots
  for (unsigned int i = 0; i < v.size(); i++) { // just enumerate how many of each
    good += (v[i] == p) ? 1 : 0;
    bad += (v[i] == PLAYER || v[i] == COMPUTER) ? 1 : 0;
    empty += (v[i] == 0) ? 1 : 0;
  }
  // bad was calculated as (bad + good), so remove good
  bad -= good;
  return heurFunction(good, bad, empty);
}

/**
 * my """heuristic function""" is pretty bad, but it seems to work
 * it scores 2s in a row and 3s in a row
 * @param g - good points
 * @param b - bad points
 * @param z - empty spots
 * @return - the score as tabulated
 */
int heurFunction(unsigned int g, unsigned int b, unsigned int z) {
  int score = 0;
  if (g == 4) { score += 5001; } // preference to go for winning move vs. block
  else if (g == 3 && z == 1) { score += 50; }
  else if (g == 2 && z == 2) { score += 5; }
  else if (b == 2 && z == 2) { score -= 5; } // preference to block
  else if (b == 3 && z == 1) { score -= 51; } // preference to block
  else if (b == 4) { score -= 5000; }
  return score;
}

/**
 * function to determine if a winning move is made
 * @param b - the board to check
 * @param p - the player to check against
 * @return - whether that player can have a winning move
 */
bool winningMove(vector<vector<int> > &b, unsigned int p) {
  unsigned int winSequence = 0; // to count adjacent pieces
  // for horizontal checks
  for (unsigned int c = 0; c < NUM_COL - 3; c++) { // for each column
    for (unsigned int r = 0; r < NUM_ROW; r++) { // each row
      for (int i = 0; i < 4; i++) { // recall you need 4 to win
        if ((unsigned int)b[r][c + i] == p) { // if not all pieces match
          winSequence++; // add sequence count
        }
        if (winSequence == 4) { return true; } // if 4 in row
      }
      winSequence = 0; // reset counter
    }
  }
  // vertical checks
  for (unsigned int c = 0; c < NUM_COL; c++) {
    for (unsigned int r = 0; r < NUM_ROW - 3; r++) {
      for (int i = 0; i < 4; i++) {
        if ((unsigned int)b[r + i][c] == p) {
          winSequence++;
        }
        if (winSequence == 4) { return true; }
      }
      winSequence = 0;
    }
  }
  // the below two are diagonal checks
  for (unsigned int c = 0; c < NUM_COL - 3; c++) {
    for (unsigned int r = 3; r < NUM_ROW; r++) {
      for (int i = 0; i < 4; i++) {
        if ((unsigned int)b[r - i][c + i] == p) {
          winSequence++;
        }
        if (winSequence == 4) { return true; }
      }
      winSequence = 0;
    }
  }
  for (unsigned int c = 0; c < NUM_COL - 3; c++) {
    for (unsigned int r = 0; r < NUM_ROW - 3; r++) {
      for (int i = 0; i < 4; i++) {
        if ((unsigned int)b[r + i][c + i] == p) {
          winSequence++;
        }
        if (winSequence == 4) { return true; }
      }
      winSequence = 0;
    }
  }
  return false; // otherwise no winning move
}

/**
 * sets up the board to be filled with empty spaces
 * also used to reset the board to this state
 */
void setup() {
  Serial.begin(9600);
  gameOver = false;
  turns = 0; // count for # turns
  currentPlayer = PLAYER;

  // setup buttons
  pinMode(LEFT1, INPUT_PULLUP);
  pinMode(RIGHT1, INPUT_PULLUP);
  pinMode(LEFT2, INPUT_PULLUP);
  pinMode(RIGHT2, INPUT_PULLUP);
  pinMode(PLACE1, INPUT_PULLUP);
  pinMode(PLACE2, INPUT_PULLUP);

  // setup ws2812b leds
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  //reset game to be empty
  for (unsigned int r = 0; r < NUM_ROW; r++) {
    for (unsigned int c = 0; c < NUM_COL; c++) {
      board[r][c] = 0; // make sure board is empty initially
    }
  }

  //reset led board to be empty
  for (unsigned int r = 0; r < NUM_ROW; r++) {
    for (unsigned int c = 0; c < NUM_COL; c++) {
      leds[board_side_1[c][r]] = eColor;
      leds[board_side_2[c][r]] = eColor;
    }
  }
}

/**
 * function to copy board state to another 2D vector
 * ie. make a duplicate board; used for mutating copies rather
 * than the original
 * @param b - the board to copy
 * @return - said copy
 */
vector<vector<int> > copyBoard(vector<vector<int> > b) {
  vector<vector<int>> newBoard(NUM_ROW, vector<int>(NUM_COL));
  for (unsigned int r = 0; r < NUM_ROW; r++) {
    for (unsigned int c = 0; c < NUM_COL; c++) {
      newBoard[r][c] = b[r][c]; // just straight copy
    }
  }
  return newBoard;
}

/**
 * prints the board to console out
 * @param b - the board to print
 */
void printBoard(vector<vector<int> > &b) {
  for (unsigned int i = 0; i < NUM_COL; i++) {
  }
  for (unsigned int r = 0; r < NUM_ROW; r++) {
    for (unsigned int c = 0; c < NUM_COL; c++) {
      switch (b[NUM_ROW - r - 1][c]) {
      case 0: 
        break; // no piece
      case 1: 
        break; // one player's piece
      case 2: 
        break; // other player's piece
      }
      if (c + 1 == NUM_COL) { 
      }
    }
  }
}


void reset() { //reset board
  gameOver = false;
  turns = 0; // count for # turns
  currentPlayer = PLAYER;

  //reset game to be empty
  for (unsigned int r = 0; r < NUM_ROW; r++) {
    for (unsigned int c = 0; c < NUM_COL; c++) {
      board[r][c] = 0; // make sure board is empty initially
    }
  }

  //reset led board to be empty
  for (unsigned int r = 0; r < NUM_ROW; r++) {
    for (unsigned int c = 0; c < NUM_COL; c++) {
      leds[board_side_1[c][r]] = eColor;
      leds[board_side_2[c][r]] = eColor;
    }
  }
}
