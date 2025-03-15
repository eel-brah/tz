#include "../include/touchpad_zoom.h"

int TOUCHPAD_ID = ID;
volatile sig_atomic_t zoom_running = 1;

void handle_signal(int sig) {
  (void)sig;
  zoom_running = 0;
}

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

int start(Display *display, pthread_t *zoom_thread, t_args *args, int opcode) {
  // Get touchpad id
  int touchpad_id = get_touchpad_id(display);
  if (touchpad_id == -1)
    return 1;

  // Setup the events
  setup_events(display, touchpad_id);

  // Start the zoom thread
  if (pthread_mutex_init(&(args->zoom_mutex), NULL) != 0) {
    fprintf(stderr, "Mutex initialization failed\n");
    return 1;
  }
  pthread_create(zoom_thread, NULL, update_zoom, args);

  // Main event loop
  while (zoom_running) {
    XEvent ev;
    XNextEvent(display, &ev);
    if (ev.type == GenericEvent && ev.xcookie.extension == opcode) {
      if (XGetEventData(display, &ev.xcookie)) {
        handle_events(ev.xcookie.data, args);
        XFreeEventData(display, &ev.xcookie);
      }
    }
  }
  return 0;
}

void clean_up(Display *display, pthread_t *zoom_thread, t_args *args) {
  pthread_join(*zoom_thread, NULL);
  pthread_mutex_destroy(&(args->zoom_mutex));
  XCloseDisplay(display);
}
void handle_arguments(int ac, char **av) {
  if (ac == 1) {
    return;
  }

  if (ac == 2 && !strcmp(av[1], "-h")) {
    printf("Usage: %s [OPTION]\n", av[0]);
    printf("  -h          Display this help message\n");
    printf("  -i [ID]     Specify touchpad ID (0-1000) manually\n");

    printf("\n\nDESCRIPTION:\n");
    printf("Enables zoom using a laptop's touchpad.\n");
    printf("By default, it automatically detects the touchpad ID.\n");
    printf("You can manually specify the ID using the -i option.\n");
    exit(0);
  }

  if (ac == 3 && strcmp(av[1], "-i") == 0) {
    char *endptr;
    errno = 0;
    long id = strtol(av[2], &endptr, 10);

    if (*endptr != '\0' || errno == ERANGE || id < 0 || id > 1000) {
      fprintf(
          stderr,
          "Error: Invalid ID '%s'. ID must be a number between 0 and 1000.\n",
          av[2]);
      exit(EXIT_FAILURE);
    }

    TOUCHPAD_ID = (int)id;
    return;
  }

  fprintf(stderr, "Invalid usage. Run '%s -h' for help.\n", av[0]);
  exit(1);
}

int main(int ac, char **av) {

  // process inputs and make it work in the background
  handle_arguments(ac, av);

  // systemd
  signal(SIGTERM, handle_signal); // systemd stop signal
  openlog(av[0], LOG_PID, LOG_DAEMON);
  syslog(LOG_INFO, "Daemon started.");

  // Init Display, args and get XInput2 info
  t_args args;
  pthread_t zoom_thread;
  int opcode, event, error;
  Display *display = init(&opcode, &event, &error, &args);
  if (display == NULL)
    return 1;

  if (start(display, &zoom_thread, &args, opcode)) {
    XCloseDisplay(display);
    return 1;
  }

  // Clean up - tho we never get here
  syslog(LOG_INFO, "Daemon shutting down.");
  closelog();
  clean_up(display, &zoom_thread, &args);
  return 0;
}
