#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <AccelStepper.h>

/* Features
//  * Connect to the wifi
//  * Settings in the Android App
//  * Start and Stop in both App and keypad
//  * Long press to go to menu
//  * In normal mode, press the keypad to start
//  * When started is press the keypad to stop
//  * Limit switch at the door to stop if the door is open
//  * When door is open, should not be able to start, but can go to menu
//  * Once process is ended, SSR should be off
//  * Every to before starting Should auto home x plotter
//  * When Start is pressed, should wait until set temp is reached
//  * If the door is open it should say close the door and press to start
//  * When the process is started, timer should start and when ended should stop, and record the time
//  * every thing should be reflected in the app
 */

/* Menu to be added
//  * Settings 
//  * - X plotter Speed
//  * - Water Pump Speed
//  * - Base Plate Temperature
//  * Logs
//  * - all run time 
 */

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2); //GPIO 22,21 for i2c

//Keypad Variables
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {12, 14, 27, 26}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {25, 33, 32}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//LCD Menu Variables

bool goToMenu = false;
const int numOfScreens = 4;
int currentScreen = 0;
String screens[numOfScreens][5] = {
    //{Title,   units,   min,   max,  steps}
    {"1.Plotter Speed", "cm/s", "1", "10", "1"}, 
    {"2.Liquid Speed", "ml/m", "2", "17", "1"}, 
    {"3.Bed Temp", "*C", "50", "300", "5"},
    {"4.Logs", " ", "0", "0", "0"} //This log wont be displayed
  };
int parameters[numOfScreens] = {1, 2, 50, 0}; //default values

uint8_t logScreenPosition = 3;

//INPUTS
#define TEMP_SENSOR_PIN 13 //ADC //max temp is 250
#define DOOR_PIN 17 //pullup
#define HOME_STOP_PIN 5 //pullup
#define END_STOP_PIN 18 //pullup

//OUTPUTS
#define MOTOR_STEP_PIN 2 //PWM
#define MOTOR_DIR_PIN 15
#define PUMP_MOTOR_PIN 23 //only one direction PWM //IN1 pin
#define SSR_PIN 4
#define COMPRESSOR_PIN 16


//Variables
bool doorState = false;
bool doorPrevState = true;

bool homeStopState = false;
bool endStopState = false;

//enum for the process states
enum Process {
  START,
  END,
  NONE
};

enum Process processState = NONE;

//Variables for not blocking progress indication
unsigned long dotChangedTime = 0;
int dotPosition = 0;

//Variables to track the process time
unsigned long startTime = 0;
unsigned long totalTime = 0;
int runTimeList[10];
int runTimeListIndex = 0;

uint8_t pumpSpeed = 0;


//Variables for temperature
// resistance at 25 degrees C
#define THERMISTORNOMINAL 100000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 100000 

int samples[NUMSAMPLES];

unsigned long tempCheckTime = 0;

//Stepper Motor Variables
#define motorInterfaceType 1 //A4988
long initial_homing = -1;  // Used to Home Stepper at startup
// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorInterfaceType, MOTOR_STEP_PIN, MOTOR_DIR_PIN);

unsigned long ssrTimer = millis();
bool ssrOnFor10Minutes = false; //flag to indicate if SSR is on for initial 10 minutes
bool ssrCurrentState = false; 
unsigned long ssrOnTime = 10;//in minutes

//inital declarations
void inputAction(char input);
void parameterChange(int key);
void keypadEvent(KeypadEvent key);
void printMenuScreen();
void stepperMotorHome();
void stepperMotorMove();
void triggerSSR();
void homeScreen();
void startProcessScreen();
void startProcess();
void endProcessScreen();
void endProcess();
void checkForEndStop();
void keyHandler();
float checkTemp();
void activateSSR();

//------------------Wifi setup-------------------
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

const char* PARAM_KEY = "fun"; //query parameter KEY

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Page Not found");
}

