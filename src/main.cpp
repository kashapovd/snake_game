#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <Adafruit_GFX.h>
#include <ST7558.h>
#include <Wire.h>
#include <SPI.h>
#include <logo.h>

/* set pins

  Display connections:
    VCC -> +5v arduino pin
    GND -> +0v arduino pin 
    SCL -> A5 arduino pin
    SDA -> A4 arduino pin
    RST -> A3 arduino pin

  Joystick connections:
    VCC -> +5v arduino pin
    GND -> +0v arduino pin
    URX -> A2 arduino pin
    URY -> A1 arduino pin
    SW -> A0(digital mode) arduino pin via 10k pullup resistor

*/
#define RST_PIN A3 
#define STICK_X A2
#define STICK_Y A1
#define SW A0

// gamefield
#define width 96
#define height 65
#define field_x 95
#define field_y 56
#define a 3

// functions
void 
  draw_menu(),
  draw_options(),
  draw_gameover_menu(),
  draw_game(),
  default_set(),
  on_click(),
  get_dir(),
  logic();
int8_t 
  menu_switcher (int8_t, int8_t);

// vars
int8_t  
  sn_x,                     // snake horizontally position
  sn_y,                     // snake vertical position
  food_x,                   // food horizontally position
  food_y,                   // food vertical position
  n_eat,                    // the number of eaten food
  prev_dir,                 // previos direction of snake. need for correct game's experience
  main_menu_item = 1,       // default menu state
  gameover_menu_item = 1, 
  options_menu_item = 1, 
  contrast;
bool 
  game_over,
  options_state,
  menu_state,
  borders,
  flicker = true;

// arrays for snake's tail
int8_t 
  tailX[60],
  tailY[60];

uint8_t EEMEM contrast_addr;
uint8_t EEMEM border_state_addr;

enum Direction { STAY, LEFT, RIGHT, UP, DOWN };
Direction dir;
ST7558 lcd = ST7558(RST_PIN);

void setup() {
  
  Serial.begin(9600); // debug
  // rand init
  randomSeed(analogRead(A7));

  pinMode( SW, INPUT ); // for switch on stick

  // lcd init
  Wire.begin();
  lcd.init();
  lcd.setRotation(0);
  lcd.setContrast( eeprom_read_byte( &contrast_addr ) );

  // game init
  default_set();
  menu_state = true;

}

// main loop
void loop() {

  if (menu_state) 
    draw_menu();
  else if (options_state)
    draw_options();
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
  // start point for snake
  sn_x = 46;
  sn_y = 34;
  // start point for food
  food_x = random(0, 30) * 3 + 1;
  food_y = random(3, 17) * 3 + 1;
  n_eat = 0;
  dir = STAY;
  main_menu_item = 1;
  gameover_menu_item = 1;

}

void draw_menu() {

  dir = STAY;

  lcd.drawBitmap( random(14, 16), random(2, 4), logo_snake, snake_logo_w, snake_logo_h, BLACK );
  lcd.drawRoundRect( (width/2) - 16, 35, 33, 11, 2, BLACK );
  lcd.setCursor( (width/2) - 14, 37 );
  lcd.print("start");
  lcd.drawRoundRect( (width/2) - 22, 49, 45, 12, 2, BLACK );
  lcd.setCursor( (width/2) - 20, 51 );
  lcd.print("options");

  int8_t n_items = 2;
  main_menu_item = menu_switcher( main_menu_item, n_items );
  if (main_menu_item == 1) {

    if (flicker) {

      lcd.drawRoundRect( (width/2) - 17, 34, 35, 13, 2, BLACK );
      flicker = !flicker;

    }
    else {

      lcd.drawRoundRect( (width/2) - 17, 34, 35, 13, 2, WHITE );
      flicker = !flicker;

    }

  }
    
  else if (main_menu_item == 2) {

    if (flicker) {

      lcd.drawRoundRect( (width/2) - 23, 48, 47, 14, 2, BLACK );
      flicker = !flicker;

    }
    else {

      lcd.drawRoundRect( (width/2) - 23, 48, 47, 14, 2, WHITE );
      flicker = !flicker;
      
    }
    
  }
    
  
  if (!digitalRead(SW)) {
    if (main_menu_item == 1) {

      on_click();
      menu_state = false;
      default_set();

    }
    else {

      on_click();
      menu_state = false;
      options_state = true;
      options_menu_item = 1;
      contrast = eeprom_read_byte( &contrast_addr );
      borders = eeprom_read_byte( &border_state_addr );

    }

  }

}

void draw_options() {

  lcd.setCursor(8, 0);
  lcd.print("contrast:");
  lcd.print(contrast);
  lcd.setCursor(8, 8);
  lcd.print("borders:");
  if (borders)
    lcd.print("true");
  else 
    lcd.print("false");
  lcd.setCursor(8, 15);
  lcd.print("exit");
    
  int n_items = 3;
  options_menu_item = menu_switcher(options_menu_item, n_items);
    
  if (options_menu_item == 1) {

    lcd.setCursor(0,0);
    lcd.print('>');

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

  }
  else if (options_menu_item == 2) {

    lcd.setCursor(0, 8);
    lcd.print('>');

    if (!digitalRead(SW)) {

      on_click();
      borders = !borders;

    }

  }
  else {

    lcd.setCursor(0, 15);
    lcd.print('>');

    if (!digitalRead(SW)) {

      eeprom_update_byte(&contrast_addr, contrast);
      eeprom_update_byte(&border_state_addr, borders);

      on_click();
      menu_state = true;
      options_state = false;

    }

  }

}

