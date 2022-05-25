#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>


/* Features
 * Connect  to the wifi
 * Settings in the Android App
 * Start and Stop in both App and keypad
 * Long press to go to menu
 * In normal mode, press the keypad to start
 * When started is press the keypad to stop
 * Limit switch at the door to stop if the door is open
 * When door is open, should not be able to start, but can go to menu
 * Once process is ended, x plotter should go back to home and SSR should be off
 * Every to before starting Should auto home x plotter
 * When Start is pressed, should wait until set temp is reached
 * If the door is open it should say close the door and press to start
 * When the process is started timer should start and when ended should stop, and record the time
 * every thing should be reflected in the app
 */

/* Menu to be added
 * Settings 
 * - X plotter Speed
 * - Water Pump Speed
 * - Base Plate Temperature
 * Logs
 * - all run time 
 */

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 7, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup()
{ 
  Serial.begin(9600);
  Serial.println("Spray Pyrolysis System");
	// initialize the LCD
	lcd.begin();

	// Turn on the blacklight and print a message.
	lcd.backlight();
	lcd.print("Spray Pyrolysis");

  //ToDo: Add stepper motor homeing code here
}

void loop()
{
	char key = keypad.getKey();
  
  if (key){
    lcd.setCursor(0,1);
    lcd.print(key);
    Serial.println(key);
  }
}