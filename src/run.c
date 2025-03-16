#include "../include/touchpad_zoom.h"

volatile sig_atomic_t zoom_running = 1;

void handle_events(XIDeviceEvent *xdata, t_args *args) {
  double scale_ratio;
  pthread_mutex_lock(&(args->zoom_mutex));
  switch (xdata->evtype) {
  case XI_GesturePinchBegin:
    enable_zoom();
    // Save the current zoom
    args->base_zoom_factor = args->current_zoom_factor;
    break;

  case XI_GesturePinchUpdate:
    scale_ratio = ((XIGesturePinchEvent *)xdata)->scale;
    // Calculate zoom based on relative change starting from
    // the current base zoom level
    if (scale_ratio > 1.0) {
      args->target_zoom_factor =
          args->base_zoom_factor +
          (scale_ratio - 1.0) * ZOOM_SENSITIVITY * args->base_zoom_factor;
    } else if (scale_ratio < 1.0) {
      args->target_zoom_factor =
          args->base_zoom_factor -
          (1.0 - scale_ratio) * ZOOM_SENSITIVITY * args->base_zoom_factor;
    }
    break;

    // case XI_GesturePinchEnd:
    //   break;

  default:
    break;
  }
  pthread_mutex_unlock(&(args->zoom_mutex));
}

int run(Display *display, pthread_t *zoom_thread, t_args *args, int opcode) {
  run_setup(display, zoom_thread, args);

  // Main event loop
  while (zoom_running) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    // Watch both X11 connection and pipe for events
    FD_SET(ConnectionNumber(display), &read_fds);
    FD_SET(pipe_fd[0], &read_fds);
    int max_fd = ConnectionNumber(display) > pipe_fd[0]
                     ? ConnectionNumber(display)
                     : pipe_fd[0];
    // Wait for an event from either X11 or pipe_fd
    if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) > 0) {
      if (FD_ISSET(pipe_fd[0], &read_fds)) {
        char buf;
        read(pipe_fd[0], &buf, sizeof(buf));
        break;
      }
      if (FD_ISSET(ConnectionNumber(display), &read_fds)) {
        XEvent ev;
        XNextEvent(display, &ev);
        if (ev.type == GenericEvent && ev.xcookie.extension == opcode) {
          if (XGetEventData(display, &ev.xcookie)) {
            handle_events(ev.xcookie.data, args);
            XFreeEventData(display, &ev.xcookie);
          }
        }
      }
    }
  }
  return 0;
}

void clean_up(Display *display, pthread_t *zoom_thread, t_args *args) {
  close(pipe_fd[0]);
  close(pipe_fd[1]);
  syslog(LOG_INFO, "Daemon shutting down.");
  printf("Exiting...\n");
  closelog();
  int result = pthread_join(*zoom_thread, NULL);
  if (result != 0) {
    fprintf(stderr, "Failed to join zoom thread: %s\n", strerror(result));
    syslog(LOG_INFO, "Failed to join zoom thread: %s", strerror(result));
  }
  pthread_mutex_destroy(&(args->zoom_mutex));
  XCloseDisplay(display);
}

void handle_signal(int sig) {
  (void)sig;
  char buf = 1;
  write(pipe_fd[1], &buf, sizeof(buf));
  zoom_running = 0;
}
