/* vim: set ft=cpp: */
#include <math.h>
#include "button.h"

/* pins */

/* these leds are reversed: pin HIGH <=> LED off */
#define ACCEL_BUTTON0 2
#define ACCEL_LED0 13 /* also led on arduino board, though not reversed */
#define ACCEL_BUTTON1 4
#define ACCEL_LED1 7

#define HUE_BUTTON0 8
#define HUE_BUTTON1 12

#define PANEL 3

/* RGB leds are reversed: we send 255 - value to them */
#define LEDR 11
#define LEDG 10
#define LEDB 9


#define ACCELERATOR_TIMEOUT 2000
#define LONG_BOUNCE_PERIOD 1500
#define SHORT_BOUNCE_PERIOD 450

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

#define BUTTONS_LENGTH 4
button_t buttons[BUTTONS_LENGTH] = {
    {
        ACCEL_BUTTON0, /* pin*/
        LOW, /* state */
        LOW, /* last_reading */
        0, /* last_debounce_time */
    },
    {
        ACCEL_BUTTON1, /* pin*/
        LOW, /* state */
        LOW, /* last_reading */
        0, /* last_debounce_time */
    },
    {
        HUE_BUTTON0, /* pin*/
        LOW, /* state */
        LOW, /* last_reading */
        0, /* last_debounce_time */
    },
    {
        HUE_BUTTON1, /* pin*/
        LOW, /* state */
        LOW, /* last_reading */
        0, /* last_debounce_time */
    }
};

int valAt(int t) {
  return (E * E * min_light - max_light) / C + 
         (E * (max_light - min_light)) / C * 
         exp(sin((2 * PI / period) * (t + 0.75 * period)));
}

/* accelerator stuff */

bool accelerator_is_on(accelerator_t *accelerator) {
    return accelerator->forced || accelerator->timeout_wait;
}

void accelerator_enable(accelerator_t *accelerator) {
    accelerator->forced = true;
}

void accelerator_disable(accelerator_t *accelerator) {
    accelerator->forced = false;
    accelerator->timeout_wait = true;
    accelerator->end_time = millis() + ACCELERATOR_TIMEOUT;
}

void accelerator_update(accelerator_t *accelerator) {
    if (!accelerator->forced
            && accelerator->timeout_wait
            && millis() > accelerator->end_time) {
        accelerator->timeout_wait = false;
        /* accelerator->end_time becomes meaningless */
    }
}

/* button stuff */
void on_press(button_t *button) {
}

void on_release(button_t *button) {
}

bool is_pressed(button_t *button) {
    return button->state == HIGH;
}

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
      if (is_pressed(button))
        on_press(button);
      else
        on_release(button);
    }
  }

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  button->last_reading = reading;
}

void check_buttons(button_t *button) {
    for (int i=0; i<BUTTONS_LENGTH; i++) {
        check_button(&buttons[i]);
    }
}

unsigned char r, g, b = 0;
unsigned char hue = 0;

void setup() {                
  pinMode(PANEL, OUTPUT);
  pinMode(ACCEL_LED0, OUTPUT);
  pinMode(ACCEL_BUTTON0, INPUT);
  
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  
  // set initial LED state
  digitalWrite(ACCEL_LED0, LOW);
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



