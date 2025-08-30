#include <SPI.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Servo.h>

Servo myservo;
RTC_DS3231 rtc;

int T;

// data pins must be LOW to be turned on
// segment pins must be HIGH to be turned on
int D1 = 22;
int pinA = 27; 
int pinF = 4;
int D2 = 5;
int D3 = 6;
int pinB = 7; 
int pinE = 8;
int pinD = 9;
int DP = 10;
int pinC = 11;
int pinG = 12;
int D4 = 13;
int buzzerPin = 47;
int ledPin = 23;

// for changing the time
int buttonMODE = 3; // either set time mode or set alarm mode
int buttonSET = 2; // enables/ disables changing the time
int buttonUP = 19; // moves up by one
int buttonDOWN = 18; // moves down by one 
                 
int digit = 0; // which digit are we on? 0 - 2 for 1st digit, 0 - 9 for 2nd digit, 0 - 5 for 3rd digit, 0 - 9 for last digit
int mode = 0; // 0 for setting an alarm, 1 for setting the time

int rs = 35, en = 37, d4 = 39, d5 = 41, d6 = 43, d7 = 45;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int firstHourDig;
int secondHourDig;
int firstMinDig;
int secondMinDig;

int hourNum; // real time variables
int minNum;

int dummyHour; // placeholders
int dummyMin;

bool pause = 0; // pause clock? 0 = unpaused, 1 = paused

int set = 0; // set = 0 --> hours place, set = 1 --> minutes place, set =  2 --> exit changing mode

const int segments[8] = {27, 7, 11, 9, 8, 4, 12, 10};
// A, B, C, D, E, F, G, DP

const int digitPatterns[10][8] = {
  {1,1,1,1,1,1,0,0}, // 0
  {0,1,1,0,0,0,0,0}, // 1
  {1,1,0,1,1,0,1,0}, // 2
  {1,1,1,1,0,0,1,0}, // 3
  {0,1,1,0,0,1,1,0}, // 4
  {1,0,1,1,0,1,1,0}, // 5
  {1,0,1,1,1,1,1,0}, // 6
  {1,1,1,0,0,0,0,0}, // 7
  {1,1,1,1,1,1,1,0}, // 8
  {1,1,1,1,0,1,1,0}, // 9
};

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wed.", "Thurs.", "Friday", "Sat."};

// debounce code
int buttonState = LOW; //this variable tracks the state of the button, low if not pressed, high if pressed
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

int upCounter = 0;
int downCounter = 0;

int brightness = 0;      // Initial brightness (0 = off)
const int fadeStep = 25; // Brightness increment (adjust for speed)
bool isOnPhase = false;  // Tracks if LED is currently ON
bool alarmOn = 0;
bool disableAlarm = 1;

int alarmHour;
int alarmMin;

int pos = 0;
long lastPosChange = 0;
int posCounter;
bool increasing = 1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  lcd.begin(16, 2);
  attachInterrupt(digitalPinToInterrupt(buttonMODE), setMode, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonUP), changeTimeUp, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonDOWN), changeTimeDown, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonSET), setButton, RISING);

  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);
  pinMode(pinE, OUTPUT);
  pinMode(pinF, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(DP, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, INPUT);

  pinMode(buttonMODE, INPUT_PULLUP);
  pinMode(buttonUP, INPUT_PULLUP);
  pinMode(buttonDOWN, INPUT_PULLUP);
  pinMode(buttonSET, INPUT_PULLUP);

  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
  if(!rtc.begin()){
    Serial.println("RTC module not found");
    while (true);
  }
  if (rtc.lostPower()){
    rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
  }

}

void loop() {
  
  DateTime now = rtc.now();

  T = rtc.getTemperature();   // temp display
  lcd.setCursor(0,0);
  lcd.print("Temp: "); 
  lcd.setCursor(5, 0); 
  lcd.print(T);
  lcd.print((char)223); // degrees symbol
  lcd.print ("C ");

  lcd.setCursor(0,1);   // date display
  lcd.print(now.month(), DEC);
  lcd.print('/');
  lcd.print(now.day(), DEC);
  lcd.print('/');
  lcd.print(now.year(), DEC);
  lcd.print(" ");
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(" ");
  //delay(500);

  // time display code
  
  hourNum = now.hour();
  minNum = now.minute();

  if(!pause){
    displayCurrentTime(hourNum, minNum);
  }
  else if (pause){
    displayCurrentTime(dummyHour,dummyMin);
  }

  if(!disableAlarm){
    if(hourNum == alarmHour && minNum == alarmMin){
      alarmOn = 1;
      pause = 1;
    }
  }

  if (alarmOn){
    digitalWrite(buzzerPin, HIGH);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("WAKE UP !!!!");

    if (isOnPhase) {
    // LED ON for 1 second at current brightness
      analogWrite(ledPin, brightness);
      delay(500);
    
      // Increase brightness for next cycle (max 255 obv)
      brightness += fadeStep;
      if (brightness > 255) {
        brightness = 0; // Reset to min brightness
      }
      } else {
      // LED OFF for 1 second
       analogWrite(ledPin, 0);
       delay(500);
      }
  
    isOnPhase = !isOnPhase; // Toggle between ON/OFF
  }
  
}