void setup_wifi() {
  WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("esp-AP","12345678");
    
    Serial.print("AP IP address: ");
    IPAddress myIP = WiFi.softAPIP();
    Serial.println(myIP);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "Spray Pyrolysis");
    });

    //start process
    server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("Strated Form http");
      processState = START;
      startProcessScreen();
      Serial.println("Starting Process");
      request->send(200, "text/plain", "ESP:Started");
    });
    //stop Process
    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("Stoped Form http");
      processState = END;
      endProcessScreen();
      Serial.println("Process Ended");
      request->send(200, "text/plain", "ESP:Stopped");
    });
    //update temperature
    server.on("/temp", HTTP_POST, [](AsyncWebServerRequest *request){
      String message;
      if (request->hasParam(PARAM_KEY)) {
        message = request->getParam(PARAM_KEY)->value();
        Serial.print("http /temp:");
        Serial.println(message.toInt());
        parameters[2] = message.toInt();
      } else {
        message = "No message sent";
      }
      request->send(200, "text/plain", "Recived: " + message);
    });
    //update Pump speed
    server.on("/pump", HTTP_POST, [](AsyncWebServerRequest *request){
      String message;
      if (request->hasParam(PARAM_KEY)) {
        message = request->getParam(PARAM_KEY)->value();
        Serial.print("http /pump:");
        Serial.println(message.toInt());
        parameters[1] = message.toInt();
      } else {
        message = "No message sent";
      }
      request->send(200, "text/plain", "Recived: " + message);
    });
    //update plotter speed
    server.on("/plotter", HTTP_POST, [](AsyncWebServerRequest *request){
      String message;
      if (request->hasParam(PARAM_KEY)) {
        message = request->getParam(PARAM_KEY)->value();
        Serial.print("http /plotter:");
        Serial.println(message.toInt());
        parameters[0] = message.toInt();
      } else {
        message = "No message sent";
      }
      request->send(200, "text/plain", "Recived: " + message);
    });
    //update ssrOnTime
    server.on("/ssrontime", HTTP_POST, [](AsyncWebServerRequest *request){
      String message;
      if (request->hasParam(PARAM_KEY)) {
        message = request->getParam(PARAM_KEY)->value();
        Serial.print("http /ssrontime:");
        Serial.println(message.toInt());
        ssrOnTime = message.toInt();
      } else {
        message = "No message sent";
      }
      request->send(200, "text/plain", "Recived: " + message);
    });

    server.onNotFound(notFound);

    server.begin();
}
//---------------------Ens wifi-setup-----------------

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
  if(currentScreen == logScreenPosition) {
    if (key == 0) {
      int lenthOfList = sizeof(runTimeList)/sizeof(runTimeList[0]);
      //i.e index is 0-9 (-1) and first 2 elements are visible(-2), so -3
      if(runTimeListIndex < lenthOfList-3) { 
        runTimeListIndex++;
      }
    }else if (key == 1) {
      //runTimeListIndex decrement upto 0
      if(runTimeListIndex > 0) {
        runTimeListIndex--;
      }
    }
    return;
  }

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
      if (key == '#' && !goToMenu && processState != START) {
        Serial.println("Going to Menu");
        goToMenu = true;
        currentScreen = 0;
        runTimeListIndex = 0;
        printMenuScreen();
      }
      break;
    case PRESSED:
      if (key == '#' && goToMenu && processState != START) {
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
  //check for log screen
  if(currentScreen == logScreenPosition) {
    // Serial.println("Log Screen");
    lcd.clear();
    lcd.print("4.Logs =>");
    lcd.setCursor(9,0);
    lcd.print(runTimeListIndex+1); //index stars from 0 so add 1
    lcd.print("-");
    lcd.print(runTimeList[runTimeListIndex+1]); //0,1 index value are same. i.e. display from index 1
    lcd.setCursor(9,1);
    lcd.print(runTimeListIndex+2); //index stars from 0, this is second value so add 2
    lcd.print("-");
    lcd.print(runTimeList[runTimeListIndex+2]);
    return;
  }

  lcd.clear();
  lcd.print(screens[currentScreen][0]);
  lcd.setCursor(0,1);
  lcd.print("     ");
  lcd.print(parameters[currentScreen]);
  lcd.print(" ");
  lcd.print(screens[currentScreen][1]);
}
//---------------End Menu Functions -----------------

