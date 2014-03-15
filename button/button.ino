#include "button.h"

#define TOUCHLED 11
#define TOUCH 8

#define DEBOUNCE_DELAY 100


button_t button0 = {
  TOUCH, /* button_pin*/
  LOW, /* button_state */
  LOW, /* last_button_state */
  0 /* last_debounce_time */
};

void setup() {
  pinMode(TOUCH, INPUT);
  pinMode(TOUCHLED, OUTPUT);

  // set initial LED state
  digitalWrite(TOUCHLED, LOW);
}

void on_press(button_t *button) {
  if (button->pin == TOUCH) {
    digitalWrite(TOUCHLED, HIGH);
  }
}

void on_release(button_t *button) {
  if (button->pin == TOUCH) {
    digitalWrite(TOUCHLED, LOW);
  }
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
}
