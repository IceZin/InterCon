#include "LedControl.h"

LedControl::LedControl() {
  
}

void LedControl::setupLeds() {
  pinMode(12, OUTPUT);
  pinMode(A0, INPUT);
  
  FastLED.addLeds<WS2811, 12, BRG>(strip, leds);
  FastLED.clear();
  FastLED.show();
  
  Serial.println("[*] Controller started");
  
  sampling_period_us = round(1000000*(1.0/FREQUENCY));
  memcpy(fade_color, party[random(0,2)], sizeof(fade_color));
}

void LedControl::clear() {
  FastLED.clear();
  FastLED.show();
}

void LedControl::start() {
  if (!state) {
    state = true;
    update_ms = millis();
  }
}

void LedControl::stop() {
  state = false;
  clearTemp();
  clear();
}

void LedControl::clearTemp() {
  led = 0;
  leds_on = 0;
  peak_led = 0;
  highest_magnitude = 0;
}

void LedControl::update() {
  bool chk_state = state;
  if (millis() >= info_ms + info_delay) {
    Serial.print("Last State: ");
    Serial.println(state);
    Serial.print("I2C Data: ");
    Serial.println(Wire.available());

    info_ms += info_delay;
  }
  
  if (millis() >= update_ms + update_delay) {
    
    if (state) {
      setIndicatorColor(mode);
      
      if (mode == TRAIL) trail(); 
      else if (mode == FADE) fade();
      else if (mode == SPECTRUM) spectrum();
      else if (mode == RAINBOW) rainbow();

      setIndicatorColor(0);
    }
    
    update_ms += update_delay;
  }
}

void LedControl::autoUpdate() {
  update();
  
  if (millis() - auto_dl >= 10000) {
    setFadeS(trail_clr[asset++]);
    if (asset == 7) asset = 0;
    
    auto_dl = millis();
  }
}

void LedControl::setFadeS(int* rgb) {
  for (int i = 0; i < 3; i++) {
    fade_color[0][i] = rgb[i];
  }
}

void LedControl::setFadeE(int* rgb) {
  for (int i = 0; i < 3; i++) {
    fade_color[1][i] = rgb[i];
  }
}

void LedControl::setLen(int len) {
  trail_len = len;
}

void LedControl::setDelay(int ms) {
  update_delay = ms;
}

void LedControl::setIndicatorColor(int i) {
  analogWrite(9, mode_clr[i][0]);
  analogWrite(10, mode_clr[i][1]);
  analogWrite(11, mode_clr[i][2]);
}

void LedControl::trail() {
  if (led-1 >= 0) {
    strip[led-1] = CRGB(0, 0, 0);
  } else {
    strip[leds-1] = CRGB(0, 0, 0);
  }
  
  for (int i = 0; i < trail_len; i++) {
    if (i + led > leds-1) {
      strip[(i+led)-leds] = CRGB(fade_color[0][0], fade_color[0][1], fade_color[0][2]);
    } else {
      strip[i+led] = CRGB(fade_color[0][0], fade_color[0][1], fade_color[0][2]);
    }
  }
  
  FastLED.show();
  led++;
  
  if (led == leds) {
    led = 0;
  }
}

void LedControl::fade() {
  double *phases[3] = {(double*)fade_color[0], (double*)fade_color[1]};
  for (int i = 0; i < leds; i++) {
    int fade[3];
    int mtl = 0;
    if (led + i >= leds) mtl = 1;
    calcFade((double)((led + i) - (leds * mtl)) / (double)leds, phases, fade);
    strip[i] = CRGB(fade[0], fade[1], fade[2]);
  }
  if (++led == leds) led = 0;
  FastLED.show();
}

void LedControl::spectrum() {
  microseconds = micros();
  
  for(int i=0; i<SAMPLES; i++){
    while(micros()-microseconds < sampling_period_us){
    }

    microseconds += sampling_period_us;
    vReal[i] = analogRead(0);
    vImag[i] = 0;
  }

  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  double peak = FFT.MajorPeak(vReal, SAMPLES, FREQUENCY);
  double freq_val = vReal[round(60.0/(FREQUENCY/SAMPLES)*1.0)];
  peak_led = (leds*round(freq_val))/FREQUENCY_MAGNITUDE;
  
  if (freq_val > highest_magnitude) {
    highest_magnitude = freq_val;
  }
  
  Serial.print("Frequency Value: ");
  Serial.println(vReal[round(peak/(FREQUENCY/SAMPLES)*1.0)]);
  //Serial.print("Peak LED: ");
  //Serial.println(peak_led);
  
  if (peak_led >= leds) {
    peak_led = leds-1;
  }
  
  if (peak_led > leds_on) {
    for (int i = leds_on; i < peak_led; i++) {
      strip[i] = CRGB(round((i/double(leds-1))*double(fade_color[1][0]-fade_color[0][0]))+fade_color[0][0], 
      round((i/double(leds-1))*double(fade_color[1][1]-fade_color[0][1]))+fade_color[0][1], 
      round((i/double(leds-1))*double(fade_color[1][2]-fade_color[0][2]))+fade_color[0][2]);
      FastLED.show();
      delay(5);
    }
    leds_on = peak_led;
  }
  
  if (millis() >= dl_a + 1) {
    if (leds_on > peak_led) {
      strip[leds_on] = CRGB(0,0,0);
      FastLED.show();
      leds_on--;
    }
    dl_a += 1;
  }
}

void LedControl::rainbow() {
  double phases[3][3] = {{0.0, 0.0, 255.0}, {0.0, 255.0, 0.0}, {255.0, 0.0, 0.0}};
  double *p[3] = {phases[0], phases[1], phases[2]};
  for (int i = 0; i < leds; i++) {
    int fade[3];
    int mtl = 0;
    
    if (led + i >= leds) mtl = 1;
    
    calcFade((double)((led + i) - (leds * mtl)) / (double)leds, p, fade);
    strip[i] = CRGB(fade[0], fade[1], fade[2]);
  }
  
  if (++led == leds) led = 0;
  FastLED.show();
}

void LedControl::calcFade(double intensity, double *phases[3], int* result) {
  //Speed intensity
  if (intensity > 1) intensity = 1.0;

  //Start Phase
  int ls_size = (sizeof(phases) / sizeof(phases[0]));
  int ph = floor((double)ls_size * intensity);

  //End Phase
  int endph;
  if (ph == ls_size - 1) endph = 0;
  else endph = ph + 1;

  //Phase value
  double phase_n = (1.0 / (double)ls_size) * ph;

  //Phase intensity
  if (phase_n > 0) intensity = (intensity - phase_n) / phase_n;
  else intensity *= ls_size;

  for (int i = 0; i < 3; i++) result[i] = round(intensity * (phases[endph][i] - phases[ph][i]) + phases[ph][i]);
}