void draw_game() {

  lcd.drawRect( 0, 9, field_x, field_y, BLACK );
  lcd.setCursor( 1, 1 );
  lcd.print("Score:");
  lcd.print(n_eat);

  logic();
  
  // draw snake's head
  lcd.fillRect( sn_x, sn_y, a, a, BLACK );
  // draw snake's tail 
  for (int8_t i = 0; i < n_eat; i++)
    lcd.drawRoundRect( tailX[i], tailY[i], a, a, 1, BLACK );
  // draw food
  lcd.drawRect( food_x, food_y, a, a, BLACK );
  
}

void draw_gameover_menu () {

  dir = STAY;

  lcd.drawBitmap( random(-1, 1), random(-1,1), logo_gameOver, gameover_logo_w, gameover_logo_h, BLACK );
  lcd.drawRoundRect( (width/2) - 22, 35, 45, 11, 2, BLACK );
  lcd.setCursor( (width/2) - 20, 37 );
  lcd.print("restart");

  lcd.drawRoundRect( (width/2) - 13, 49, 27, 12, 2, BLACK );
  lcd.setCursor( (width/2) - 11, 51 );
  lcd.print("menu");

  int8_t n_items = 2;
  gameover_menu_item = menu_switcher(gameover_menu_item, n_items);
  
  if (gameover_menu_item == 1) {

    if (flicker) {

      lcd.drawRoundRect( (width/2) - 23, 34, 47, 13, 2, BLACK );
      flicker = !flicker;

    }
    else {

      lcd.drawRoundRect( (width/2) - 23, 34, 47, 13, 2, WHITE );
      flicker = !flicker;

    }
    
  }
  else if (gameover_menu_item == 2) {
    
    if (flicker) {

      lcd.drawRoundRect( (width/2) - 14, 48, 29, 14, 2, BLACK );
      flicker = !flicker;

    }
    else {

      lcd.drawRoundRect( (width/2) - 14, 48, 29, 14, 2, WHITE );
      flicker = !flicker;

    }
    
  }
    

  if (!digitalRead(SW)) {

    on_click();
    if (gameover_menu_item == 1)
      default_set();
    else {
      menu_state = true;
    }

  }

}

int8_t menu_switcher (int8_t menu_item, int8_t n_items) {

  get_dir();
  switch (dir) {

  case DOWN:
    menu_item--;
    if (menu_item <= 0)
      menu_item = n_items;
    break;
  case UP:
    menu_item++;
    if (menu_item > n_items)
      menu_item = 1;
    break;
  default:
    break;

  }
  while (dir == UP || dir == DOWN) {
    if (analogRead(STICK_Y) > 400 && analogRead(STICK_Y) < 600)
      dir = STAY;
  }
  return menu_item;

}

void on_click() {

  while (!digitalRead(SW)){
      ;;
  }
  
}

void get_dir() {

  int stick_x = analogRead(A2);
  int stick_y = analogRead(A1);
  
  if (stick_x > 700) {
    dir = RIGHT;
  } 
  else if (stick_x < 100) {
    dir = LEFT;
  }
  if (stick_y > 700) {
    dir = UP;
  }
  else if (stick_y < 100) {
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
  switch (prev_dir) {

    case UP:
      if (dir == DOWN)
        dir = UP;
      break;
    case DOWN:
      if (dir == UP)
        dir = DOWN;
        break;
    case LEFT:
      if (dir == RIGHT)
      dir = LEFT;
      break;
    case RIGHT:
      if (dir == LEFT)
      dir = RIGHT;
      break;
    default:
      break;

  }

  switch (dir) {

    case UP:
      if (sn_x != -2) 
        sn_y += a;
      break;
    case DOWN:
      if (sn_x != 94)
        sn_y -= a;
      break;
    case LEFT:
      if (sn_y != 7)
        sn_x -= a;
      break;
    case RIGHT:
      if (sn_y != 64)
        sn_x += a;
      break;
    case STAY:
      break;

  } 

  prev_dir = dir;

  if (!borders) {

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
    
  }
  else
    // if bump into borders
    if ( sn_y+a == 10 || sn_y-a == 61 || sn_x+a == 1 || sn_x-a == 91 )
      game_over = true;
  
  // renew food position
  if (sn_x == food_x && sn_y == food_y) {

    food_x = random(0, 30) * 3 + 1;
    food_y = random(3, 17) * 3 + 1;
    n_eat++;

  }

  // check self-eating
  for (int k = 0; k < n_eat; k++)
    if (tailX[k] == sn_x && tailY[k] == sn_y)
      game_over = true;
      
}