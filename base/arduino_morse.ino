/*
  Arduino Uno — Morse Code Output
  Receives single character from PC via Serial
  Outputs Morse code via Buzzer (pin 8) and LED (pin 13)

  ONLY beeps/flashes when person is detected:
    P = Person detected   (· - - ·)
    M = Multiple people   (- -)
    U = Unknown object    (· · -)
    C = Clear / no one    (silent - no beep)
*/

const int BUZZER_PIN = 8;
const int LED_PIN    = 13;

// Morse timing (ms)
const int DOT        = 150;
const int DASH       = 450;
const int SYMBOL_GAP = 150;
const int WORD_GAP   = 1000;

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("Arduino Morse Ready.");

  // Startup beep to confirm it's working
  beep(DOT); delay(SYMBOL_GAP);
  beep(DOT); delay(SYMBOL_GAP);
  beep(DOT);
}

void loop() {
  if (Serial.available()) {
    char code = Serial.read();

    // Ignore newline characters
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
        // CLEAR — no person, stay silent
        Serial.println("CLEAR — silent");
        digitalWrite(LED_PIN, LOW);
        noTone(BUZZER_PIN);
        break;

      default:
        break;
    }
  }
}

// --- Morse patterns ---

void morsePerson() {
  // P = · - - ·
  dot(); gap(); dash(); gap(); dash(); gap(); dot();
}

void morseMultiple() {
  // M = - -
  dash(); gap(); dash();
}

void morseUnknown() {
  // U = · · -
  dot(); gap(); dot(); gap(); dash();
}

// --- Primitives ---

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
