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

//Keypad Variables
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

//LCD Menu Variables

bool goToMenu = false;
const int numOfScreens = 4;
int currentScreen = 0;
String screens[numOfScreens][2] = {
    {"1.Plotter Speed","m/s"}, 
    {"2.Liquid Speed", "l/m"}, 
    {"3.Bed Temp","*C"},
    {"4.Logs"," "}
  };
int parameters[numOfScreens];

//inital declarations
void inputAction(char input);
void parameterChange(int key);
void printMenuScreen();
void homeScreen();
void keypadEvent(KeypadEvent key);

//Functions
void inputAction(char input) {
  Serial.println(input);

  //screen previous
  if(input == '4') {
    if (currentScreen == 0) {
      currentScreen = numOfScreens-1;
    }else{
      currentScreen--;
    }
  }
  //screen next
  else if(input == '6') {
    if (currentScreen == numOfScreens-1) {
      currentScreen = 0;
    }else{
      currentScreen++;
    }
  }
  //value up
  else if(input == '2') {
    parameterChange(0);
  }
  //value down
  else if(input == '8') {
    parameterChange(1);
  }
}

void parameterChange(int key) {
  if(key == 0) {
    parameters[currentScreen]++;
  }else if(key == 1) {
    parameters[currentScreen]--;
  }
}

void printMenuScreen() {
  lcd.clear();
  lcd.print(screens[currentScreen][0]);
  lcd.setCursor(0,1);
  lcd.print(parameters[currentScreen]);
  lcd.print(" ");
  lcd.print(screens[currentScreen][1]);
}

void homeScreen() {
  lcd.clear();
  lcd.print("  Ready to Use  ");
  lcd.setCursor(0,1);
  lcd.print(" Press to start ");
}

// Taking care of some special events.
void keypadEvent(KeypadEvent key) {
  switch (keypad.getState()) {
    case HOLD:
      if (key == '*' && !goToMenu) {
        Serial.println("Going to Menu");
        goToMenu = true;
        currentScreen = 0;
        printMenuScreen();
      }
      break;
    case PRESSED:
      if (key == '#' && goToMenu) {
        Serial.println("Exiting Menu");
        goToMenu = false;
        homeScreen();
      }
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Spray Pyrolysis System");
	// initialize the LCD
	lcd.begin();

	// Turn on the blacklight and print a message.
	lcd.backlight();
	lcd.print("Spray Pyrolysis");
  // delay(1000); //add later

  keypad.setHoldTime(1000);
  keypad.addEventListener(keypadEvent); // Add an event listener for this keypad

  //ToDo: Add stepper motor homeing code here

  homeScreen();
}

void loop() {
	char key = keypad.getKey();
  
  if (key) {
    Serial.print("Key: ");
    Serial.println(key);
    if (goToMenu) {
      inputAction(key);
      printMenuScreen();
    }
  }
}