//---------------Functions -----------------
void stepperMotorHome() {
  //  Set Max Speed and Acceleration of each Steppers at startup for homing
  stepper.setSpeed(100.0);
  stepper.setMaxSpeed(100.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper.setAcceleration(100.0);  // Set Acceleration of Stepper

  Serial.print("Stepper is Homing . . . .");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Homing Stepper..");
  lcd.setCursor(0,1);
  lcd.print("  Please Wait   ");
  // Make the Stepper move counter clockwise until the switch is activated(LOW)
  // while(digitalRead(HOME_STOP_PIN)) {
  //   // Serial.println("CCW");
  //   stepper.moveTo(initial_homing);  // Set the position to move to
  //   initial_homing--;  // Decrease by 1 for next move if needed
  //   stepper.run();  // Start moving the stepper
  //   delay(5);
  // }

  stepper.setCurrentPosition(0);  // Set the current position as zero for now
  stepper.setSpeed(100.0);        // Set the speed of the stepper
  stepper.setMaxSpeed(100.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper.setAcceleration(100.0);  // Set Acceleration of Stepper
  initial_homing=1;

  // Make the Stepper move CW until the switch is deactivated(HIGH)
  // while (!digitalRead(HOME_STOP_PIN)) { 
  //   // Serial.println("CW");
  //   stepper.moveTo(initial_homing);  
  //   stepper.run();
  //   initial_homing++;
  //   delay(5);
  // }
  
  stepper.setCurrentPosition(0);
  Serial.println("Homing Completed");
  Serial.println("");
  stepper.setMaxSpeed(1000.0);      // Set Max Speed of Stepper (Faster for regular movements)
  stepper.setAcceleration(1000.0);  // Set Acceleration of Stepper
  
  float stepperSteps = parameters[0] * 100; // index of Stepper Speed is 0. (value*100)
  Serial.println(stepperSteps);
  stepper.setSpeed(stepperSteps); 
}

bool previousDir = false;
bool presentDir = true;

void stepperMotorMove() {
  stepper.setCurrentPosition(0);
  if(!digitalRead(HOME_STOP_PIN)) {
    Serial.println("Reversing the Direction!");
    //reverse the direction every time the home switch is pressed
    presentDir = !presentDir;
    delay(1000);
  }
  if(previousDir == false) {
      stepper.moveTo(parameters[0] * 100);
      stepper.runSpeed();
      stepper.run();
    }
  else {
      stepper.moveTo(-parameters[0] * 100);
      stepper.runSpeed();
      stepper.run();
    }
  previousDir = presentDir;
}


void triggerSSR() {
  //Turn on the SSR for 20 seconds with out delay
  if(!ssrOnFor10Minutes){
    if(millis() - ssrTimer > 5000) { //make this 1000*60*10
      digitalWrite(SSR_PIN, LOW);
      ssrOnFor10Minutes = true;
      Serial.println("SSR was on For first 10 min");
    }
    else {
      digitalWrite(SSR_PIN, HIGH);
      ssrCurrentState = HIGH;
    }
  }
  else {
    //toggle the SSR every 10 seconds
    if(millis() - ssrTimer > ssrOnTime * 1000) { //make this 1000*60
      ssrTimer = millis();
      ssrCurrentState = !ssrCurrentState;
      digitalWrite(SSR_PIN, ssrCurrentState);
      Serial.println("SSR State toggle");
    }
  }
}

void homeScreen() {
  lcd.clear();
  lcd.print("  Ready to Use  ");
  lcd.setCursor(0,1);
  lcd.print(" Press to start ");
}

void startProcessScreen() {
  //home the stepper motor
  stepperMotorHome();//Todo: add after testing

  pumpSpeed = map(parameters[1], 2, 17, 70, 255);
  Serial.println("Pump Speed:" + String(pumpSpeed));

  lcd.clear();
  lcd.print("Processing_400*C");
  lcd.setCursor(0,1);
  lcd.print("  Press to stop ");
  //start the timer
  startTime = millis();
}

void progressScreen() {
  //non blocking progress indication
  if (millis() - dotChangedTime > 100) {
    dotChangedTime = millis();
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
  // progressScreen();
  checkForEndStop();

  digitalWrite(COMPRESSOR_PIN, HIGH);
  analogWrite(PUMP_MOTOR_PIN, pumpSpeed);
  // activateSSR();
  stepperMotorMove();
}

void endProcessScreen() {
  lcd.clear();
  lcd.print(" Process Ended! ");
  lcd.setCursor(0,1);
  lcd.print(" Press to start ");
  totalTime = millis() - startTime;
  Serial.print("Time taken: ");
  Serial.println(totalTime);
  // add the time to the list
  runTimeList[0] = totalTime;
  // shift the list
  int lenthOfList = sizeof(runTimeList)/sizeof(runTimeList[0]);
  for (int i = lenthOfList; i > 0; i--) {
    runTimeList[i] = runTimeList[i-1];
  }
  // Serial.print("List of times: ");
  // for (int i = 0; i < lenthOfList; i++) {
  //   Serial.print(runTimeList[i]);
  //   Serial.print(" ");
  // }
}

void endProcess() { 
  processState = END;
  digitalWrite(COMPRESSOR_PIN, LOW);
  analogWrite(PUMP_MOTOR_PIN, 0);
  //digitalWrite(SSR_PIN, LOW);
}

void doorCheck() {
  doorState = !digitalRead(DOOR_PIN); //Pull up resistor
  if (doorState != doorPrevState) {
    doorPrevState = doorState;
    if (doorState == HIGH) {
      Serial.println("Door is Closed");
      homeScreen();
    } else {
      Serial.println("Door is open");
      lcd.clear();
      lcd.print(" Door is open!   ");
      lcd.setCursor(0,1);
      lcd.print(" Close to start  ");
      //Stop all the processes
      endProcess();
    }
  }
}

void checkForEndStop() {
  if (digitalRead(END_STOP_PIN) == LOW && processState == START) {
    processState = END;
    Serial.println("End-Stop Triggered");
    endProcessScreen();
    endProcess();
  }
}

void keyHandler() {
  //Todo: add debounce here
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key: ");
    Serial.println(key);
    if (goToMenu) {
      inputAction(key);
      printMenuScreen();
      processState = NONE;
    }
    else {
      if(key == '*' || key == '#') return; // '*,#' are menu keys
      if(doorState == LOW) return; //door is open
      if (processState == START) {
        processState = END;
        endProcessScreen();
        Serial.println("Process Ended");
      } else {
        processState = START;
        startProcessScreen();
        Serial.println("Starting Process");
      }
    }
  }
}

float checkTemp() {
  uint8_t i;
  float average;

  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
    samples[i] = analogRead(TEMP_SENSOR_PIN);
    delay(5);
  }
  
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
      average += samples[i];
  }
  average /= NUMSAMPLES;

  // Serial.print("Average analog reading "); 
  // Serial.println(average);
  
  // convert the value to resistance
  average = 1023 / average - 1; //for esp32 4095 / average - 1;
  average = SERIESRESISTOR / average;
  // Serial.print("Thermistor resistance "); 
  // Serial.println(average);
  
  float steinhart;    // value of the steinhart-Hart equation 
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert absolute temp to C
  
  Serial.print("Temperature "); 
  Serial.print(steinhart);
  Serial.println(" *C");

  return steinhart;
}

