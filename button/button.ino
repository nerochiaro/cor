#include "button.h"

#define TOUCHLED 6
#define TOUCH 2
#define LED_FADE_TIME 2000

#define DEBOUNCE_DELAY 100

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

void setup() {
  pinMode(TOUCH, INPUT);
  pinMode(TOUCHLED, OUTPUT);

  // set initial LED state
  digitalWrite(TOUCHLED, LOW);
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

void loop() {
  check_button(&button0);
  update_led(&button0.led);
}
