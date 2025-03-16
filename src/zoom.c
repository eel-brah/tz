#include "../include/touchpad_zoom.h"

void *update_zoom(void *arg) {
  t_args *args = arg;
  struct timespec sleep_time;
  sleep_time.tv_sec = 0;
  sleep_time.tv_nsec = 16666667; // ~60fps (16.67ms)

  while (zoom_running) {
    pthread_mutex_lock(&(args->zoom_mutex));
    if (fabs(args->current_zoom_factor - args->target_zoom_factor) > 0.01) {
      args->current_zoom_factor +=
          args->target_zoom_factor - args->current_zoom_factor;
      set_zoom_factor(args->current_zoom_factor);
    }
    pthread_mutex_unlock(&(args->zoom_mutex));
    nanosleep(&sleep_time, NULL);
  }
  return NULL;
}

void set_zoom_factor(double factor) {
  if (factor < MIN_ZOOM_FACTOR)
    factor = MIN_ZOOM_FACTOR;
  if (factor > MAX_ZOOM_FACTOR)
    factor = MAX_ZOOM_FACTOR;

  char command[125];
  sprintf(command,
          "gsettings set org.gnome.desktop.a11y.magnifier mag-factor %.2f",
          factor);
  system(command);
}

void enable_zoom() {
  system("gsettings set org.gnome.desktop.a11y.applications "
         "screen-magnifier-enabled true");
}

void disable_zoom() {
  system("gsettings set org.gnome.desktop.a11y.applications "
         "screen-magnifier-enabled false");
}
