#include <LiquidCrystal.h>

//morse keyboard switches (mksw)
const int mksw_dot = 6, mksw_dash = 5, mksw_enter = 4, mksw_space = 3, mksw_clr = 2;

//Assigning LDR, buzzer+led, and RESET pins
const int LDR_VALUE = A0, Reset = 7, BuzLed = A1;

//lcd display pin assignment
const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 = 9, d7 = 8;

//initializing lcd
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//checking variables
bool received_on = 0, timeout = 0;

//timer variables
unsigned long t1, t2, on_duration = 0, off_duration = 0, timeout_initial = 0, timeout_check = 0;

//alphabets
const String m_letters[26] = { ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.." };

const char a_letters[26] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

//Forward Declaration of functions
int custom_print(bool letterSpace, bool wordSpace, bool timeOut, bool dot, bool dash, bool clear_morse);
void input_select();
void play();

void setup() {

  digitalWrite(Reset, HIGH);

  pinMode(Reset, OUTPUT);

  pinMode(BuzLed, OUTPUT);

  Serial.begin(9600);

  lcd.begin(16, 2);

  delay(200);
}

void loop() {

  input_select();
}

//state machine
void input_select() {
  enum class input_state : uint8_t {
    IDLE,
    LIGHT,
    SWITCH,
  };

  static input_state currState = input_state::IDLE;

  switch (currState) {
    case input_state::IDLE:
      lcd.print("SELECT INPUT.");

      delay(50);

      lcd.clear();

      if (digitalRead(mksw_dot) == HIGH || digitalRead(mksw_dash) == HIGH || digitalRead(mksw_enter) == HIGH || digitalRead(mksw_space) == HIGH || digitalRead(mksw_clr) == HIGH) {
        currState = input_state::SWITCH;
        lcd.print("MODE: SWITCH");
        lcd.setCursor(0, 1);
        while (digitalRead(mksw_dot) == HIGH || digitalRead(mksw_dash) == HIGH || digitalRead(mksw_enter) == HIGH || digitalRead(mksw_space) == HIGH || digitalRead(mksw_clr) == HIGH) {};  //wait until "select" signal dies
        lcd.clear();
        timeout_initial = millis();
      } else if (analogRead(LDR_VALUE) >= 350) {
        currState = input_state::LIGHT;
        lcd.print("MODE: LIGHT");
        lcd.setCursor(0, 1);
        while (analogRead(LDR_VALUE) >= 350) {}  //wait until "select" signal dies
        lcd.clear();
      }
      break;

    case input_state::SWITCH:
      if (digitalRead(mksw_enter) == HIGH) {
        while (digitalRead(mksw_enter) == HIGH) {}
        custom_print(1, 0, 0, 0, 0, 0);
        timeout_initial = millis();
      } else if (digitalRead(mksw_space) == HIGH) {
        while (digitalRead(mksw_space) == HIGH) {}
        custom_print(0, 1, 0, 0, 0, 0);
        timeout_initial = millis();
      } else if (digitalRead(mksw_dot) == HIGH) {
        while (digitalRead(mksw_dot) == HIGH) {}
        custom_print(0, 0, 0, 1, 0, 0);
        timeout_initial = millis();
      } else if (digitalRead(mksw_dash) == HIGH) {
        while (digitalRead(mksw_dash) == HIGH) {}
        custom_print(0, 0, 0, 0, 1, 0);
        timeout_initial = millis();
      } else if (digitalRead(mksw_clr) == HIGH) {
        while (digitalRead(mksw_clr) == HIGH) {}
        custom_print(0, 0, 0, 0, 0, 1);
        timeout_initial = millis();
      } else if (millis() - timeout_initial > 5000) {
        custom_print(0, 0, 1, 0, 0, 0);
      }
      break;

    case input_state::LIGHT:

      while (received_on == 0) {
        lcd.setCursor(0, 1);

        lcd.print("AWAITING SIGNAL");

        delay(25);

        lcd.clear();

        if (analogRead(LDR_VALUE) >= 350) {

          received_on = 1;

          lcd.clear();

          break;
        }
      }

      if (received_on) {

        t1 = millis();
      }

      if (analogRead(LDR_VALUE) >= 350) {

        while (analogRead(LDR_VALUE) >= 350) {}

        t2 = millis();

        on_duration = t2 - t1;

      } else {


        timeout_initial = millis();

        while (analogRead(LDR_VALUE) < 350) {

          timeout_check = millis();

          if (timeout_check - timeout_initial > 2100 && analogRead(LDR_VALUE) < 350) {

            timeout = 1;

            break;
          }
        }

        t2 = millis();

        off_duration = t2 - t1;
      }

      //depending on whether we received an on_duration, we decide

      switch (bool(on_duration)) {

        case 0:  // intending to add a space

          if (custom_print(0, 0, 0, 0, 0, 0) != 0) {  //no point adding a space if no input has arrived

            if (off_duration >= 850) {  //letter space

              custom_print(1, 0, 0, 0, 0, 0);
            }
            if (off_duration >= 2065) {  // word space

              custom_print(0, 1, 0, 0, 0, 0);
            }
            if (off_duration > 2100 && timeout == 1) {  //time out

              custom_print(0, 0, 1, 0, 0, 0);
            }
          } else {

            custom_print(0, 0, 0, 0, 0, 1);
            ;  //reset morse message
          }

          break;

        case 1:  //intenting to add a dot or dash

          if (custom_print(0, 0, 0, 0, 0, 0) != 4) {  // there is no string with length 5 in the morse alphabet

            if (on_duration <= 330 && on_duration >= 200) {  //dot

              custom_print(0, 0, 0, 1, 0, 0);

            } else if (on_duration <= 930 && on_duration >= 660) {  //dash

              custom_print(0, 0, 0, 0, 1, 0);
            }
          } else {

            custom_print(0, 0, 0, 0, 0, 1);  // reset morse message
          }

          break;
      }

      on_duration = 0;

      off_duration = 0;

      break;
  }
}

int custom_print(bool letterSpace, bool wordSpace, bool timeOut, bool dot, bool dash, bool clear_morse) {
  static String message = "", morse_code = "";

  if (letterSpace) {
    int i = 0;
    for (i = 0; i < 26; i++) {
      if (morse_code == m_letters[i]) {
        message += a_letters[i];
        morse_code = "";

        lcd.setCursor(0, 0);
        lcd.print(message);
        lcd.setCursor(0, 1);
        lcd.print("    ");
        break;
      }
    }
    if (i == 26) {
      morse_code = "";

      lcd.clear();
      lcd.print("LETTER NOT FOUND");
      delay(1000);
      lcd.clear();
    }
  } else if (wordSpace) {
    message += ' ';

    lcd.setCursor(0, 0);
    lcd.print(message);
  } else if (timeOut) {
    lcd.setCursor(0, 0);
    lcd.print(message);

    delay(1000);

    lcd.clear();
    lcd.print("! TIMEOUT !");
    delay(1000);
    lcd.clear();
    lcd.print("PLAYING MESSAGE");
    play(message);
    lcd.clear();
    for (int i = 5; i >= 0; i--) {
      lcd.print("RESTARTING IN ");
      lcd.print(i);
      delay(1000);
      lcd.clear();
    }
    digitalWrite(Reset, LOW);
  } else if (dot) {
    morse_code += '.';

    lcd.setCursor(0, 1);
    lcd.print(morse_code);
  } else if (dash) {
    morse_code += '-';

    lcd.setCursor(0, 1);
    lcd.print(morse_code);
  } else if (clear_morse) {
    message = "";
    morse_code = "";

    lcd.clear();
  }
  return morse_code.length();
}

void play(String message) {
  int break_case = 0;
  for (int i = 0; i < message.length(); i++) {
    for (int j = 0; j < 26; j++) {
      if (break_case == message.length()) {
        break;
      }
      if (message[i] == a_letters[j]) {
        for (int k = 0; k < m_letters[j].length(); k++) {
          if (m_letters[j][k] == '.') {
            digitalWrite(BuzLed, HIGH);
            delay(200);
            digitalWrite(BuzLed, LOW);
            delay(200);
          } else {
            digitalWrite(BuzLed, HIGH);
            delay(600);
            digitalWrite(BuzLed, LOW);
            delay(200);
          }
        }
        delay(600);
        break_case++;
      } else if (message[i] == ' ') {
        delay(1400);
        break_case++;
      }
    }
  }
}
