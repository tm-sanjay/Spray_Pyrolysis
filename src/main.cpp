#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>


/* Features
 * Connect  to the wifi
 * Settings in the Android App
 * Start and Stop in both App and keypad
//  * Long press to go to menu
//  * In normal mode, press the keypad to start
//  * When started is press the keypad to stop
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
byte colPins[COLS] = {A0, A1, A2}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//LCD Menu Variables

bool goToMenu = false;
const int numOfScreens = 4;
int currentScreen = 0;
String screens[numOfScreens][5] = {
    //{Title,   units,   min,   max,  steps}
    {"1.Plotter Speed", "cm/s", "0", "5", "1"}, 
    {"2.Liquid Speed", "l/m", "0", "20", "2"}, 
    {"3.Bed Temp", "*C", "0", "25", "5"},
    {"4.Logs", " ", "0", "2", "1"}
  };
int parameters[numOfScreens] = {1,2,5,0}; //default values

//INPUTS
#define DOOR_PIN A3
#define HOME_STOP_PIN 6
#define END_STOP_PIN 7

//OUTPUTS
#define MOTOR_STEP_PIN 9
#define MOTOR_DIR_PIN 10
#define PUMP_MOTOR_PIN 11 //only one direction //IN1 pin
#define SSR_PIN 12
#define COMPRESSOR_PIN 13


//Variables
bool runState = false;

//enum for the process states
enum Process {
  START,
  END,
  NONE
};

enum Process processState = NONE;

//Variables for not blocking progress indication
unsigned long lastTime = 0;
int dotPosition = 0;

//inital declarations
void inputAction(char input);
void parameterChange(int key);
void keypadEvent(KeypadEvent key);
void printMenuScreen();
void stepperMotorHome();
void homeScreen();
void startProcessScreen();
void startProcess();
void endProcessScreen();
void endProcess();
void keyHandler();

//---------------Menu Functions -----------------
void inputAction(char input) {
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
    //incremeant value
    parameterChange(0);
  }
  //value down
  else if(input == '8') {
    //decrement value
    parameterChange(1);
  }
}

void parameterChange(int key) {
  int presentValue = parameters[currentScreen];
  int minValue = screens[currentScreen][2].toInt();
  int maxValue = screens[currentScreen][3].toInt();
  int stepValue = screens[currentScreen][4].toInt();

  if (key == 0) {
    if (presentValue < maxValue) {
      presentValue += stepValue;
    }
  } else if (key == 1) {
    if (presentValue > minValue) {
      presentValue -= stepValue;
    }
  }
  parameters[currentScreen] = presentValue;
  Serial.println( "min:" + String(minValue) + " max:" + String(maxValue) + " step:" + String(stepValue) + " present:" + String(presentValue));
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

void printMenuScreen() {
  lcd.clear();
  lcd.print(screens[currentScreen][0]);
  lcd.setCursor(0,1);
  lcd.print("     ");
  lcd.print(parameters[currentScreen]);
  lcd.print(" ");
  lcd.print(screens[currentScreen][1]);
}
//---------------End Menu Functions -----------------
void stepperMotorHome() {

}

void homeScreen() {
  lcd.clear();
  lcd.print("  Ready to Use  ");
  lcd.setCursor(0,1);
  lcd.print(" Press to start ");
}

void startProcessScreen() {
  lcd.clear();
  lcd.print("  Processing    ");
  lcd.setCursor(0,1);
  lcd.print("  Press to stop ");
}

void progressScreen() {
  //non blocking progress indication
  if (millis() - lastTime > 100) {
    lastTime = millis();
    if (dotPosition == 0) {
      lcd.setCursor(12,0);
      lcd.print(".");
      dotPosition = 1;
    } else if (dotPosition == 1) {
      lcd.setCursor(13,0);
      lcd.print(".");
      dotPosition = 2;
    } else if (dotPosition == 2) {
      lcd.setCursor(14,0); 
      lcd.print(".");
      dotPosition = 3;
    } else {
      lcd.setCursor(12,0);
      lcd.print("   ");
      dotPosition = 0;
    }
  }
}

void startProcess() {
  progressScreen();
}

void endProcessScreen() {
  lcd.clear();
  lcd.print(" Process Ended! ");
  lcd.setCursor(0,1);
  lcd.print(" Press to start ");
}

void endProcess() { 
}

void keyHandler() {
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key: ");
    Serial.println(key);
    if (goToMenu) {
      inputAction(key);
      printMenuScreen();
      // runState = false;
      processState = NONE;
    }
    else {
      if(key == '*' || key == '#') return; // '*,#' are menu keys

      if (processState == START) {
        // runState = false;
        processState = END;
        endProcessScreen();
        Serial.println("Process Ended");
      } else {
        // runState = true;
        processState = START;
        startProcessScreen();
        Serial.println("Starting Process");
      }
    }
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

  pinMode(MOTOR_STEP_PIN, OUTPUT);
  pinMode(MOTOR_DIR_PIN, OUTPUT);
  pinMode(PUMP_MOTOR_PIN, OUTPUT);
  pinMode(SSR_PIN, OUTPUT);
  pinMode(COMPRESSOR_PIN, OUTPUT);
  pinMode(HOME_STOP_PIN, INPUT);
  pinMode(END_STOP_PIN, INPUT);
  pinMode(DOOR_PIN, INPUT);

  digitalWrite(MOTOR_STEP_PIN, LOW);
  digitalWrite(MOTOR_DIR_PIN, LOW);
  digitalWrite(PUMP_MOTOR_PIN, LOW);
  digitalWrite(SSR_PIN, LOW);
  digitalWrite(COMPRESSOR_PIN, LOW);
  
  //ToDo: Add stepper motor homeing code here
  stepperMotorHome();
  homeScreen();
}

void loop() {
	keyHandler();

  if (processState == START) {
    startProcess();
  }
  else if (processState == END) {
    endProcess();
  }
}