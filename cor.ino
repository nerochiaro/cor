#include <math.h>
#include "button/button.h"

#define TOUCHLED 6
#define TOUCH 2
#define LED_FADE_TIME 2000

#define PANEL 3

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

button_t button0 = {
  TOUCH, /* pin*/
  LOW, /* state */
  LOW, /* last_reading */
  0, /* last_debounce_time */
  /* led */
  {
    TOUCHLED, /* pin */
    false, /* forced */
    -1 /* fade start */
  }
};

int valAt(int t) {
  return (E * E * min_light - max_light) / C + 
         (E * (max_light - min_light)) / C * 
         exp(sin((2 * PI / period) * (t + 0.75 * period)));
}

void start_fade(led_t *led) {
  led->forced = false;
  led->fade_start = millis();
}

void force_on(led_t *led) {
  led->forced = true;
  led->fade_start = -1;
}

void update_led(led_t *led) {
  int led_val = 0;

  if (led->forced) {
    led_val = 255;
  } else if (led->fade_start >= 0) {
    long time_since_trigger = millis() - led->fade_start;

    if (time_since_trigger <= LED_FADE_TIME) {
      led_val = 255 - time_since_trigger * 255 / LED_FADE_TIME;
    }
  }
  analogWrite(led->pin, led_val);
}

void on_press(button_t *button) {
  force_on(&button->led);
}

void on_release(button_t *button) {
  start_fade(&button->led);
}

/* Should be called frequently in loop() */
void check_button(button_t *button) {
  int reading = digitalRead(button->pin);

  long now = millis();

  if (reading != button->last_reading) {
    // reset the debouncing timer
    button->last_debounce_time = now;
  }

  if ((now - button->last_debounce_time) > DEBOUNCE_DELAY) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != button->state) {
      button->state = reading;
      if (button->state == HIGH)
        on_press(button);
      else
        on_release(button);
    }
  }

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  button->last_reading = reading;
}

long lastTouch = 0;

unsigned char r, g, b = 0;
unsigned char hue = 0;

void setup() {                
  pinMode(PANEL, OUTPUT);
  pinMode(TOUCHLED, OUTPUT);
  pinMode(TOUCH, INPUT);
  
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  
  // set initial LED state
  digitalWrite(TOUCHLED, LOW);
}

/* HSV to RGB conversion function with only integer
 * math. S and V go from 0 to 255, and H goes from 0 to 360
 */
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
    check_button(&button0);
    update_led(&button0.led);
  
    delay(period / SAMPLES);
  }
  period = 450;
  max_light = 120;
  for (int i = 0; i < SAMPLES; i++) {
    t = (period / SAMPLES) * i;
    int val = valAt(t);
    analogWrite(PANEL, val);

    updateHue();
    check_button(&button0);
    update_led(&button0.led);

    delay(period / SAMPLES);
  }

  delay(1000);
}



