#include "../include/touchpad_zoom.h"

int main(int ac, char **av) {
  t_args args;
  pthread_t zoom_thread;
  int opcode, event, error;
  Display *display;

  // process inputs and make it work in the background
  handle_arguments(ac, av);
  // systemd integration
  systemd_integration(av[0]);
  // Init Display, args and get XInput2 info
  display = init(&opcode, &event, &error, &args);
  if (!display)
    return 1;
  if (run(display, &zoom_thread, &args, opcode)) {
    XCloseDisplay(display);
    return 1;
  }
  // Clean up
  clean_up(display, &zoom_thread, &args);
  return 0;
}
