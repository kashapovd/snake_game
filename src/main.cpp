#include <avr/pgmspace.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <ST7558.h>
#include <Wire.h>
#include <SPI.h>
#include <logo.h>

// set pin
#define RST_PIN A3
#define STICK_X A2
#define STICK_Y A1
#define SW A0

// lcd parameters
#define width 96
#define height 65

// game field
#define field_x 95
#define field_y 56
#define a 3

void draw_menu();
void draw_options();
void draw_gameover_menu();
void draw_game();
void default_set();
bool menu_switcher(bool);
void get_dir();
void logic();


int sn_x, sn_y, food_x, food_y, n_eat = 0; 
int score = 0;
bool game_over = false;
bool menu_state = true;
bool restart = true;
bool start = true;
bool options = false;
int tailX[60];
int tailY[60];
enum Direction { STAY, LEFT, RIGHT, UP, DOWN };
Direction dir;
ST7558 lcd = ST7558(RST_PIN);
byte contrast;

void setup() {
  Serial.begin(9600);
  
  // rand init
  randomSeed(analogRead(A7));

  pinMode( SW, INPUT ); // for switch on stick

  // lcd init
  Wire.begin();
  lcd.init();
  lcd.setRotation(0);
  contrast = 65;
  lcd.setContrast(contrast);

  // game init
  default_set();
  menu_state = true;
}

void loop() {

  if (menu_state) {
    draw_menu();
  }
  else {
    if (!game_over)
      draw_game();
    else
      draw_gameover_menu();
  }
  
  lcd.display();
  lcd.clearDisplay();
}

// set default game options
void default_set() {

  game_over = false;
  score = 0;
  // start point for snake
  sn_x = 46;
  sn_y = 37;
  // start point for food
  food_x = random(0, 30) * 3 + 1;
  food_y = random(3, 17) * 3 + 1;
  n_eat = 0;
  dir = STAY;

}

void draw_menu() {

  dir = STAY;

  lcd.drawBitmap( (width/2) - (logo_w/2), 3, logo_snake, logo_w, logo_h, BLACK );
  lcd.drawRoundRect( (width/2) - 16, 35, 33, 11, 2, BLACK );
  lcd.setCursor( (width/2) - 14, 37 );
  lcd.print("start");
  lcd.drawRoundRect( (width/2) - 22, 49, 45, 12, 2, BLACK );
  lcd.setCursor( (width/2) - 20, 51 );
  lcd.print("options");

  start = menu_switcher(start);
  if (start)
    lcd.drawRoundRect( (width/2) - 17, 34, 35, 13, 2, BLACK );
  else
    lcd.drawRoundRect( (width/2) - 23, 48, 47, 14, 2, BLACK );
  
  if (!digitalRead(SW)) {
    if (start) {
      menu_state = false;
      default_set();
    }
    else {
      draw_options();
    }
  }
}

void draw_options() {
  options = true;
  while (options) {

    lcd.setCursor(0,0);
    lcd.print("Set contrast:");
    lcd.print(contrast);
    lcd.display();
    lcd.clearDisplay();
    
    get_dir();
    switch (dir) {
    case LEFT:
      contrast += 5;
      dir = STAY;
      break;
    case RIGHT:
      contrast -= 5;
      dir = STAY;
    default:
      break;
    }
    
    lcd.setContrast(contrast);
    if (!digitalRead(SW) == true) {
      options = false;
    }

  }
}

void draw_game() {

  lcd.drawRect( 0, 9, field_x, field_y, BLACK );
  lcd.setCursor( 1, 1 );
  lcd.print("Score:");
  lcd.print(score);

  logic();
  lcd.fillRect( sn_x, sn_y, a, a, BLACK );
  lcd.drawRect( food_x, food_y, a, a, BLACK );

  // draw snake's tail 
  for (int i = 0; i < n_eat; i++)
    lcd.drawRoundRect( tailX[i], tailY[i], a, a, 1, BLACK );
  
}

void draw_gameover_menu () {

  dir = STAY;

  lcd.drawBitmap( 0, 0, logo_gameOver, gameOver_w, gameOver_h, BLACK );
  lcd.drawRoundRect( (width/2) - 22, 35, 45, 11, 2, BLACK );
  lcd.setCursor( (width/2) - 20, 37 );
  lcd.print("restart");

  lcd.drawRoundRect( (width/2) - 13, 49, 27, 12, 2, BLACK );
  lcd.setCursor( (width/2) - 11, 51 );
  lcd.print("menu");
    
  restart = menu_switcher(restart);
    
  if (restart)
    lcd.drawRoundRect( (width/2) - 23, 34, 47, 13, 2, BLACK );
  else
    lcd.drawRoundRect( (width/2) - 14, 48, 29, 14, 2, BLACK );

  if (!digitalRead(SW)) {
    if (restart)
      default_set();
    else {
      menu_state = true;
    } 
  }

}

bool menu_switcher (bool var_sw) {
  get_dir();
  switch (dir) {
  case DOWN:
    if (var_sw)
      var_sw = !var_sw;
    else
      var_sw = true;
    break;
  case UP:
    if (!var_sw)
      var_sw = !var_sw;
    else 
      var_sw = false;
    break;
  default:
    break;
  }
  return var_sw;
}

void get_dir() {

  int stick_x = analogRead(A2);
  int stick_y = analogRead(A1);
  
  if (stick_x > 1020) {
    dir = RIGHT;
  } 
  else if (stick_x < 10) {
    dir = LEFT;
  }
  if (stick_y > 1020) {
    dir = UP;
  }
  else if (stick_y < 10) {
    dir = DOWN;
  }
    
}
  
void logic() {

  int prevX = tailX[0];
  int prevY = tailY[0];
  int prev2X, prev2Y;
  tailX[0] = sn_x;
  tailY[0] = sn_y;

  for (int i = 1; i <= n_eat; i++) {

    prev2X = tailX[i];
    prev2Y = tailY[i];
    tailX[i] = prevX;
    tailY[i] = prevY;
    prevX = prev2X;
    prevY = prev2Y;

  }

  get_dir();
  switch (dir) {

    case UP:
      sn_y += a;
      break;
    case DOWN:
      sn_y -= a;
      break;
    case LEFT:
      sn_x -= a;
      break;
    case RIGHT:
      sn_x += a;
      break;
    case STAY:
      break;

  } 

  // check borders
  switch (sn_y) {

  case 64:
    sn_y = 10;
    break;
  case 7: 
    sn_y = 64;
    break;

  }
  switch (sn_x) {

  case -2:
    sn_x = 91;
    break;
  case 94:
    sn_x = 1;
    break;

  }

  // renew food position
  if (sn_x == food_x && sn_y == food_y) {

    food_x = random(0, 30) * 3 + 1;
    food_y = random(3, 17) * 3 + 1;
    n_eat++;
    score++;

  }

  for (int k = 0; k < n_eat; k++) {
    if (tailX[k] == sn_x && tailY[k] == sn_y)
      game_over = true;
  }

}