#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <Adafruit_GFX.h>
#include <ST7558.h>
#include <Wire.h>
#include <SPI.h>
#include <logo.h>

#define DEBUG 0             // 1 - enable, 0 - disable
#define ENABLE_FPS 1

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
    no_tone(uint8_t);

uint8_t menu_switcher(uint8_t, uint8_t);

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

uint32_t prev_millis;

struct {
    unsigned 
        gameover : 1,
        options : 1, 
        menu : 1,
        borders : 1,
        flicker : 1,
        inverter : 1,
        spk : 1;
} state;


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
    borders_state_addr,
    inverter_addr;

enum { 
    STAY,
    LEFT,
    RIGHT,
    UP,
    DOWN
} dir;

ST7558 lcd = ST7558(RST_PIN);

void setup() {

#if(DEBUG)
    Serial.begin(9600); // for debug
#endif

    randomSeed(analogRead(A7)); // rand init

    pinMode(SW, INPUT); // for switch on a joystick

    // lcd init
    Wire.begin();
    lcd.init();
    lcd.setRotation(0);
    lcd.setContrast(eeprom_read_byte(&contrast_addr));
    lcd.clearDisplay();

    // game init
    default_set();
    state.borders = eeprom_read_byte(&borders_state_addr);
    state.inverter = eeprom_read_byte(&inverter_addr);
    state.menu = true;
    prev_millis = 0;
    state.flicker = true;
    state.spk = false;
}

// main loop
void loop() {
    
    lcd.invertDisplay(state.inverter);

    if (state.menu)
        draw_menu();
    else if (state.options)
        draw_options();
    else {

        if (!state.gameover)
            draw_game();
        else
            draw_gameover_menu();
    }

#if(DEBUG)
    Serial.println("Snake: [" + String(sn_x) + ',' + String(sn_y) + ']');
#endif

    lcd.display();
    lcd.clearDisplay();
}

// set default game options
void default_set() {
    
    state.gameover = false;
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

    lcd.drawBitmap(random(14, 16), random(2, 4), snake_logo, snake_logo_w, snake_logo_h, BLACK);
    lcd.drawRoundRect((width / 2) - 16, 35, 33, 11, 2, BLACK);
    lcd.setCursor((width / 2) - 14, 37);
    lcd.print(F("start"));
    lcd.drawRoundRect((width / 2) - 22, 49, 45, 12, 2, BLACK);
    lcd.setCursor((width / 2) - 20, 51);
    lcd.print(F("options"));

    const static uint8_t n_items = 2;
    main_menu_item = menu_switcher(main_menu_item, n_items);
    if (main_menu_item == 1)
        lcd.drawRoundRect((width / 2) - 17, 34, 35, 13, 2, state.flicker ? BLACK : WHITE);
    else if (main_menu_item == 2)
        lcd.drawRoundRect((width / 2) - 23, 48, 47, 14, 2, state.flicker ? BLACK : WHITE);
    state.flicker = !state.flicker;

    if (!digitalRead(SW)) {
        if (main_menu_item == 1)
            default_set(); 
        else {

            state.options = true;
            options_menu_item = 1;
            contrast = eeprom_read_byte(&contrast_addr);
            state.borders = eeprom_read_byte(&borders_state_addr);
            state.inverter = eeprom_read_byte(&inverter_addr);
        }
        state.menu = false;
        on_click();
    }
}

void draw_options() {
    
    dir = STAY;

    lcd.setCursor(8, 1);
    lcd.print(F("contrast:"));
    lcd.print(contrast);
    lcd.setCursor(8, 9);
    lcd.print(F("state.borders:"));
    lcd.print((state.borders ? F("true") : F("false")));
    lcd.setCursor(8, 17);
    lcd.print(F("theme:"));
    lcd.print((state.inverter ? F("dark") : F("light")));
    lcd.setCursor(8, 25);
    lcd.print(F("exit"));


    const static uint8_t n_items = 4;
    options_menu_item = menu_switcher(options_menu_item, n_items);

    if (options_menu_item == 1) {

        lcd.setCursor(0, 1);
        lcd.print(F(">"));

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
    } else if (options_menu_item == 2) {

        lcd.setCursor(0, 9);
        lcd.print(F(">"));

        if (!digitalRead(SW)) {

            state.borders = !state.borders;
            on_click();
        }
    } else if (options_menu_item == 3) {

        lcd.setCursor(0, 17);
        lcd.print(F(">"));

        if (!digitalRead(SW)) {

            state.inverter = !state.inverter;
            on_click();
        }
    } else {

        lcd.setCursor(0, 25);
        lcd.print(F(">"));

        if (!digitalRead(SW)) {

            state.menu = true;
            state.options = false;
            eeprom_write_byte(&contrast_addr, contrast);
            eeprom_write_byte(&borders_state_addr, state.borders);
            eeprom_write_byte(&inverter_addr, state.inverter);
            on_click();
        }
    }
}

