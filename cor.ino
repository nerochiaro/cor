#include <math.h>

#define PANEL 3
#define TOUCHLED 11
#define TOUCH 8

#define E 2.71828

// Based on http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
//#define PULSEF(x) (MIN_LIGHT + ((exp(sin(x)) - 1 / E) * ((255 - MIN_LIGHT) / (E - (1 / E)))))

#define MIN_LIGHT 30
#define A (MIN_LIGHT - (1 / E))
#define B ((255 - MIN_LIGHT) / (E - (1 / E)))

#define SAMPLES 50

int period = 5000;
int valAt(int t) {
  return A + 
         exp(sin(((2 * PI) / period) * (t + 0.75 * period))) * B;
}

void setup() {                
  pinMode(PANEL, OUTPUT);
  pinMode(TOUCHLED, OUTPUT);
  pinMode(TOUCH, INPUT);
  Serial.begin(9600);
}

#define BOUNCE_TIME 200
long lastTouch = 0;

void loop() {
  int touch = digitalRead(TOUCH);
  
  int t = 0;
  for (int i = 0; i < SAMPLES; i++) {
    t = (period / SAMPLES) * i;
    int val = valAt(t);
    Serial.print(val);
    Serial.print(", ");
    analogWrite(PANEL, val);
    delay(period / SAMPLES);
  }
  
  Serial.print("\n");
  
  analogWrite(PANEL, 0);
  delay(2000);
//   if (touch == HIGH) {
// //     if (now - lastTouch > BOUNCE_TIME) {
//       analogWrite(TOUCHLED, 255);
// //     }
//     lastTouch = now;
//   } else {
//     analogWrite(TOUCHLED, 0);
//   }
}


