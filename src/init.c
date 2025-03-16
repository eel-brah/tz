#include "../include/touchpad_zoom.h"

void handle_arguments(int ac, char **av) {
  if (ac == 1)
    return;

  if (ac == 2 && !strcmp(av[1], "-h")) {
    printf("Usage: %s [OPTION]\n", av[0]);
    printf("  -h          Display this help message\n");
    printf("  -i [ID]     Specify touchpad ID (0-1000) manually\n");
    printf("\n\nDESCRIPTION:\n");
    printf("Enables zoom using a laptop's touchpad.\n By default, it "
           "automatically detects the touchpad ID.\n You can manually specify "
           "the ID using the -i option, or you can set the ID at compile time "
           "by specifying the ID macro'.\n");
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

void systemd_integration(char *av0) {
  if (pipe(pipe_fd) == -1) {
    perror("pipe");
    exit(1);
  }
  signal(SIGTERM, handle_signal);
  signal(SIGINT, handle_signal);
  openlog(av0, LOG_PID, LOG_DAEMON);
  syslog(LOG_INFO, "Daemon started.");
}

Display *init(int *opcode, int *event, int *error, t_args *args) {
  args->base_zoom_factor = 1;
  args->current_zoom_factor = 1;
  args->target_zoom_factor = 1;

  // Open connection to the X server
  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    syslog(LOG_INFO, "Cannot open display");
    fprintf(stderr, "Cannot open display\n");
    return NULL;
  }
  // Check if XInput2 is available
  if (!XQueryExtension(display, "XInputExtension", opcode, event, error)) {
    syslog(LOG_INFO, "XInput2 not available");
    fprintf(stderr, "XInput2 not available\n");
    XCloseDisplay(display);
    return NULL;
  }
  return display;
}

int get_touchpad_id(Display *display) {
  int touchpad_id = TOUCHPAD_ID;
  if (touchpad_id != -1) {
    syslog(LOG_INFO, "Using touchpad ID: %d", touchpad_id);
    printf("Using touchpad ID: %d\n", touchpad_id);
    return touchpad_id;
  }
  // Query all input devices
  int ndevices;
  XIDeviceInfo *devices = XIQueryDevice(display, XIAllDevices, &ndevices);
  if (!devices) {
    syslog(LOG_INFO, "Failed to query input devices");
    fprintf(stderr, "Failed to query input devices\n");
    return -1;
  }
  // Find the touchpad device
  for (int i = 0; i < ndevices; i++) {
    if (strstr(devices[i].name, "Touchpad") ||
        strstr(devices[i].name, "TouchPad")) {
      touchpad_id = devices[i].deviceid;
      syslog(LOG_INFO, "Found touchpad: %s (ID: %d)\n", devices[i].name,
             touchpad_id);
      printf("Using touchpad: %s (ID: %d)\n", devices[i].name, touchpad_id);
      break;
    }
  }
  XIFreeDeviceInfo(devices);
  if (touchpad_id == -1) {
    syslog(LOG_INFO, "No touchpad found");
    fprintf(stderr, "No touchpad found\n");
    return -1;
  }
  return touchpad_id;
}

void setup_events(Display *display, int touchpad_id) {
  // Set up the event mask
  unsigned char mask[XIMaskLen(XI_LASTEVENT)] = {0};
  XIEventMask eventmask;
  eventmask.deviceid = touchpad_id;
  eventmask.mask_len = sizeof(mask);
  eventmask.mask = mask;
  // Enable all relevant event types
  XISetMask(mask, XI_GesturePinchBegin);
  XISetMask(mask, XI_GesturePinchEnd);
  XISetMask(mask, XI_GesturePinchUpdate);
  // Register the event
  XISelectEvents(display, DefaultRootWindow(display), &eventmask, 1);
  XSync(display, False);
}

int run_setup(Display *display, pthread_t *zoom_thread, t_args *args) {
  // Get touchpad id
  int touchpad_id = get_touchpad_id(display);
  if (touchpad_id == -1)
    return 1;
  // Setup the events
  setup_events(display, touchpad_id);
  // Start the zoom thread
  if (pthread_mutex_init(&(args->zoom_mutex), NULL) != 0) {
    fprintf(stderr, "Mutex initialization failed\n");
    syslog(LOG_INFO, "Mutex initialization failed");
    return 1;
  }
  int result = pthread_create(zoom_thread, NULL, update_zoom, args);
  if (result != 0) {
    fprintf(stderr, "Failed to create zoom thread: %s\n", strerror(result));
    syslog(LOG_INFO, "Failed to create zoom thread: %s", strerror(result));
    pthread_mutex_destroy(&(args->zoom_mutex));
    return 1;
  }
  return 0;
}
