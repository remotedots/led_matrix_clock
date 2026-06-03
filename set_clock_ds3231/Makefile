ARDUINO_DIR  = /usr/share/arduino

ARDUINO_SKETCHBOOK = $(HOME)/playground

BOARD_TAG    = nano328
ARDUINO_PORT = /dev/ttyUSB0

CPPFLAGS += \
  -DUTC_YEAR=$(shell date -u +%Y) \
  -DUTC_MONTH=$(shell date -u +%-m) \
  -DUTC_DAY=$(shell date -u +%-d) \
  -DUTC_HOUR=$(shell date -u +%-H) \
  -DUTC_MIN=$(shell date -u +%-M) \
  -DUTC_SEC=$(shell date -u +%-S)

include /usr/share/arduino/Arduino.mk