void setMode(){

  buttonState = digitalRead(buttonMODE);

  //filter out any noise by setting a time buffer
    if ( (millis() - lastDebounceTime) > debounceDelay) {

    //if the button has been pressed, lets toggle the LED from "off to on" or "on to off"
      if ( (buttonState == HIGH) ) {
        mode++;
        Serial.println(mode);
        lastDebounceTime = millis(); //set the current time
      }
    }

  // mode = 0 --> nothing done, mode = 1 --> set alarm, mode = 2 --> set time
  
  dummyHour = hourNum; // we use the dummy variables when changing the time so that we don't actually change the real time variables
  dummyMin = minNum;

  if(alarmOn && mode == 1){ // if the alarm is off and the mode button is pressed once, the alarm will go off
    
    disableAlarm = 1;
    alarmOn = 0;
    pause = 0;
    mode = 0;
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledPin, LOW);
    posCounter = 0;
  }

  if(mode>1){ // mode cannot exceed 1
    mode = 0;
    pause = 0;
    set = 0;
    upCounter = 0;
    downCounter = 0;
  }

  if(mode == 1){
    // display alarm mode on lcd
    set = 0;
    pause = 1;
    lcd.setCursor(10,0);
    lcd.print("      "); // make sure the number of spaces don't interfere with anything
    lcd.setCursor(10,0);
    lcd.print("ALARM ");
  }
  else{
    lcd.setCursor(10,0);
    lcd.print("     "); // clear the lcd screen-- no mode
  }
  
}

void setButton(){ // this is to set an alarm
  //... wait for user input
  
  buttonState = digitalRead(buttonSET);

  //filter out any noise by setting a time buffer
    if ( (millis() - lastDebounceTime) > debounceDelay) {

    //if the button has been pressed, lets toggle the LED from "off to on" or "on to off"
      if ( (buttonState == HIGH) ) {
        Serial.println(set);
        set++;
        Serial.println(set);
        lastDebounceTime = millis(); //set the current time
      }
    }
  
    if (set>1){ // resetting everythinggg
    
      if(mode == 1){ // if we're done changing the alarm and want to set an alarm
        setAlarm(dummyHour, dummyMin);
      }
    
      pause = 0; // go back to displaying real time
      mode = 0;  // return to base mode
      upCounter = 0; // resetting counters
      downCounter = 0;
      set = 0;

      lcd.setCursor(10,0);
      lcd.print("     "); // clear the lcd screen-- no mode
    }
    if (set == 1){
      upCounter = 0; // resetting counters
      downCounter = 0;
    }

}

void changeTimeUp(){

  if(upCounter == 0){
    if (set == 0){
      dummyHour = hourNum; // only changing the hours place if we're in hours mode
    }
    dummyMin = minNum;
  }
  upCounter++;

  buttonState = digitalRead(buttonUP);

  //filter out any noise by setting a time buffer
    if ( (millis() - lastDebounceTime) > debounceDelay) {

    //if the button has been pressed, lets toggle the LED from "off to on" or "on to off"
      if ( (buttonState == HIGH) ) {
  
        if (set == 0){ // changing hour
          dummyHour++;
            if(dummyHour>24){
              dummyHour = 0;
            }
        }
        else if(set == 1){
          dummyMin++;
            if(dummyMin>59){
              dummyMin = 0;
              dummyHour++;
              if(dummyHour>24){
                dummyHour = 0;
              }
            }
        }

        lastDebounceTime = millis(); //set the current time
      }
    }

}


void changeTimeDown(){
  buttonState = digitalRead(buttonDOWN);

  if(downCounter == 0){
    if (set == 0){
      dummyHour = hourNum; // only changing the hours place if we're in hours mode
    }
    dummyMin = minNum;
  }
  downCounter++;

  //filter out any noise by setting a time buffer
    if ( (millis() - lastDebounceTime) > debounceDelay) {

    //if the button has been pressed, lets toggle the LED from "off to on" or "on to off"
      if ( (buttonState == HIGH) ) { // -----------------------------------

        if (set == 0){ // if we're able to change hour
          dummyHour--;
            if(dummyHour<0){
              dummyHour = 0;
            }
        }
        else if(set == 1){
          dummyMin--;
            if(dummyMin<0){
              dummyMin = 0;
            }
        }
        lastDebounceTime = millis(); //set the current time
      }
    }
}

