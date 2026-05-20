const int BUZZER_PIN = 8;
const int LED_PIN    = 13;

const int DOT        = 150;
const int DASH       = 450;
const int SYMBOL_GAP = 150;
const int WORD_GAP   = 1000;

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("Arduino Morse Ready.");

  beep(DOT); delay(SYMBOL_GAP);
  beep(DOT); delay(SYMBOL_GAP);
  beep(DOT);
}

void loop() {
  if (Serial.available()) {
    char code = Serial.read();

    if (code == '\n' || code == '\r') return;

    Serial.print("Received: ");
    Serial.println(code);

    switch (code) {
      case 'P':
        Serial.println("PERSON DETECTED → · - - ·");
        morsePerson();
        delay(WORD_GAP);
        break;

      case 'M':
        Serial.println("MULTIPLE PEOPLE → - -");
        morseMultiple();
        delay(WORD_GAP);
        break;

      case 'U':
        Serial.println("UNKNOWN OBJECT → · · -");
        morseUnknown();
        delay(WORD_GAP);
        break;

      case 'C':
        Serial.println("CLEAR — silent");
        digitalWrite(LED_PIN, LOW);
        noTone(BUZZER_PIN);
        break;

      default:
        break;
    }
  }
}


void morsePerson() {
  dot(); gap(); dash(); gap(); dash(); gap(); dot();
}

void morseMultiple() {
  dash(); gap(); dash();
}

void morseUnknown() {
  dot(); gap(); dot(); gap(); dash();
}


void dot() {
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1000, DOT);
  delay(DOT);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);
}

void dash() {
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1000, DASH);
  delay(DASH);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);
}

void gap() {
  delay(SYMBOL_GAP);
}

void beep(int duration) {
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1000, duration);
  delay(duration);
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);
}
