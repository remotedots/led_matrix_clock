#include "libCharacters.h"
#include <Time.h>
#include <RTClib.h>
#include <Arduino.h>
#include <Wire.h>

/**
 *  A0 - pin 13 (row8)
 *  A1 - pin  3 (row7)
 *  A2 - pin  4 (row6)
 *  A3 - pin 10 (row5)
 *  A4 - RTC (SDA)
 *  A5 - RTC (SCL)
 *   1 - pin  5 (col1)
 *   2 - pin  2 (col2)
 *   3 - pin  7 (col3)
 *   4 - pin  1 (col4)
 *   5 - pin 12 (col5)
 *   6 - pin  8 (col6)
 *   7 - pin 14 (col7)
 *   8 - pin  9 (col1)
 *   9 - pin 15 (row2)
 *  10 - pin 16 (row8)
 *  11 - pin  6 (row4)
 *  12 - pin 11 (row3)
 *
 *               
 *               ------__------__------
 *               |                    |
 *      4 <- (col4) o 1         16 o (row1) -> 10
 *      2 <- (col2) o 2         15 o (row2) ->  9
 *     A1 <- (row7) o 3         14 o (col7) ->  7
 *     A2 <- (row6) o 4         13 o (row8) -> A0
 *     13 <- (col1) o 5         12 o (col5) ->  5
 *     11 <- (row4) o 6         11 o (row3) -> 12
 *      3 <- (col3) o 7         10 o (row5) -> A3
 *      6 <- (col6) o 8          9 o (col8) ->  8
 *               |                    |
 *               ------__------__------
 *                      top view
 */

int rows[] = {A0, A1, A2, A3, 11, 12,  9, 10};
int cols[] = {13,  2,  3,  4,  5,  6,  7,  8};

const int MATRIX_H         =   8;
const int MATRIX_W         =   8;
const int MATRIX_LED_COUNT =   MATRIX_H * MATRIX_W;

const int BUFFER_W         =  27;
const int BUFFER_H         =   8;
const int LED_STATE_OFF    =   0;
const int LED_STATE_ON     =   1;

const int REFRESH_RATE     = 200;

time_t currentTime;
String currentTimeStr;

long int bufferTimer;

int leds[MATRIX_W][MATRIX_H]    = {0};
int buffer[BUFFER_W][BUFFER_H]  = {0};
int bufferWidth  = 0;
int bufferOffset = 0;
int ledOffset    = 7;

RTC_DS3231 rtc;

void initPins() {

  for (int i = 0; i < MATRIX_H; i++) {
    pinMode(rows[i], OUTPUT);
    digitalWrite(rows[i], HIGH);
  }
  for (int i = 0; i < MATRIX_W; i++) {
    pinMode(cols[i], OUTPUT);
    digitalWrite(cols[i], LOW);
  }
}


int ledOn(int col, int row) {

  digitalWrite(cols[col], HIGH);
  digitalWrite(rows[row], LOW);
}


int ledOff(int col, int row) {

  digitalWrite(cols[col], LOW);
  digitalWrite(rows[row], HIGH);
}


void initDisplay() {

  for (int mil = 0; mil < 50; mil++) {

    for (int i = 0; i < MATRIX_W; i++)
      for (int j = 0; j < MATRIX_H; j++)
        ledOn(i, j);
    for (int i = 0; i < MATRIX_W; i++)
      for (int j = 0; j < MATRIX_H; j++)
        ledOff(i, j);

    delay(20);
  }
}


void initTimers() {

  bufferTimer = millis();
}


void initTime(int h = 23, int m = 43, int s = 45) {

  if (rtc.begin()) {
    DateTime curTime = rtc.now();
    h = curTime.hour();
    m = curTime.minute();
    s = curTime.second();
  }

  setTime(h, m, s, 1, 1, 2000);
  adjustTime(-3600);

}


int *getChar(int charCode) {

  int data[LETTER_W][LETTER_H];
  for (int i = 0; i < LETTER_DOT_COUNT; i++)
    data[i / LETTER_W][i % LETTER_W] = letter[charCode][i / LETTER_W][i % LETTER_W];
//  return data;
}


void charToBuffer(int data[LETTER_W][LETTER_H], int offsetX, int offsetY = 0) {

  int k = 0;
  for (int i = 0; i < LETTER_W; i++) {

    k = 0;
    for (int j = 0; j < LETTER_H; j++)
      if (data[i][j] == 0)
        k++;

    if (k < LETTER_H) {
      for (int j = 0; j < LETTER_H; j++)
//        buffer[i + offsetX][j + offsetY] = data[i][j];
        buffer[bufferWidth][j + offsetY] = data[i][j];
      bufferWidth++;
    }
  }
  bufferWidth++;
}


void writeToBuffer(String s) {

  int temp[LETTER_W][LETTER_H];

  bufferWidth = 0;

  for (int k = 0; k < s.length(); k++) {

    for (int i = 0; i < LETTER_W; i++)
      for (int j = 0; j < LETTER_H; j++)
        temp[i][j] = numbers[int(s[k]) - 47][j][i];

    charToBuffer(temp, k * (LETTER_W + 1), 2);
  }
}


void flushBuffer() {

  for (int i = 0; i < MATRIX_LED_COUNT; i++)
    leds[i / MATRIX_H][i % MATRIX_W] = 0;
}


void drawBuffer() {

  int startPoint = bufferOffset * MATRIX_H;
  int stopPoint;

  if (BUFFER_W - bufferOffset >= MATRIX_W)
    stopPoint = bufferOffset * MATRIX_H - ledOffset * MATRIX_H + MATRIX_LED_COUNT;
  else
    stopPoint = bufferOffset * MATRIX_H - ledOffset * MATRIX_H + (BUFFER_W - bufferOffset) * MATRIX_H;

  for (int i = startPoint; i < stopPoint; i++) {
    if (buffer[i / 8][i % 8] == 1)
      ledOn((i - startPoint) / 8 + ledOffset, (i - startPoint) % 8);
    ledOff((i - startPoint) / 8 + ledOffset, (i - startPoint) % 8);
  }
}


void scrollBuffer() {

  if (bufferWidth > MATRIX_W) {
    //    scrollBuffer
    if (millis() - bufferTimer > REFRESH_RATE) {
      if (ledOffset > 0)
        ledOffset--;
      else
        if (bufferOffset < bufferWidth)
          bufferOffset++;
        else {
          bufferOffset = 0;
          ledOffset = 7;
        }
      bufferTimer = millis();
    }
  }
  else
    ledOffset = 0;
}


time_t getTime() {

  DateTime curTime = now();
  if ((curTime.hour() == 0) && (curTime.minute() == 0) && (curTime.second() == 0)) {
    initTime();
    ledOn(0, 0);
  }
  return now();
}


String timeToStr(time_t t) {

  char sTime[50];
  sprintf(sTime, "%02d:%02d", hour(t), minute(t));
//  sprintf(sTime, "%02d", second(t));
  return String(sTime);
}


String getStringFromSerial() {


}


void setup() {

//  Serial.begin(9600);
  initPins();
  initTime();
  initTimers();
  initDisplay();
}


void loop() {

  currentTime = getTime();
  currentTimeStr = timeToStr(currentTime);
  writeToBuffer(currentTimeStr);
  scrollBuffer();
  drawBuffer();
}
