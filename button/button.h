typedef struct {
  int pin;
  bool forced;
  long fade_start;
} led_t;

typedef struct {
  int pin;
  int state; /* confirmed state after debounce */
  int last_reading; /* last measured, could be bounce effect */
  long last_debounce_time; /* last time we had a reading change */
  led_t led;
} button_t;