void setAlarm(int hourIn, int minIn){
  
  Serial.println("time to set the alarm!");
  disableAlarm = 0;
  alarmHour = hourIn;
  alarmMin = minIn;
  
  /*
  DateTime now1 = rtc.now();
  int day = now1.day();
  int year = now1.year();
  int month = now1.month();
  int hour = now1.hour();
  
  
  if(alarmHour<hour || alarmHour == hour){
    day++;
  }
  */

  //DateTime alarm1Time = DateTime(2025, 5, 10, alarmHour, alarmMin, 0);

  ds3231.setA1Time(0, alarmHour, alarmMin, 0, DS3231_MATCH_H_M_S);
  //rtc.setAlarm1(DateTime(2025,5,10,alarmHour,alarmMin,0), DS3231_A1_Hour);
  
  Serial.print("your alarm has been set!");
  

}

void displayHour(int hourNum){

  firstHourDig = hourNum / 10;
  secondHourDig = hourNum % 10;

        // display 1st dig (hour)
  turnOnDigit(1);
  displayTime(firstHourDig);
  delay(5);
        // display 2nd dig (hour)
  turnOnDigit(2);
  displayTime(secondHourDig);
  delay(5);

}

void displayMin(int minNum){
  
  firstMinDig = minNum / 10;
  secondMinDig = minNum % 10;

        // display 3rd dig (minute)
  turnOnDigit(3);
  displayTime(firstMinDig);
  delay(5);
        // display 4th dig (minute)
  turnOnDigit(4);
  displayTime(secondMinDig);
  delay(5);
}

void displayCurrentTime(int hour, int minute){

  displayHour(hour);
  displayMin(minute);
  
}

void turnOnDigit(int digit){
  switch(digit){
    case 1:
      digitalWrite(D1, LOW); // only 1st digit is on
      digitalWrite(D2, HIGH);
      digitalWrite(D3, HIGH);
      digitalWrite(D4, HIGH);
    break;
    case 2: 
      digitalWrite(D1, HIGH); 
      digitalWrite(D2, LOW);  // only 2nd digit is on
      digitalWrite(D3, HIGH);
      digitalWrite(D4, HIGH);
    break;
    case 3:
      digitalWrite(D1, HIGH); 
      digitalWrite(D2, HIGH);
      digitalWrite(D3, LOW);  // only 3rd digit is on
      digitalWrite(D4, HIGH);
    break;
    case 4:
      digitalWrite(D1, HIGH); 
      digitalWrite(D2, HIGH);
      digitalWrite(D3, HIGH);
      digitalWrite(D4, LOW);  // only 4th digit is on
    break;
  }
}

void displayTime(int number){ 
  switch(number){
    case 0:
      for(int i = 0; i<8; i++){
        digitalWrite(segments[i], digitPatterns[0][i]);
      }
      break;
    case 1:
      for(int i = 0; i<8; i++){
        digitalWrite(segments[i], digitPatterns[1][i]);
      }
      break;
    case 2:
      for(int i = 0; i<8; i++){
        digitalWrite(segments[i], digitPatterns[2][i]);
      }
      break;
    case 3:
      for(int i = 0; i<8; i++){
        digitalWrite(segments[i], digitPatterns[3][i]);
      }
      break;
    case 4:
      for(int i = 0; i<8; i++){
        digitalWrite(segments[i], digitPatterns[4][i]);
      }
      break;
    case 5:
      for(int i = 0; i<8; i++){
        digitalWrite(segments[i], digitPatterns[5][i]);
      }
      break;
    case 6:
      for(int i = 0; i<8; i++){
        digitalWrite(segments[i], digitPatterns[6][i]);
      }
      break;
    case 7:
      for(int i = 0; i<8; i++){
        digitalWrite(segments[i], digitPatterns[7][i]);
      }
      break;
    case 8:
      for(int i = 0; i<8; i++){
        digitalWrite(segments[i], digitPatterns[8][i]);
      }
      break;
    case 9:
      for(int i = 0; i<8; i++){
        digitalWrite(segments[i], digitPatterns[9][i]);
      }
      break;
  }
  
}

void alarm(){
  Serial.println("meow!");
  alarmOn = 1;
  pause = 1;
}