void draw_game() {

    (state.inverter) ? lcd.drawLine(0, 9, 95, 9, BLACK) : lcd.drawRect(0, 9, field_x, field_y, BLACK);
    lcd.setCursor(1, 1);
    lcd.print(F("Score:"));
    lcd.print(n_eat);

#if(ENABLE_FPS)
    fps = 1000/(millis()-fps_millis);
    lcd.setCursor(72,1);
    lcd.setTextColor(BLACK);
#if(DEBUG)
    Serial.println(fps);
#endif
    lcd.print(fps);
    lcd.print("fps");
    fps_millis = millis();
#endif

    logic();

    // draw snake's head
    lcd.fillRect(sn_x, sn_y, a, a, BLACK);
    // draw snake's tail
    for (uint8_t i = 0; i < n_eat; i++)
        lcd.drawRect(tailX[i], tailY[i], a, a, BLACK);
    // draw food
    lcd.drawRect(food_x, food_y, a, a, BLACK);
}

void draw_gameover_menu() {
    
    dir = STAY;

    lcd.drawBitmap(random(-1, 1), random(-1, 1), gameover_logo, gameover_logo_w, gameover_logo_h, BLACK);
    lcd.drawRoundRect((width / 2) - 22, 35, 45, 11, 2, BLACK);
    lcd.setCursor((width / 2) - 20, 37);
    lcd.print(F("restart"));

    lcd.drawRoundRect((width / 2) - 13, 49, 27, 12, 2, BLACK);
    lcd.setCursor((width / 2) - 11, 51);
    lcd.print(F("menu"));

    const static uint8_t n_items = 2;
    gameover_menu_item = menu_switcher(gameover_menu_item, n_items);

    if (gameover_menu_item == 1)
        lcd.drawRoundRect((width / 2) - 23, 34, 47, 13, 2, state.flicker ? BLACK : WHITE); 
    else if (gameover_menu_item == 2)
        lcd.drawRoundRect((width / 2) - 14, 48, 29, 14, 2, state.flicker ? BLACK : WHITE);
    state.flicker = !state.flicker;

    if (!digitalRead(SW)) {

        on_click();
        if (gameover_menu_item == 1)
            default_set();
        else
            state.menu = true;
    }
}

uint8_t menu_switcher(uint8_t menu_item, uint8_t n_items)
{
    get_dir();
    uint8_t cur_item = menu_item;
    if (millis() - prev_millis >= 250 && dir != 0) {

        prev_millis = millis();
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
    }
    if (cur_item != menu_item) {

        tone(SPK, 80); // 80 Hz tone frequency
        state.spk = true;
    }
    no_tone(100);

    return menu_item;
}

void on_click() {
    while (!digitalRead(SW));
}

void no_tone(uint8_t duration) {

    if (millis() - prev_millis >= duration && state.spk) {

        prev_millis = 0;
        noTone(SPK);
        state.spk = false;
    }
}

void get_dir() {

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

void logic() {

    for (int i = n_eat; i > 0; i--) {

        tailX[i] = tailX[i - 1];
        tailY[i] = tailY[i - 1];
    }
    tailX[0] = sn_x;
    tailY[0] = sn_y;

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

    if (!state.borders) {

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
        // if u bump into state.borders
        if (sn_y == 7 || sn_y == 64 || sn_x == -2 || sn_x == 94)
            state.gameover = true;

    if (sn_x == food_x && sn_y == food_y) {
        
        // calculate food position 
        food_x = random(0, 30) * 3 + 1;
        food_y = random(3, 17) * 3 + 1;
        n_eat++;
        tone(SPK, 150); // 150 Hz tone frequency
        state.spk = true;
        prev_millis = millis();
    }
    no_tone(100); // 100ms tone duration

    // check self-eating
    for (int k = 0; k < n_eat; k++)
        if (tailX[k] == sn_x && tailY[k] == sn_y)
            state.gameover = true;
}
