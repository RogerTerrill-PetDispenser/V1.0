/*
  LiquidCrystal Library - Autoscroll

  Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
  library works with all LCD displays that are compatible with the
  Hitachi HD44780 driver. There are many of them out there, and you
  can usually tell them by the 16-pin interface.

  This sketch demonstrates the use of the autoscroll()
  and noAutoscroll() functions to make new text scroll or not.

  The circuit:
   LCD RS pin to digital pin 12
   LCD Enable pin to digital pin 11
   LCD D4 pin to digital pin 5
   LCD D5 pin to digital pin 4
   LCD D6 pin to digital pin 3
   LCD D7 pin to digital pin 2
   LCD R/W pin to ground
   10K resistor:
   ends to +5V and ground
   wiper to LCD VO pin (pin 3)

  Library originally added 18 Apr 2008
  by David A. Mellis
  library modified 5 Jul 2009
  by Limor Fried (http://www.ladyada.net)
  example added 9 Jul 2009
  by Tom Igoe
  modified 22 Nov 2010
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/LiquidCrystalAutoscroll

*/

// include the library code:
#include <stdio.h>
#include <string.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <DS3231.h>

#define BUFF_MAX 128

void timeStamp();

// Init the DS3231 using the hardware interface
DS3231 rtc;

// Init a Time-data structure
RTCDateTime t;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int button1Pin = 6;         // pushbutton 1 pin
const int button2Pin = 7;         // pushbutton 2 pin for correction
const int ledPin = 13;            // LED pin
const int motorPin = 10;          // Motor Pin
unsigned long buttonPushedMillis; // when button was released
unsigned long autoMillis;         // when the auto feed last ran
bool manualFeed = false;
bool ledOn = false;
bool buttonPushed = false;
bool button2Pushed = false;
bool autoFeed = false;
const int durMilli = 8100;
const int CorrectionMilli = 100;

void setup()
{
    // Setup Serial connection
    Serial.begin(9600);

    // Initialize the rtc object
    rtc.begin();

    // Manual (YYYY, MM, DD, HH, II, SS uncomment to set date
    //rtc.setDateTime(2018, 04, 29, 20, 15, 00);

    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);

    // Set up the pushbutton pins to be an input:
    pinMode(button1Pin, INPUT);

    // Set up the LED pin to be an output:
    pinMode(ledPin, OUTPUT);

    // set up the motor pin to be an output:
    pinMode(motorPin, OUTPUT);

    // clear alarm
    // rtc.armAlarm1(false);
    // rtc.clearAlarm1();
    // rtc.armAlarm2(false);
    // rtc.clearAlarm2();

    // setAlarm1(Date or Day, Hour, Minute, Second, Mode, Armed = true)
    rtc.setAlarm1(0, 06, 00, 00, DS3231_MATCH_H_M_S);
    rtc.setAlarm2(0, 18, 00, DS3231_MATCH_H_M);

    // Check alarm settings
    checkAlarms();
}

void loop()
{
    // Get data from the DS3231
    t = rtc.getDateTime();

    char buff[BUFF_MAX];

    // snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", t.year, t.mon, t.mday, t.hour, t.min, t.sec);

    // lcd.print(buff);

    // get current millis to track initial time
    unsigned long currentMillis = millis();

    // set the cursor to (0,0):
    lcd.setCursor(0, 0);

    // string representation of time
    lcd.print(rtc.dateFormat("M d  h:i:s", t));

    // start on second row
    lcd.setCursor(0, 1);

    // display time fed and how
    lcd.print("BLF: ");

    // variables to hold the pushbutton states
    int button1State = digitalRead(button1Pin);
    int button2State = digitalRead(button2Pin);
  /*if(millis() % 1000 >= 0 && manualFeed == true)
  {
    Serial.println("manualFeed == true");
  }
  else if(millis() % 1000 >= 0 && manualFeed == false)
  {
    Serial.println("manualFeed == false");
  }*/

  //Correction Button
  
  if(button2State == LOW) 
  {
    button2Pushed = true;
    buttonPushedMillis = millis();
    digitalWrite(ledPin, HIGH);
    feed();
  }
    if(buttonPushedMillis + CorrectionMilli < millis() && button2Pushed == true)
    {
      digitalWrite(ledPin, LOW);
      digitalWrite(motorPin, LOW);
      button2Pushed = false;
    }

  //Manual Feed Button
  if(button1State == LOW) 
  {
    //song();
    buttonPushedMillis = millis();
    manualFeed = true;
    ledOn = true;
    buttonPushed = true;
    digitalWrite(ledPin, HIGH);
    timeStamp();
    lcd.print(" (B)");
    feed();
    }

    if(buttonPushedMillis + durMilli < millis() && buttonPushed == true)
  {
    digitalWrite(ledPin, LOW);
    digitalWrite(motorPin, LOW);
    buttonPushed = false;
    }
  if((buttonPushedMillis + hours(4)) < millis() && manualFeed == true)
  {
    manualFeed = false;
  }

    // Either alarm is ran as long as checkLastFed with 4 hours since last button press is confirmed
    // Prevents auto from running if manual was executed withing 4 hours.
    if((rtc.isAlarm1() || rtc.isAlarm2()) && manualFeed == false) 
  {
    song();
    autoMillis = millis();
    timeStamp();
    lcd.print(" (A)");
    feed();
    digitalWrite(ledPin, HIGH);
    autoFeed = true;
  }
  
    if(autoMillis + durMilli < millis() && autoFeed == true) 
  {
    digitalWrite(ledPin, LOW);
    digitalWrite(motorPin, LOW);
    autoFeed = false;
    }
}

