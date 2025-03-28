#ifndef TOUCHPAD_ZOOM_H
#define TOUCHPAD_ZOOM_H

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define MIN_ZOOM_FACTOR 1.0
#define MAX_ZOOM_FACTOR 4.0
#define ZOOM_SENSITIVITY 0.5 
#ifndef ID
#define ID -1
#endif

extern volatile sig_atomic_t zoom_running;
extern int pipe_fd[2];

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
void handle_arguments(int ac, char **av);
void systemd_integration(char *av0);
int run_setup(Display *display, pthread_t *zoom_thread, t_args *args);

// Run
int run(Display *display, pthread_t *zoom_thread, t_args *args, int opcode);
void clean_up(Display *display, pthread_t *zoom_thread, t_args *args);
void handle_signal(int sig);

#endif
