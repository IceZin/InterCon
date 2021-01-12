#ifndef LedControl_h
#define LedControl_h

#include "Arduino.h"
#include <FastLED.h>
#include <arduinoFFT.h>
#include <Wire.h>

class LedControl {
  public:
    LedControl();
    void clear();
    void start();
    void stop();
    void update();
    void autoUpdate();
    void setupLeds();
    void setFadeS(int* rgb);
    void setFadeE(int* rgb);
    void setLen(int len);
    void setLed(int led);
    void setDelay(int ms);
    void clearTemp();
    
    int8_t mode = 0x00;
    int8_t t = 3;
  private:
    void trail();
    void fade();
    void spectrum();
    void rainbow();
    void setIndicatorColor(int i);
    void calcFade(double intensity, double *phases[3], int* result);
    
    bool state = false;

    int leds = 50;
    int led = 0;
    int trail_len = 10;
    int SAMPLES = 32;
    int FREQUENCY = 480;
    int FREQUENCY_MAGNITUDE = 110;
    int peak_led = 0;
    int leds_on = 0;
    
    int party[2][2][3] = {{{0, 255, 0}, {255, 0, 0}}, {{0, 0, 255}, {255, 0, 0}}};
    int trail_clr[7][3] = {{255, 0, 0}, {255, 50, 0}, {255, 255, 0}, {0, 255, 0}, {0, 255, 50}, {0, 0, 255}, {128, 0, 128}};
    int mode_clr[5][3] = {{0, 0, 0}, {255, 0, 0}, {255, 50, 0}, {0, 255, 0}, {0, 30, 255}};
    
    int asset = 0;
    int strip_color[3];
    int fade_color[2][3];
    int update_delay = 0;
    int info_delay = 1000;
    
    int8_t TRAIL = 0x1;
    int8_t FADE = 0x2;
    int8_t SPECTRUM = 0x3;
    int8_t RAINBOW = 0x4;
    
    unsigned int sampling_period_us = round(1000000*(1.0/480));
    unsigned long microseconds;
    
    long dl_a;
    long dl_b;
    long auto_dl;
    long update_ms;
    long info_ms;
    
    double vReal[32];
    double vImag[32];
    double highest_magnitude = 0;
    
    CRGB strip[50];
    arduinoFFT FFT = arduinoFFT();
};

#endif