void activateSSR() {
  //every 2 seconds, check the temperature
  if (millis() - tempCheckTime > 2000) {
    tempCheckTime = millis();
    Serial.print("Checking Temperature: ");
    Serial.print(checkTemp());
    Serial.println(" p2: " + String(parameters[2]));
    float temp = checkTemp();
    //parameters[2] is temp. i.e. 2 is the index of temp Menu
    if (temp < parameters[2] - 10) { //the range is setTemp and setTemp - 10
      Serial.println("Temprature is less, SSR High");
      digitalWrite(SSR_PIN, HIGH);
    } else if (temp > parameters[2]) {
      Serial.println("Temprature is greater, SSR Low");
      digitalWrite(SSR_PIN, LOW);
    }
  }
}
//-----------------End Functions ---------------------

void setup() {
  Serial.begin(9600); //! donot remove this line
  Serial.println("Spray Pyrolysis System");
	
  //initialize the IO pins
  pinMode(MOTOR_STEP_PIN, OUTPUT);
  pinMode(MOTOR_DIR_PIN, OUTPUT);
  pinMode(PUMP_MOTOR_PIN, OUTPUT);
  pinMode(SSR_PIN, OUTPUT);
  pinMode(COMPRESSOR_PIN, OUTPUT);
  pinMode(HOME_STOP_PIN, INPUT_PULLUP);
  pinMode(END_STOP_PIN, INPUT_PULLUP);
  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(TEMP_SENSOR_PIN, INPUT);

  digitalWrite(MOTOR_STEP_PIN, LOW);
  digitalWrite(MOTOR_DIR_PIN, LOW);
  analogWrite(PUMP_MOTOR_PIN, 0);
  digitalWrite(SSR_PIN, LOW);
  digitalWrite(COMPRESSOR_PIN, LOW);
  
  keypad.setHoldTime(1000); //hold time for the Menu keys
  keypad.addEventListener(keypadEvent); // Add an event listener for this keypad

  // initialize the LCD
	lcd.begin();
  // lcd.init();
	lcd.backlight();
	lcd.print("Spray Pyrolysis");
  // delay(1000); //add later
  setup_wifi();
  // stepperMotorHome();
  homeScreen();
  // analogReference(EXTERNAL); //not applicable for esp32
}

void loop() {
  triggerSSR();
	keyHandler();
  if(!goToMenu) {
    doorCheck();
    if (processState == START) {
      startProcess();
    }
    else if (processState == END) {
      endProcess();
    }
  }
}