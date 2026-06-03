#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>

RTC_DS3231 rtc;

const uint8_t EEPROM_MAGIC      = 0xAB;
const int     EEPROM_ADDR_MAGIC = 0;
const int     EEPROM_ADDR_TZ    = 1;

void printDateTime(const DateTime& dt) {
  char buf[20];
  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
    dt.year(), dt.month(), dt.day(),
    dt.hour(), dt.minute(), dt.second());
  Serial.print(buf);
}

void setup() {
  Serial.begin(9600);
  delay(2000);

  if (!rtc.begin()) {
    Serial.println("ERROR: DS3231 not found. Check wiring.");
    while (1);
  }

  // __DATE__ / __TIME__ are set by the compiler to local time at compile time.
  // Upload promptly to minimise drift (typically 30-60 s).
  DateTime localTime(F(__DATE__), F(__TIME__));

  Serial.println("--- DS3231 Clock Setup ---");
  Serial.print("Compile time (local): ");
  printDateTime(localTime);
  Serial.println();
  Serial.println();
  Serial.println("Enter your UTC offset and press Enter.");
  Serial.println("Examples: 2 for UTC+2, -5 for UTC-5");
  Serial.print("> ");

  while (Serial.available() == 0) {}
  int8_t tz = (int8_t)Serial.parseInt();

  // UTC = local time - offset
  DateTime utcTime = localTime - TimeSpan((int32_t)tz * 3600);

  Serial.println(tz);
  Serial.println();
  Serial.print("Setting RTC to UTC:      ");
  printDateTime(utcTime);
  Serial.println();
  Serial.print("Your local time (UTC");
  if (tz >= 0) Serial.print("+");
  Serial.print(tz);
  Serial.print("):  ");
  printDateTime(localTime);
  Serial.println();

  rtc.adjust(utcTime);
  EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
  EEPROM.write(EEPROM_ADDR_TZ, (uint8_t)tz);

  Serial.println();
  Serial.println("Done. You can now upload the clock sketch.");
}

void loop() {
  DateTime now = rtc.now();
  Serial.print("RTC now (UTC): ");
  printDateTime(now);
  Serial.println();
  delay(5000);
}