void timeStamp()
{
    lcd.print(rtc.dateFormat("h:ia", t));
}

void feed()
{
    int speed = 255;
    analogWrite(motorPin, speed);
}

void checkAlarms()
{
    RTCAlarmTime a1;
    RTCAlarmTime a2;

    if(rtc.isArmed1()) {
  a1 = rtc.getAlarm1();

  Serial.print("Alarm1 is triggered ");
  switch(rtc.getAlarmType1()) {
  case DS3231_EVERY_SECOND:
      Serial.println("every second");
      break;
  case DS3231_MATCH_S:
      Serial.print("when seconds match: ");
      Serial.println(rtc.dateFormat("__ __:__:s", a1));
      break;
  case DS3231_MATCH_M_S:
      Serial.print("when minutes and sencods match: ");
      Serial.println(rtc.dateFormat("__ __:i:s", a1));
      break;
  case DS3231_MATCH_H_M_S:
      Serial.print("when hours, minutes and seconds match: ");
      Serial.println(rtc.dateFormat("__ H:i:s", a1));
      break;
  case DS3231_MATCH_DT_H_M_S:
      Serial.print("when date, hours, minutes and seconds match: ");
      Serial.println(rtc.dateFormat("d H:i:s", a1));
      break;
  case DS3231_MATCH_DY_H_M_S:
      Serial.print("when day of week, hours, minutes and seconds match: ");
      Serial.println(rtc.dateFormat("l H:i:s", a1));
      break;
  default:
      Serial.println("UNKNOWN RULE");
      break;
  }
    } else {
  Serial.println("Alarm1 is disarmed.");
    }

    if(rtc.isArmed2()) {
  a2 = rtc.getAlarm2();

  Serial.print("Alarm2 is triggered ");
  switch(rtc.getAlarmType2()) {
  case DS3231_EVERY_MINUTE:
      Serial.println("every minute");
      break;
  case DS3231_MATCH_M:
      Serial.print("when minutes match: ");
      Serial.println(rtc.dateFormat("__ __:i:s", a2));
      break;
  case DS3231_MATCH_H_M:
      Serial.print("when hours and minutes match:");
      Serial.println(rtc.dateFormat("__ H:i:s", a2));
      break;
  case DS3231_MATCH_DT_H_M:
      Serial.print("when date, hours and minutes match: ");
      Serial.println(rtc.dateFormat("d H:i:s", a2));
      break;
  case DS3231_MATCH_DY_H_M:
      Serial.println("when day of week, hours and minutes match: ");
      Serial.print(rtc.dateFormat("l H:i:s", a2));
      break;
  default:
      Serial.println("UNKNOWN RULE");
      break;
  }
    } else {
  Serial.println("Alarm2 is disarmed.");
    }
}

void song()
{

    const int buzzerPin = 9;
    pinMode(buzzerPin, OUTPUT);
    // We'll set up an array with the notes we want to play
    // change these values to make different songs!

    // Length must equal the total number of notes and spaces

    const int songLength = 18;

    // Notes is an array of text characters corresponding to the notes
    // in your song. A space represents a rest (no tone)

    char notes[] = "cdfda ag cdfdg gf "; // a space represents a rest

    // Beats is an array values for each note and rest.
    // A "1" represents a quarter-note, 2 a half-note, etc.
    // Don't forget that the rests (spaces) need a length as well.

    int beats[] = { 1, 1, 1, 1, 1, 1, 4, 4, 2, 1, 1, 1, 1, 1, 1, 4, 4, 2 };

    // The tempo is how fast to play the song.
    // To make the song play faster, decrease this value.

    int tempo = 150;

    int i, duration;

    for(i = 0; i < songLength; i++) // step through the song arrays
    {
  duration = beats[i] * tempo; // length of note/rest in ms

  if(notes[i] == ' ') // is this a rest?
  {
      delay(duration); // then pause for a moment
  } else               // otherwise, play the note
  {
      tone(buzzerPin, frequency(notes[i]), duration);
      delay(duration); // wait for tone to finish
  }
  delay(tempo / 10); // brief pause between notes
    }

    // We only want to play the song once, so we'll pause forever:
    // while(true){}
    // If you'd like your song to play over and over,
    // remove the above statement
}

int frequency(char note)
{
    // This function takes a note character (a-g), and returns the
    // corresponding frequency in Hz for the tone() function.

    int i;
    const int numNotes = 8; // number of notes we're storing

    // The following arrays hold the note characters and their
    // corresponding frequencies. The last "C" note is uppercase
    // to separate it from the first lowercase "c". If you want to
    // add more notes, you'll need to use unique characters.

    // For the "char" (character) type, we put single characters
    // in single quotes.

    char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
    int frequencies[] = { 262, 294, 330, 349, 392, 440, 494, 523 };

    // Now we'll search through the letters in the array, and if
    // we find it, we'll return the frequency for that note.

    for(i = 0; i < numNotes; i++) // Step through the notes
    {
    if(names[i] == note) // Is this the one?
    {
      return (frequencies[i]); // Yes! Return the frequency
    }
    }
    return (0); // We looked through everything and didn't find it,
                // but we still need to return a value, so return 0.
}

unsigned long hours(unsigned long hr)
{
  return (hr * 60 * 60 * 1000);
}
