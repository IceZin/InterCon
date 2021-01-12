#include "LedControl.h"
#include "Command.h"
#include <Wire.h>

LedControl led_strip;
Command observer;

long change_mode_dl;
long last_delay = 0;
long btn_delay = 0;
long spd_delay = 0;
long mnl_delay = 0;
int speed = 0;
int rgb_pins[3] = {9, 10, 11};
double phases[3][3] = {{30.0, 30.0, 255.0}, {0.0, 255.0, 0.0}, {255.0, 0.0, 0.0}};

bool manualCtrl = false;

void setup() {
  Wire.begin(0);
  Wire.onReceive(i2cHandler);
  
  pinMode(7, INPUT);
  Serial.begin(115200);
  Serial.println("[!] Starting Controller");
  led_strip.setupLeds();
  led_strip.mode = 0x00;
  led_strip.start();
  observer.strip = &led_strip;
}

void loop() {
  if (Wire.available() > 0) Serial.println(Wire.available());

  while (!Wire.available()) {
    if (manualCtrl) {
      led_strip.autoUpdate();
  
      if (millis() - btn_delay >= 200) {
        if (digitalRead(5) == HIGH) {
          ++led_strip.mode;
          led_strip.clearTemp();
          
          if (led_strip.mode == 0x1) {
            led_strip.start();
          } else if (led_strip.mode > 0x3) {
            led_strip.mode = 0x0;
            led_strip.stop();
          }
    
          Serial.print("Mode");
          Serial.println(led_strip.mode);
        }
      }
    } else {
      led_strip.update();
    }
  
    if (millis() - btn_delay >= 200) {
      if (digitalRead(4) == HIGH) {
        if (!manualCtrl) {
          manualCtrl = true;
          Serial.println("[!] Manual control ON");
        } else {
          manualCtrl = false;
          Serial.println("[!] Manual control OFF");
        }
      }
      
      btn_delay = millis();
    }
  
    if (millis() - spd_delay >= 5) {
      int s = speed;
      if (digitalRead(6) == HIGH and speed > 0) speed--;
      else if (digitalRead(7) == HIGH and speed < 400) speed++;

      if (manualCtrl) setColor();
      
      if (speed != s) led_strip.setDelay(speed);
  
      /*if (speed <= 20) {
        setRGB(255, 0, 0);
      } else if (speed > 20 and speed <= 100) {
        setRGB(0, 255, 0);
      } else {
        setRGB(0, 0, 255);
      }*/
  
      spd_delay = millis();
    }
  }
}

void i2cHandler(int bytes) {
  observer.handleI2C(bytes);
}

void setRGB(int r, int g, int b) {
  analogWrite(rgb_pins[0], r);
  analogWrite(rgb_pins[1], g);
  analogWrite(rgb_pins[2], b);
}

void setColor() {
  //Speed intensity
  double intensity = speed / 400.0;
  if (intensity > 1) intensity = 1.0;

  //Start Phase
  int ls_size = (sizeof(phases) / sizeof(phases[0])) - 1;
  int ph = floor((double)ls_size * intensity);

  //End Phase
  int endph;
  if (ph == ls_size) endph = ph;
  else endph = ph + 1;

  //Phase value
  double phase_n = (1.0 / (double)ls_size) * ph;

  //Phase intensity
  if (phase_n > 0) intensity = (intensity - phase_n) / phase_n;
  else intensity *= ls_size;
  
  int r = round(intensity * (phases[endph][0] - phases[ph][0]) + phases[ph][0]);
  int g = round(intensity * (phases[endph][1] - phases[ph][1]) + phases[ph][1]);
  int b = round(intensity * (phases[endph][2] - phases[ph][2]) + phases[ph][2]);
  
  setRGB(r, g, b);
}
