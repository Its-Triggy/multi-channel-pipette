const int enPin     = 8;

const int stepXPin  = 2;
const int dirXPin   = 5;
const int stepYPin  = 3;
const int dirYPin   = 6;
const int stepZPin  = 4;
const int dirZPin   = 7;
const int stepAPin  = 12;
const int dirAPin   = 13;
const int limXPin = 9;
const int limYPin = 10;
const int limZPin = 11;
const int limAPin = A3;

const unsigned long stepsPer_uL = 12;
unsigned int pulseWidthMicros   = 100;   // µs HIGH time
unsigned int microsBtwnSteps    = 1000;  // µs LOW time

void initMotors() {
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);

  pinMode(stepXPin, OUTPUT);
  pinMode(dirXPin, OUTPUT);
  pinMode(stepYPin, OUTPUT);
  pinMode(dirYPin, OUTPUT);
  pinMode(stepZPin, OUTPUT);
  pinMode(dirZPin, OUTPUT);
  pinMode(stepAPin, OUTPUT);
  pinMode(dirAPin, OUTPUT);

  pinMode(limXPin, INPUT_PULLUP);
  pinMode(limYPin, INPUT_PULLUP);
  pinMode(limZPin, INPUT_PULLUP);
  pinMode(limAPin, INPUT_PULLUP);
}

void setDir(const char motors[], int listLength, int state) {
  for (int i = 0; i < listLength; i++) {
    switch (motors[i]) {
      case 'X': digitalWrite(dirXPin, state); break;
      case 'Y': digitalWrite(dirYPin, state); break;
      case 'Z': digitalWrite(dirZPin, state); break;
      case 'A': digitalWrite(dirAPin, state); break;
    }
  }
}

void setStep(const char motors[], int listLength, int state) {
  for (int i = 0; i < listLength; i++) {
    switch (motors[i]) {
      case 'X': digitalWrite(stepXPin, state); break;
      case 'Y': digitalWrite(stepYPin, state); break;
      case 'Z': digitalWrite(stepZPin, state); break;
      case 'A': digitalWrite(stepAPin, state); break;
    }
  }
}

void runMotors(int uL, int dir, const char motors[], int listLength) {
  setDir(motors, listLength, dir);
  unsigned long totalSteps = (unsigned long)uL * stepsPer_uL;
  for (unsigned long i = 0; i < totalSteps; i++) {
    setStep(motors, listLength, HIGH);
    delayMicroseconds(pulseWidthMicros);
    setStep(motors, listLength, LOW);
    delayMicroseconds(microsBtwnSteps);

    if (dir==HIGH){ // Watch for limits if dispensing
      bool xHit = (digitalRead(limXPin) == LOW);
      bool yHit = (digitalRead(limYPin) == LOW);
      bool zHit = (digitalRead(limZPin) == LOW);
      bool aHit = (digitalRead(limAPin) == LOW);

      if (xHit || yHit || zHit || aHit) {
        return;
      }
    }
  }
}

bool homeMotors() {
  digitalWrite(dirXPin, HIGH);
  digitalWrite(dirYPin, HIGH);
  digitalWrite(dirZPin, HIGH);
  digitalWrite(dirAPin, HIGH);

  bool xDone = false;
  bool yDone = false;
  bool zDone = false;
  bool aDone = false;

  unsigned long triggerTime = 0;
  bool anyTriggered = false;

  const unsigned long maxSteps = 10000;
  unsigned long stepCount = 0;

  while (!(xDone && yDone && zDone && aDone)) {
    // Step all that are not yet homed
    if (!xDone) digitalWrite(stepXPin, HIGH);
    if (!yDone) digitalWrite(stepYPin, HIGH);
    if (!zDone) digitalWrite(stepZPin, HIGH);
    if (!aDone) digitalWrite(stepAPin, HIGH);
    delayMicroseconds(pulseWidthMicros);

    if (!xDone) digitalWrite(stepXPin, LOW);
    if (!yDone) digitalWrite(stepYPin, LOW);
    if (!zDone) digitalWrite(stepZPin, LOW);
    if (!aDone) digitalWrite(stepAPin, LOW);
    delayMicroseconds(microsBtwnSteps);

    stepCount++;

    bool xHit = (digitalRead(limXPin) == LOW);
    bool yHit = (digitalRead(limYPin) == LOW);
    bool zHit = (digitalRead(limZPin) == LOW);
    bool aHit = (digitalRead(limAPin) == LOW);

    if (!xDone && xHit) xDone = true;
    if (!yDone && yHit) yDone = true;
    if (!zDone && zHit) zDone = true;
    if (!aDone && aHit) aDone = true;

    if (!anyTriggered && (xHit || yHit || zHit || aHit)) {
      anyTriggered = true;
      triggerTime = millis();
      return true; // Return true if any is triggered. Waiting for all 4 has issues.
    }

    // Safety 1: if one triggered but others don’t finish soon enough
    if (anyTriggered && millis() - triggerTime > 2500) {
      return false;
    }

    // Safety 2: if too many steps and still not all homed
    if (stepCount > maxSteps) {
      return false;
    }
  }

  return true; // homing successful
}

// char motorsToRun[] = {'X','Y'};
// runMotors(10, HIGH, motorsToRun, 2);
