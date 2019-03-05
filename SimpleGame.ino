// 12345678901234567890
// S: XXXX   High: XXXX
//
//      [12345678]
//            [X]

// Buttons: New Game, Go, -, +

// Every Duration add number to the board string
// if "Up" button then add current number
// if "Down" button then subtruct current number
// if "Go" button then Check current number against Board[length]

// use EEPROM to save high score

#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <toneAC.h>
#include "pitches.h"

String Board;
int Score;
int HighScore;
int CurrentValue;
int addressEEPROM = 10;
int Duration;
boolean GameOn = false;
long StartGameTime;
int Count;
boolean Mute = false;

int adc_key_val[5] = {50, 200, 400, 600, 800 };
int NUM_KEYS = 5;
int adc_key_in;
int key = -1;
int oldkey = -1;

LiquidCrystal_I2C lcd(0x27, 20, 4);

int ReadHighScore()
{
  int Value, Value1;
  Value = EEPROM.read(addressEEPROM);
  Value1 = EEPROM.read(addressEEPROM + 1);
  return Value * 256 + Value1;
}

void WriteHighScore(int NewHighScore)
{
  EEPROM.write(addressEEPROM, NewHighScore / 256);
  EEPROM.write(addressEEPROM + 1, NewHighScore % 256);
}

// Convert ADC value to key number
int get_key(unsigned int input)
{
  int k;
  for (k = 0; k < NUM_KEYS; k++)
  {
    if (input < adc_key_val[k])
    {
      return k;
    }
  }
  if (k >= NUM_KEYS)k = -1; // No valid key pressed
  return k;
}

void EndGame()
{
  GameOn = false;
  // 12345678901234567890
  //	 Final Score
  //         XXXX
  //    New High Score
  //
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Final Score");
  lcd.setCursor(8, 2);
  lcd.print(Score);
  lcd.setCursor(10, 0);
  lcd.print("High: ");
  lcd.setCursor(16, 0);
  lcd.print(HighScore);
  // Check for HighScore
  if (Score > HighScore)
  {
    WriteHighScore(Score);
    HighScore = Score;
    lcd.setCursor(3, 3);
    lcd.print("New High Score");
  }
  if (!Mute) {
    toneAC (NOTE_F4, 4, 200, false);
    delay (100);
    toneAC (NOTE_A4, 4, 400, false);
    delay (100);
    toneAC (NOTE_C5, 4, 400, true);
  }
}

void NewGame()
{
  // 12345678901234567890
  //
  //      Get Ready!
  //         [3]
  lcd.clear();
  lcd.setCursor(10, 0);
  lcd.print("High: ");
  lcd.setCursor(16, 0);
  lcd.print(HighScore);
  lcd.setCursor(5, 1);
  lcd.print("Get Rready!");
  lcd.setCursor(8, 2);
  lcd.print("[3]");
  delay(700);
  lcd.setCursor(9, 2);
  lcd.print("2");
  delay(700);
  lcd.setCursor(9, 2);
  lcd.print("1");
  delay(150);
  if (!Mute) {
    toneAC (NOTE_C4, 3, 100, false);
    delay(50);
    toneAC (NOTE_C6, 3, 100, false);
    delay(50);
    toneAC (NOTE_C4, 3, 100, false);
    Serial.println("buzz");
  }
  else {
    delay(500);
  }
  // 12345678901234567890
  // S: 0000   High: XXXX
  //
  //      [        ]
  //         [0]
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("S: 0000   High: ");
  lcd.setCursor(16, 0);
  lcd.print(HighScore);
  lcd.setCursor(5, 2);
  lcd.print("[        ]");
  lcd.setCursor(8, 3);
  lcd.print("[0]");

  GameOn = true;
  Score = 0;
  Count = 0;
  CurrentValue = 0;
  Board = "";
  Duration = 1500;
  StartGameTime = millis();
}


void setup() {
  Serial.begin(9600); //set up serial port
  Serial.println("Setup");
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Board = "";

  pinMode(4, OUTPUT); // set a pin for buzzer output

  HighScore = ReadHighScore();
  lcd.begin(); 
  lcd.backlight();

  // randomSeed(analogRead(0));
  NewGame();
}

void loop()
{
  adc_key_in = analogRead(0); // read the value from the sensor
  key = get_key(adc_key_in); // convert into key press
  if (key != oldkey) // if keypress is detected
  {
    delay(50); // wait for debounce time
    adc_key_in = analogRead(0); // read the value from the sensor
    key = get_key(adc_key_in); // convert into key press
    if (key != oldkey)
    {
      oldkey = key;
      if (key >= 0) {
        switch (key)
        {
          case 0:
            Serial.println("NEW GAME just pressed");
            NewGame();
            break;
          case 1:
            if (GameOn) {
              Serial.println("UP just pressed");
              if (CurrentValue == 9) {
                CurrentValue = 0;
              }
              else {
                CurrentValue = CurrentValue + 1;
              }
              lcd.setCursor(9, 3);
              lcd.print(CurrentValue);
            }
            break;
          case 2:
            if (GameOn) {
              Serial.println("DOWN just pressed");
              if (CurrentValue == 0) {
                CurrentValue = 9;
              }
              else {
                CurrentValue = CurrentValue - 1;
              }
              lcd.setCursor(9, 3);
              lcd.print(CurrentValue);
            }
            break;
          case 3: // Do nothing (Right)
            Mute = !Mute;
            break;
          case 4:
            if (GameOn && Board.length() > 0) {
              Serial.println("GO just pressed");
              Serial.println(Board.charAt(Board.length() - 1) - '0');
              Serial.println(CurrentValue);
              if (!Mute) {
                toneAC (NOTE_F4, 4, 30, true);
              }
              if ((Board.charAt(Board.length() - 1) - '0') == CurrentValue) { // '0'=48
                Serial.println(Board);
                Board = Board.substring(0, Board.length() - 1);
                Serial.println(Board);
                lcd.setCursor(6, 2);
                lcd.print(Board + " ");
                Score = Score + (8 - Board.length());
                lcd.setCursor(3, 0); // update Score Display
                if (Score < 1000) lcd.print("0");
                if (Score < 100) lcd.print("0");
                if (Score < 10) lcd.print("0");
                lcd.print(Score);
              }
            }
            break;
        } // switch (key)
      }
    }
  }
  delay(20);
  if ((millis() - StartGameTime > Duration) && GameOn) {
    StartGameTime = millis();
    // check Board
    if (Board.length() == 8) {
      EndGame();
    }
    else { // Board.length less then 8
      // Add number to Board
      Board = String(random(10)) + Board;
      lcd.setCursor(6, 2);
      lcd.print(Board);
      Count = Count + 1;
      if (Count % 20 == 0) { // Change the Duration after every 20 numbers
        Duration = Duration - 100;
      }
    }
  } // Duration

} // loop

