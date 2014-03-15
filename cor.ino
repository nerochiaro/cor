#include <math.h>

#define PANEL 3
#define TOUCHLED 11
#define TOUCH 8

#define LEDR 11
#define LEDG 10
#define LEDB 9

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
  
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
}

#define BOUNCE_TIME 200
long lastTouch = 0;

unsigned char r, g, b = 0;

/* HSV to RGB conversion function with only integer
 * math */
void
hsvtorgb(unsigned char *r, unsigned char *g, unsigned char *b, unsigned char h, unsigned char s, unsigned char v)
{
    unsigned char region, fpart, p, q, t;
    
    if(s == 0) {
        /* color is grayscale */
        *r = *g = *b = v;
        return;
    }
    
    /* make hue 0-5 */
    region = h / 43;
    /* find remainder part, make it from 0-255 */
    fpart = (h - (region * 43)) * 6;
    
    /* calculate temp vars, doing integer multiplication */
    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * fpart) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - fpart)) >> 8))) >> 8;
        
    /* assign temp vars based on color cone region */
    switch(region) {
        case 0:
            *r = v; *g = t; *b = p; break;
        case 1:
            *r = q; *g = v; *b = p; break;
        case 2:
            *r = p; *g = v; *b = t; break;
        case 3:
            *r = p; *g = q; *b = v; break;
        case 4:
            *r = t; *g = p; *b = v; break;
        default:
            *r = v; *g = p; *b = q; break;
    }
    
    return;
}
                         
                         
unsigned char hue = 0;
void updateHue() {
  hue = (hue == 360) ? 0 : hue + 1;
  hsvtorgb(&r, &g, &b, hue, 255, 255); 
  
  analogWrite(LEDR, r);
  analogWrite(LEDG, g);
  analogWrite(LEDB, b);
}

void loop() {
  int t = 0;
  period = 1500;
  max_light = 255;
  for (int i = 0; i < SAMPLES; i++) {
    t = (period / SAMPLES) * i;
    int val = valAt(t);
    analogWrite(PANEL, val);
    updateHue();
    delay(period / SAMPLES);
  }
  period = 450;
  max_light = 120;
  for (int i = 0; i < SAMPLES; i++) {
    t = (period / SAMPLES) * i;
    int val = valAt(t);
    analogWrite(PANEL, val);
    updateHue();
    delay(period / SAMPLES);
  }

  delay(1000);
}


