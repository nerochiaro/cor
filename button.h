typedef struct {
  int pin;
  int state; /* confirmed state after debounce */
  int last_reading; /* last measured, could be bounce effect */
  unsigned long last_debounce_time; /* last time we had a reading change */
} button_t;

typedef struct {
  bool forced; /* is enabled */
  bool timeout_wait; /* is enabled until now > end_time, overridden by
                        forced */
  unsigned long end_time; /* when timeout_wait stops. Only meaningful if
                             !forced && timeout_wait, undefined otherwise */
} accelerator_t;

#define DEBOUNCE_DELAY 100
#define BOUNCE_TIME 200



