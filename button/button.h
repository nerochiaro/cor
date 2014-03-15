typedef struct {
  int pin;
  int state; /* confirmed state after debounce */
  int last_reading; /* last measured, could be bounce effect */
  int last_debounce_time; /* last time we had a reading change */
} button_t;

