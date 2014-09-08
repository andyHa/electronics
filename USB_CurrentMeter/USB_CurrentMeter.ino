#include <LiquidCrystal.h>
#include <stdlib.h>

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  analogReference(INTERNAL);
  lcd.setCursor(15, 0);
  lcd.print("V");
  lcd.setCursor(14, 1);
  lcd.print("mA");
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  pinMode(0, INPUT);
  digitalWrite(0, HIGH);
}

boolean measuringMicroAmps = false;

void measureVoltage() {
  char buf[10];
  float volts = analogRead(5);
  if (digitalRead(0)  == HIGH) {
    volts = 0.0082*analogRead(5)-0.0389;
    dtostrf(volts, 4, 1, buf);
  } else {
    dtostrf(volts, 4 , 0, buf);
  }
  lcd.setCursor(11,0);
  lcd.print(buf);
}

void measureCurrent() {
  int current = analogRead(0);
  double value = 0;
  if (measuringMicroAmps) {
    if (current >= 900) {
      measuringMicroAmps = false;
      digitalWrite(9, HIGH);
      lcd.setCursor(14, 1);
      lcd.print("mA");
    } else {
      if (current < 5) {
        value = -1;
      } else {
        if (digitalRead(0)  == HIGH) {
          value = 4.5114 * current + 10.076;
         } else {
           value = current;
         }
      }
    }
  } else {
    if (current <= 20) {
      measuringMicroAmps = true;
      digitalWrite(9, LOW);
      lcd.setCursor(14, 1);
      lcd.print("uA");
    } else {
      if (current > 1022) {
        value = -2;
      } else {
          if (digitalRead(0)  == HIGH) {
            value = 0.2217 * current - 0.5292;
           } else {
             value = current;
           }
      }
    }
  }
  String str = String("");  
  if (value == -1) {
    str = "min.";
  } else if(value == -2) {
    str = "max.";
  } else {
    char buf[10];
    dtostrf(value, 4, 0, buf);
    str = String(buf);
  }
  if (str.length() == 2) {
    str = "  " + str;
  } else if (str.length() == 1) {
    str = "   " + str;
  } else if (str.length() == 3) {
    str = " " + str;
  }
  lcd.setCursor(10, 1);
  lcd.print(str);
}

void loop() {
  measureVoltage();
  measureCurrent();
  delay(100);
}

