#include "../include/touchpad_zoom.h"

Display *init(int *opcode, int *event, int *error, t_args *args) {
  args->base_zoom_factor = 1;
  args->current_zoom_factor = 1;
  args->target_zoom_factor = 1;

  // Open connection to the X server
  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display\n");
    return NULL;
  }
  // Check if XInput2 is available
  if (!XQueryExtension(display, "XInputExtension", opcode, event, error)) {
    fprintf(stderr, "XInput2 not available\n");
    XCloseDisplay(display);
    return NULL;
  }
  return display;
}

int get_touchpad_id(Display *display) {
  int touchpad_id = TOUCHPAD_ID;
  if (touchpad_id != -1)
  {
    syslog(LOG_INFO, "Using touchpad ID: %d", touchpad_id);
    printf("Using touchpad ID: %d\n", touchpad_id);
    return touchpad_id;
  }
  // Query all input devices
  int ndevices;
  XIDeviceInfo *devices = XIQueryDevice(display, XIAllDevices, &ndevices);
  if (!devices) {
    fprintf(stderr, "Failed to query input devices\n");
    return -1;
  }
  // Find the touchpad device
  for (int i = 0; i < ndevices; i++) {
    if (strstr(devices[i].name, "Touchpad") ||
        strstr(devices[i].name, "TouchPad")) {
      touchpad_id = devices[i].deviceid;
      syslog(LOG_INFO, "Found touchpad: %s (ID: %d)\n", devices[i].name, touchpad_id);
      printf("Found touchpad: %s (ID: %d)\n", devices[i].name, touchpad_id);
      break;
    }
  }
  XIFreeDeviceInfo(devices);
  if (touchpad_id == -1) {
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
