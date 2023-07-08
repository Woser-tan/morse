#include <LiquidCrystal.h>

const int rs=13, en=12, d4=11, d5=10, d6=9, d7=8, LDR_VALUE=A0, Reset=7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
unsigned int string_length=0, morse_length=0;
bool received_on=0, timeout=0;
unsigned long t1, t2, on_duration=0, off_duration=0, timeout_initial=0, timeout_check=0;
const String m_letters[26] = { ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.." };
const char a_letters[26] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

void custom_print(bool letterSpace, bool wordSpace, bool timeOut , bool dot, bool dash) {
  static int message_len=0, morse_len=0;
  static String message="", morse_code="";

  if ( letterSpace ) {
    int i=0;
    for ( i=0; i<26; i++ ) {
      if ( morse_code==m_letters[i] ) {
        //Serial.print(a_letters[i])
        lcd.print( a_letters[i] );
        delay( 50 );
        message+=a_letters[i];
      }
    }
    if ( i==26 ) {
      morse_code="";
    }
  } 
  else if ( wordSpace ) {
    int i=0;
    for ( i=0; i<26; i++ ) {
      if ( morse_code==m_letters[i] ) {
        lcd.print( a_letters[i] );
        delay( 50 );
        lcd.print( ' ' );
        delay( 50 );
        message+=a_letters[i];
        message+=' ';
      }
    }
    if ( i==26 ) {
      morse_code="";
    }
  }
  else if ( timeOut ) {
        int i = 0;
        for (i = 0; i < 26; i++) {
          if (morse_code == m_letters[i]) {
            lcd.print(a_letters[i]);
            delay(50);
            message += a_letters[i];
          }

        }if (i == 26) {
          morse_code = "";
        }
        lcd.print(message);
        delay(250);
        digitalWrite(Reset, LOW);
  } 
  else if ( dot ) {
    morse_code += '.';

  } 
  else if ( dash ) {
    morse_code += '-';
    
  } 
}

void setup() {

  digitalWrite(Reset, HIGH);

  pinMode(Reset, OUTPUT);

  Serial.begin(9600);

  lcd.begin(16, 2);

  delay(200);

}

void loop() {

  while (received_on == 0) {

    lcd.print("AWAITING SIGNAL");

    delay(100);

    lcd.clear();

    if (analogRead(LDR_VALUE) >= 350) {

      received_on = 1;

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

  } 
  else {


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

  //define what we expect to see in the morse code message e.g. '.' , '-' , '<-> (a letter space)' , '<--> (a word space)'

  switch (bool(on_duration)) {

    case 0:  // intending to add a space

      if (morse_code.length() != 0) {

        if (off_duration <= 890 && off_duration >= 850) {

          custom_print(1,0,0,0,0);
          
        } else if (off_duration <= 2100 && off_duration >= 2065) {

          custom_print(0,1,0,0,0);

        } else if (off_duration > 2100 && timeout == 1) {

          custom_print(0,0,1,0,0)

        }
      }
      else{

        morse_code = "";

      }

      break;

    case 1:  //intenting to add a dot or dash

      if (morse_code.length() != 4) {  // there is no string with length 5 in the morse alphabet

        if (on_duration <= 330 && on_duration >= 200) {

          custom_print(0,0,0,1,0);

        } 
        else if (on_duration <= 930 && on_duration >= 660) {

          custom_print(0,0,0,0,1);

        }
      } 
      else{

        morse_code = "";

      }

      break;

  }

  on_duration = 0;

  off_duration = 0;

}
