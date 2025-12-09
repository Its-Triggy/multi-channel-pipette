#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

const byte ENCODER_A_PIN = 0;   // encoder A
const byte ENCODER_B_PIN = 1;   // encoder B
const byte SWITCH_PIN    = A1;  // encoder push switch

int8_t state = 0;
long index = 0;
int8_t index_items = 4;
long volume = 10;
bool volMode = false;
static int lastIndex = -1;
static int lastVolume = -1;
static bool lastMode = false;


void setup() {
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,1);
  lcd.print("START");

  pinMode(ENCODER_A_PIN, INPUT_PULLUP);
  pinMode(ENCODER_B_PIN, INPUT_PULLUP);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  initMotors();
}

void loop() {
  int8_t state = 0;
  if (rotaryEncoder(state)) {
    if (index == 0){
      volMode = !volMode;
    } else if (index == 1){
      lcd.setCursor(0,1);
      lcd.print("> Aspirating...");
      lcd.setCursor(0,1);
      aspirate(volume);
      lcd.print("> Aspirate DONE ");
    }
    else if (index == 2){
      lcd.setCursor(0,2);
      lcd.print("> Dispensing...");
      dispense(volume);
      lcd.setCursor(0,2);
      lcd.print("> Dispense DONE ");
    }
    else if (index == 3){
      lcd.setCursor(0,3);
      lcd.print("> Recalibrating...");
      bool success = homeMotors();
      lcd.setCursor(0,3);
      if (success){
        lcd.print("> Recalibrate DONE ");
      }
      else{
        lcd.print("> Recalibrate FAIL ");
      }
    }
  }
  
  if (state == -1) {
    if (volMode){
      volume-=10;
      if (volume < 10){
        volume = 10;
      }
    }
    else {
      index--;
      if (index < 0) {
        index = index_items-1;
      }
      index = index % index_items;
    }
  }
  if (state == 1) {
    if (volMode){
      volume+=10;
      if (volume > 990){
        volume = 990;
      }
    }
    else {
      index++;
      index = index % index_items;
    }
  }
  
  if (index != lastIndex || volume != lastVolume || volMode != lastMode) {
    lcd.setCursor(0,0);
    if (index == 0 && !volMode){
    lcd.print("> Volume ");
    }
    else{
      lcd.print("  Volume ");
    }
    if (volMode) lcd.print("> "); else lcd.print("  ");
    char buf[8];
    snprintf(buf, sizeof(buf), "%-3d", volume);
    lcd.print(buf);
    lcd.print("uL");
    if (volMode){
      lcd.print(" OK?");
    }
    else{
      lcd.print("     ");
    }
    lastVolume = volume;
    lastMode = volMode;
  }
  

  if (index != lastIndex) {
    if (index == 1) {
      lcd.setCursor(0,1);
      lcd.print("> Aspirate      ");
      lcd.setCursor(0,2);
      lcd.print("  Dispense      ");
      lcd.setCursor(0,3);
      lcd.print("  Recalibrate      ");
    } else if (index == 2) {
      lcd.setCursor(0,1);
      lcd.print("  Aspirate      ");
      lcd.setCursor(0,2);
      lcd.print("> Dispense      ");
      lcd.setCursor(0,3);
      lcd.print("  Recalibrate      ");
    } else if (index == 3) {
      lcd.setCursor(0,1);
      lcd.print("  Aspirate      ");
      lcd.setCursor(0,2);
      lcd.print("  Dispense      ");
      lcd.setCursor(0,3);
      lcd.print("> Recalibrate      ");
    } else {
      lcd.setCursor(0,1);
      lcd.print("  Aspirate      ");
      lcd.setCursor(0,2);
      lcd.print("  Dispense      ");
      lcd.setCursor(0,3);
      lcd.print("  Recalibrate      ");
    }
    lastIndex = index;
  }
}


bool rotaryEncoder(int8_t &delta) {
  delta = 0;
  enum {STATE_LOCKED, STATE_TURN_RIGHT_START, STATE_TURN_RIGHT_MIDDLE, STATE_TURN_RIGHT_END,
        STATE_TURN_LEFT_START, STATE_TURN_LEFT_MIDDLE, STATE_TURN_LEFT_END, STATE_UNDECIDED};
  static uint8_t encoderState = STATE_LOCKED;

  bool a = !digitalRead(ENCODER_A_PIN);
  bool b = !digitalRead(ENCODER_B_PIN);
  bool s = !digitalRead(SWITCH_PIN);

  static bool switchState = s;

  switch (encoderState) {
    case STATE_LOCKED:
      if (a && b) encoderState = STATE_UNDECIDED;
      else if (!a && b) encoderState = STATE_TURN_LEFT_START;
      else if (a && !b) encoderState = STATE_TURN_RIGHT_START;
      else encoderState = STATE_LOCKED;
      break;

    case STATE_TURN_RIGHT_START:
      if (a && b) encoderState = STATE_TURN_RIGHT_MIDDLE;
      else if (!a && b) encoderState = STATE_TURN_RIGHT_END;
      else if (a && !b) encoderState = STATE_TURN_RIGHT_START;
      else { encoderState = STATE_LOCKED; }
      break;

    case STATE_TURN_RIGHT_MIDDLE:
    case STATE_TURN_RIGHT_END:
      if (a && b) encoderState = STATE_TURN_RIGHT_MIDDLE;
      else if (!a && b) encoderState = STATE_TURN_RIGHT_END;
      else if (a && !b) encoderState = STATE_TURN_RIGHT_START;
      else { encoderState = STATE_LOCKED; delta = -1; }
      break;

    case STATE_TURN_LEFT_START:
      if (a && b) encoderState = STATE_TURN_LEFT_MIDDLE;
      else if (!a && b) encoderState = STATE_TURN_LEFT_START;
      else if (a && !b) encoderState = STATE_TURN_LEFT_END;
      else { encoderState = STATE_LOCKED; }
      break;

    case STATE_TURN_LEFT_MIDDLE:
    case STATE_TURN_LEFT_END:
      if (a && b) encoderState = STATE_TURN_LEFT_MIDDLE;
      else if (!a && b) encoderState = STATE_TURN_LEFT_START;
      else if (a && !b) encoderState = STATE_TURN_LEFT_END;
      else { encoderState = STATE_LOCKED; delta = 1; }
      break;

    case STATE_UNDECIDED:
      if (a && b) encoderState = STATE_UNDECIDED;
      else if (!a && b) encoderState = STATE_TURN_RIGHT_END;
      else if (a && !b) encoderState = STATE_TURN_LEFT_END;
      else encoderState = STATE_LOCKED;
      break;
  }

  // simple debounce for switch
  uint32_t current_time = millis();
  static uint32_t switch_time = 0;
  const uint32_t bounce_time = 30;
  bool back = false;
  if (current_time - switch_time >= bounce_time) {
    if (switchState != s) {
      switch_time = current_time;
      back = s;
      switchState = s;
    }
  }
  return back;
}

void aspirate(int ul){
  char motorsToRun[] = {'X','Y','Z','A'};
  runMotors(ul, LOW, motorsToRun, 4);
}

void dispense(int ul){
  char motorsToRun[] = {'X','Y','Z','A'};
  runMotors(ul, HIGH, motorsToRun, 4);
}
