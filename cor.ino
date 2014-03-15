/* vim: set ft=cpp: */
#include <math.h>
#include "button.h"

/* pins */

/* these leds are reversed: pin HIGH <=> LED off */
#define ACCEL_BUTTON0_PIN 2
#define ACCEL_LED0 13 /* also led on arduino board, though not reversed */
#define ACCEL_BUTTON1_PIN 4
#define ACCEL_LED1 7

#define HUE_BUTTON0_PIN 8
#define HUE_BUTTON1_PIN 12

#define PANEL 3

/* RGB leds are reversed: we send 255 - value to them */
#define LEDR 11
#define LEDG 10
#define LEDB 9


#define ACCELERATOR_TIMEOUT 2000
#define LONG_BOUNCE_PERIOD 1500
#define SHORT_BOUNCE_PERIOD 450

#define E 2.71828

int min_light = 20;
int max_light = 255;
int period = LONG_BOUNCE_PERIOD;
int acceleration_divider = 1;

#define SAMPLES 50
#define C (E * E - 1)

/* We keep a bit field state. Whenever that state changes, we need to stop the
 * current bouncing and reinit the main loop. */
#define HUE_SELECTION_MODE (1 << 0)
#define ACCELERATOR0_ON (1 << 1)
#define ACCELERATOR1_ON (1 << 2)
int loop_state = 0;

#define ACCEL_BUTTON0 0
#define ACCEL_BUTTON1 1
#define HUE_BUTTON0 2
#define HUE_BUTTON1 3

#define BUTTONS_LENGTH 4
button_t buttons[BUTTONS_LENGTH] = {
    {
        ACCEL_BUTTON0_PIN, /* pin*/
        LOW, /* state */
        LOW, /* last_reading */
        0, /* last_debounce_time */
    },
    {
        ACCEL_BUTTON1_PIN, /* pin*/
        LOW, /* state */
        LOW, /* last_reading */
        0, /* last_debounce_time */
    },
    {
        HUE_BUTTON0_PIN, /* pin*/
        LOW, /* state */
        LOW, /* last_reading */
        0, /* last_debounce_time */
    },
    {
        HUE_BUTTON1_PIN, /* pin*/
        LOW, /* state */
        LOW, /* last_reading */
        0, /* last_debounce_time */
    }
};

#define ACCELERATORS_LENGTH 2
accelerator_t accelerators[ACCELERATORS_LENGTH] = {
    {
        false, /* forced */
        false, /* timeout_wait */
        0 /* end_time */
    },
    {
        false, /* forced */
        false, /* timeout_wait */
        0 /* end_time */
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

void update_accelerators() {
    acceleration_divider = 1;
    for (int i = 0; i < ACCELERATORS_LENGTH; i++) {
        accelerator_update(&accelerators[i]);
        if (accelerator_is_on(&accelerators[i])) {
            acceleration_divider *= 2;
        }
    }

    if (accelerator_is_on(&accelerators[0]))
        digitalWrite(ACCEL_LED0, LOW); // led on
    else
        digitalWrite(ACCEL_LED0, HIGH);

    if (accelerator_is_on(&accelerators[1]))
        digitalWrite(ACCEL_LED1, LOW); // led on
    else
        digitalWrite(ACCEL_LED1, HIGH);
}

/* button stuff */
void on_press(button_t *button) {
    switch(button->pin) {
        case ACCEL_BUTTON0_PIN:
            accelerator_enable(&accelerators[0]);
            break;
        case ACCEL_BUTTON1_PIN:
            accelerator_enable(&accelerators[1]);
            break;
        default:
            break;
    }
}

void on_release(button_t *button) {
    switch(button->pin) {
        case ACCEL_BUTTON0_PIN:
            accelerator_disable(&accelerators[0]);
            break;
        case ACCEL_BUTTON1_PIN:
            accelerator_disable(&accelerators[1]);
            break;
        default:
            break;
    }
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

void check_buttons() {
    for (int i=0; i<BUTTONS_LENGTH; i++) {
        check_button(&buttons[i]);
    }
}

/* state stuff */

bool is_hue_selection_mode() {
    return is_pressed(&buttons[HUE_BUTTON0])
            && is_pressed(&buttons[HUE_BUTTON1]);
}

int current_state() {
    int state = 0;
    if (is_hue_selection_mode())
        state |= HUE_SELECTION_MODE;
    if (accelerator_is_on(&accelerators[0]))
        state |= ACCELERATOR0_ON;
    if (accelerator_is_on(&accelerators[1]))
        state |= ACCELERATOR1_ON;

    return state;
}

bool need_reinit() {
    int new_state;
    bool result;

    check_buttons();
    update_accelerators();

    new_state = current_state();

    result = new_state != loop_state;

    loop_state = new_state;

    return result;
}

unsigned char r, g, b = 0;
unsigned char hue = 0;

void setup() {
  pinMode(ACCEL_BUTTON0_PIN, INPUT);
  pinMode(ACCEL_LED0, OUTPUT);
  pinMode(ACCEL_BUTTON1_PIN, INPUT);
  pinMode(ACCEL_LED1, OUTPUT);

  pinMode(HUE_BUTTON0_PIN, INPUT);
  pinMode(HUE_BUTTON1_PIN, INPUT);

  pinMode(PANEL, OUTPUT);

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
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
init:

  digitalWrite(ACCEL_LED0, HIGH);
  digitalWrite(ACCEL_LED1, HIGH);
  analogWrite(PANEL, min_light);
  hsvtorgb (&r, &g, &b, hue, min_light, 255);
  analogWrite(LEDR, r);
  analogWrite(LEDG, g);
  analogWrite(LEDB, b);

  while (is_hue_selection_mode()) {
      updateHue();
      delay(20);
      if (need_reinit())
          goto init;
  }

  int t = 0;
  period = LONG_BOUNCE_PERIOD / acceleration_divider;
  max_light = 255;
  for (int i = 0; i < SAMPLES; i++) {
    t = (period / SAMPLES) * i;
    int val = valAt(t);
    analogWrite(PANEL, val);

    delay(period / SAMPLES);

    if (need_reinit())
        goto init;
  }

  /* second bounce is on rgb, not on panel */
  period = SHORT_BOUNCE_PERIOD / acceleration_divider;
  max_light = 120;
  for (int i = 0; i < SAMPLES; i++) {
    t = (period / SAMPLES) * i;
    int val = valAt(t);

    hsvtorgb (&r, &g, &b, hue, val, 255);
    analogWrite(LEDR, r);
    analogWrite(LEDG, g);
    analogWrite(LEDB, b);

    delay(period / SAMPLES);

    if (need_reinit())
        goto init;
  }

  for (int i = 0; i<(100/acceleration_divider); i++) {
      delay(10);

    if (need_reinit())
        goto init;
  }
}



