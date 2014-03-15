#include <math.h>

#define PANEL 3
#define TOUCHLED 11
#define TOUCH 8

#define E 2.71828

// Based on http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
//#define PULSEF(x) (MIN_LIGHT + ((exp(sin(x)) - 1 / E) * ((255 - MIN_LIGHT) / (E - (1 / E)))))

int min_light = 20;
int max_light = 255;
int period = 1500;
#define A (E * E * MIN_LIGHT - MAX_LIGHT) / (E * E - 1)
#define B (E * (MAX_LIGHT - MIN_LIGHT)) / (E * E - 1)

#define SAMPLES 50
#define C (E * E - 1)


int valAt(int t) {
  return (E * E * min_light - max_light) / C + 
         (E * (max_light - min_light)) / C * 
         exp(sin((2 * PI / period) * (t + 0.75 * period)));
}

void setup() {                
  pinMode(PANEL, OUTPUT);
  pinMode(TOUCHLED, OUTPUT);
  pinMode(TOUCH, INPUT);
}

#define BOUNCE_TIME 200
long lastTouch = 0;

void loop() {
  int t = 0;
  period = 1500;
  max_light = 255;
  for (int i = 0; i < SAMPLES; i++) {
    t = (period / SAMPLES) * i;
    int val = valAt(t);
    analogWrite(PANEL, val);
    delay(period / SAMPLES);
  }
  period = 450;
  max_light = 120;
  for (int i = 0; i < SAMPLES; i++) {
    t = (period / SAMPLES) * i;
    int val = valAt(t);
    analogWrite(PANEL, val);
    delay(period / SAMPLES);
  }

  delay(1000);
}


