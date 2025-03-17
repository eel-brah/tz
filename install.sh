#!/bin/bash
set -e

if [ -n "$1" ]; then
  echo "Building with ID=$1..."
  make ID=$1
else
  echo "Building without ID..."
  make
fi

echo "Copying the tz binary to /usr/bin/..."
sudo cp ./tz /usr/bin/

echo "Cleaning up build files..."
make fclean

if [ ! -d "$HOME/.config/systemd/user/" ]; then
    echo "Creating systemd user directory at $HOME/.config/systemd/user/..."
    mkdir -p "$HOME/.config/systemd/user/"
fi

echo "Copying tz.service to systemd user directory..."
cp ./systemd/tz.service "$HOME/.config/systemd/user/"

echo "Reloading systemd daemon..."
systemctl --user daemon-reload
echo "Enabling tz.service to start on boot..."
systemctl --user enable tz.service
echo "Starting tz.service..."
systemctl --user start tz.service

echo "Installation complete! The service should now start automatically."
exit 0
