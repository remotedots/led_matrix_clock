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

// European DST: last Sunday of March 01:00 UTC to last Sunday of October 01:00 UTC.
// Pass UTC time and the standard (winter) offset.
bool isSummerTime(int year, int month, int day, int hour, int baseTz) {
  if (month < 3 || month > 10) return false;
  if (month > 3 && month < 10) return true;
  if (month == 3)
    return (hour + 24 * day) >= (1 + baseTz + 24 * (31 - (5 * year / 4 + 4) % 7));
  return (hour + 24 * day) < (1 + baseTz + 24 * (31 - (5 * year / 4 + 1) % 7));
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
  Serial.println("Enter your current UTC offset and press Enter.");
  Serial.println("Examples: 2 for CEST, 1 for CET, -5 for EST");
  Serial.print("> ");

  while (Serial.available() == 0) {}
  int8_t localOffset = (int8_t)Serial.parseInt();

  // Compute UTC from local time
  DateTime utcTime = localTime - TimeSpan((int32_t)localOffset * 3600);

  // Strip DST to get the base (standard/winter) timezone for EEPROM.
  // The clock sketch applies DST automatically on top of the base offset.
  int8_t baseTz = localOffset;
  if (isSummerTime(utcTime.year(), utcTime.month(), utcTime.day(), utcTime.hour(), localOffset - 1))
    baseTz = localOffset - 1;

  Serial.println(localOffset);
  Serial.println();
  Serial.print("UTC saved to RTC:     ");
  printDateTime(utcTime);
  Serial.println();
  Serial.print("Base timezone saved:  UTC");
  if (baseTz >= 0) Serial.print("+");
  Serial.println(baseTz);
  Serial.println();
  Serial.println("Done. You can now upload the clock sketch.");

  rtc.adjust(utcTime);
  EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
  EEPROM.write(EEPROM_ADDR_TZ, (uint8_t)baseTz);
}

void loop() {
  DateTime now = rtc.now();
  Serial.print("RTC now (UTC): ");
  printDateTime(now);
  Serial.println();
  delay(5000);
}
