#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <Adafruit_GFX.h>
#include <ST7558.h>
#include <Wire.h>
#include <SPI.h>
#include <logo.h>

#define DEBUG 0
#define ENABLE_FPS 0

/* set pins

  Display connections:
    VCC -> +3.3v arduino pin
    GND -> +0v arduino pin 
    SCL -> A5 arduino pin
    SDA -> A4 arduino pin
    RST -> A3 arduino pin

  Joystick connections:
    VCC -> +5v arduino pin
    GND -> +0v arduino pin
    URX -> A2 arduino pin
    URY -> A1 arduino pin
    SW -> A0 arduino pin via 10k pullup resistor

    speaker -> 4 arduino pin

*/
#define RST_PIN A3
#define STICK_X A2
#define STICK_Y A1
#define SW A0
#define SPK 4


// lcd resolution
#define width 96
#define height 65

// gamefield
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
  logic(),
  no_tone(uint8_t duration);
uint8_t 
  menu_switcher (uint8_t menu_item, uint8_t n_items);

// vars
int8_t 
  sn_x,                     // snake horizontally position
  sn_y,                     // snake vertical position
  food_x,                   // food horizontally position
  food_y;                   // food vertical position           
uint8_t
  n_eat,                    // the number of eaten food
  prev_dir,                 // previos direction of snake. need for correct game experience
  main_menu_item,
  gameover_menu_item,
  options_menu_item,
  contrast;                 // lcd contrast parameter
uint32_t
  prev_millis;
bool
  game_over,
  options_state,
  menu_state,
  borders,
  flicker,
  invert,
  spk_state;

#if(ENABLE_FPS)
uint32_t fps_millis;
uint8_t fps;
#endif

// arrays for snake's tail
int8_t 
  tailX[50],
  tailY[50];

// automatic eeprom address distributor
uint8_t EEMEM
  contrast_addr,
  border_state_addr,
  invert_addr;

enum
{ 
  STAY, 
  LEFT, 
  RIGHT, 
  UP, 
  DOWN 
} dir;

ST7558 lcd = ST7558(RST_PIN);

void setup() 
{
#if(DEBUG)
  Serial.begin(9600); // for debug
#endif

  randomSeed(analogRead(A7)); // rand init

  pinMode( SW, INPUT ); // for switch on a joystick

  // lcd init
  Wire.begin();
  lcd.init();
  lcd.setRotation(0);
  lcd.setContrast(eeprom_read_byte(&contrast_addr));
  lcd.clearDisplay();

  // game init
  default_set();
  borders = eeprom_read_byte(&border_state_addr);
  invert = eeprom_read_byte(&invert_addr);
  menu_state = true;
  prev_millis = 0;
  flicker = true,
  spk_state = false;
}

// main loop
void loop() 
{
  lcd.invertDisplay(invert);

  if (menu_state) 
    draw_menu();
  else if (options_state)
    draw_options();
  else 
  {
    if (!game_over)
      draw_game();   
    else
      draw_gameover_menu();
  }

  lcd.display();
  lcd.clearDisplay();
}

