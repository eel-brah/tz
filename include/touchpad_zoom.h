#ifndef TOUCHPAD_ZOOM_H
# define TOUCHPAD_ZOOM_H

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>

#define MIN_ZOOM_FACTOR 1.0
#define MAX_ZOOM_FACTOR 4.0
#define ZOOM_SENSITIVITY 0.5 // Adjust this to control zoom speed

extern int TOUCHPAD_ID;
extern volatile sig_atomic_t zoom_running;

// Struct to track zoom state
typedef struct s_args {
  double target_zoom_factor;
  double current_zoom_factor;
  double base_zoom_factor;
  pthread_mutex_t zoom_mutex;
} t_args;

// Zoom
void set_zoom_factor(double factor);
void enable_zoom();
// void disable_zoom();
void *update_zoom(void *arg);

// Init
Display *init(int *opcode, int *event, int *error, t_args *args);
int get_touchpad_id(Display *display);
void setup_events(Display *display, int touchpad_id);

#endif