// set default game options
void default_set() 
{
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

void draw_menu() 
{
  dir = STAY;

  lcd.drawBitmap( random(14, 16), random(2, 4), snake_logo, snake_logo_w, snake_logo_h, BLACK );
  lcd.drawRoundRect( (width/2) - 16, 35, 33, 11, 2, BLACK );
  lcd.setCursor( (width/2) - 14, 37 );
  lcd.print(F("start"));
  lcd.drawRoundRect( (width/2) - 22, 49, 45, 12, 2, BLACK );
  lcd.setCursor( (width/2) - 20, 51 );
  lcd.print(F("options"));

  const static uint8_t n_items = 2;
  main_menu_item = menu_switcher( main_menu_item, n_items );
  if (main_menu_item == 1)
  {
    (flicker) ? 
    lcd.drawRoundRect( (width/2) - 17, 34, 35, 13, 2, BLACK ) 
    : 
    lcd.drawRoundRect( (width/2) - 17, 34, 35, 13, 2, WHITE );
  }
  else if (main_menu_item == 2)
  {
    (flicker) ? 
    lcd.drawRoundRect( (width/2) - 23, 48, 47, 14, 2, BLACK ) 
    : 
    lcd.drawRoundRect( (width/2) - 23, 48, 47, 14, 2, WHITE );
  }
  flicker = !flicker;

  if (!digitalRead(SW)) 
  {
    if (main_menu_item == 1) 
    {
      default_set();
    }
    else 
    {
      options_state = true;
      options_menu_item = 1;
      contrast = eeprom_read_byte(&contrast_addr);
      borders = eeprom_read_byte(&border_state_addr);
      invert = eeprom_read_byte(&invert_addr);
    }
    menu_state = false;
    on_click();
  }
}

void draw_options() 
{
  dir = STAY;

  lcd.setCursor(8, 1);
  lcd.print(F("contrast:"));
  lcd.print(contrast);
  lcd.setCursor(8, 9);
  lcd.print(F("borders:"));
  (borders) ? lcd.print(F("true")) : lcd.print(F("false"));
  lcd.setCursor(8, 17);
  lcd.print(F("theme:"));
  (invert) ? lcd.print(F("dark")) : lcd.print(F("light"));
  lcd.setCursor(8, 25);
  lcd.print(F("exit"));

    
  const static uint8_t n_items = 4;
  options_menu_item = menu_switcher(options_menu_item, n_items);
  
  if (options_menu_item == 1) 
  {
    lcd.setCursor(0,1);
    lcd.print(F(">"));

    get_dir();
    switch (dir) 
    {
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
  else if (options_menu_item == 2) 
  {
    lcd.setCursor(0, 9);
    lcd.print(F(">"));

    if (!digitalRead(SW)) 
    {
      borders = !borders;
      on_click();
    }
  }
  else if (options_menu_item == 3) 
  {
    lcd.setCursor(0, 17);
    lcd.print(F(">"));
    
    if (!digitalRead(SW)) 
    {
      invert = !invert;
      on_click();
    }
  }
  else 
  {
    lcd.setCursor(0, 25);
    lcd.print(F(">"));

    if (!digitalRead(SW)) 
    {
      menu_state = true;
      options_state = false;
      eeprom_write_byte(&contrast_addr, contrast);
      eeprom_write_byte(&border_state_addr, borders);
      eeprom_write_byte(&invert_addr, invert);
      on_click();
    }
  }

}

void draw_game() 
{
  (invert) ? lcd.drawLine(0, 9, 95, 9, BLACK) : lcd.drawRect( 0, 9, field_x, field_y, BLACK );
  lcd.setCursor( 1, 1 );
  lcd.print(F("Score:"));
  lcd.print(n_eat);

#if(ENABLE_FPS)
  fps = 1000/(millis()-fps_millis);
  lcd.setCursor(72,1);
  lcd.setTextColor(BLACK);
  lcd.print(fps);
  lcd.print("fps");
  fps_millis = millis();
#endif

  logic();

  // draw snake's head
  lcd.fillRect( sn_x, sn_y, a, a, BLACK );
  // draw snake's tail 
  for (uint8_t i = 0; i < n_eat; i++)
    lcd.drawRect( tailX[i], tailY[i], a, a, BLACK );
  // draw food
  lcd.drawRect( food_x, food_y, a, a, BLACK );
}

void draw_gameover_menu()
{
  dir = STAY;

  lcd.drawBitmap( random(-1, 1), random(-1,1), gameover_logo, gameover_logo_w, gameover_logo_h, BLACK );
  lcd.drawRoundRect( (width/2) - 22, 35, 45, 11, 2, BLACK );
  lcd.setCursor( (width/2) - 20, 37 );
  lcd.print(F("restart"));

  lcd.drawRoundRect( (width/2) - 13, 49, 27, 12, 2, BLACK );
  lcd.setCursor( (width/2) - 11, 51 );
  lcd.print(F("menu"));

  const static uint8_t n_items = 2;
  gameover_menu_item = menu_switcher(gameover_menu_item, n_items);
  
  if (gameover_menu_item == 1) 
  {
    (flicker) ?
    lcd.drawRoundRect( (width/2) - 23, 34, 47, 13, 2, BLACK ) 
    :
    lcd.drawRoundRect( (width/2) - 23, 34, 47, 13, 2, WHITE );
  }
  else if (gameover_menu_item == 2) 
  {
    (flicker) ? 
    lcd.drawRoundRect( (width/2) - 14, 48, 29, 14, 2, BLACK ) 
    : 
    lcd.drawRoundRect( (width/2) - 14, 48, 29, 14, 2, WHITE );
  }
  flicker = !flicker;

  if (!digitalRead(SW)) 
  {
    on_click();
    if (gameover_menu_item == 1)
      default_set();
    else
      menu_state = true;
  }
}

uint8_t menu_switcher (uint8_t menu_item, uint8_t n_items)
{
  get_dir();
  uint8_t cur_item = menu_item;
  if (millis() - prev_millis >= 250 && dir != 0)
  {
    prev_millis = millis();
    switch (dir) 
    {
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
  }
  if (cur_item != menu_item)
  {
    tone(SPK, 80); // 80 Hz tone frequency
    spk_state = true;
  }
  no_tone(100);

  return menu_item;
}

void on_click() 
{
  while (!digitalRead(SW));
}

void no_tone(uint8_t duration) 
{
  if (millis() - prev_millis >= duration && spk_state) 
  {
    prev_millis = 0;
    noTone(SPK);
    spk_state = false;
  }
}

void get_dir() 
{
  int stick_x = analogRead(STICK_X);
  int stick_y = analogRead(STICK_Y);

  if (stick_x > 700)
    dir = RIGHT;
  else if (stick_x < 100)
    dir = LEFT;
  if (stick_y > 700)
    dir = UP;
  else if (stick_y < 100)
    dir = DOWN;
}
  
void logic() 
{
  int prevX = tailX[0];
  int prevY = tailY[0];
  int prev2X, prev2Y;
  tailX[0] = sn_x;
  tailY[0] = sn_y;

  for (int i = 1; i <= n_eat; i++)
  {
    prev2X = tailX[i];
    prev2Y = tailY[i];
    tailX[i] = prevX;
    tailY[i] = prevY;
    prevX = prev2X;
    prevY = prev2Y;
  }

  get_dir();
  switch (prev_dir)
  {
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

  switch (dir) 
  {
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
    switch (sn_y) 
    {
    case 64:
      sn_y = 10;
      break;
    case 7: 
      sn_y = 64;
      break;
    }
    switch (sn_x) 
    {
    case -2:
      sn_x = 91;
      break;
    case 94:
      sn_x = 1;
      break;
    }
  }
  else
    // if u bump into borders
    if ( sn_y == 7 || sn_y == 64 || sn_x == -2 || sn_x == 94 )
      game_over = true;
      
  if ( sn_x == food_x && sn_y == food_y ) 
  {
    // calculate food position 
    food_x = random(0, 30) * 3 + 1;
    food_y = random(3, 17) * 3 + 1;
    n_eat++;
    tone(SPK, 150); // 150 Hz tone frequency
    spk_state = true;
    prev_millis = millis();
  }
  no_tone(100); // 100ms tone duration

  // check self-eating
  for (int k = 0; k < n_eat; k++)
    if ( tailX[k] == sn_x && tailY[k] == sn_y )
      game_over = true;
}