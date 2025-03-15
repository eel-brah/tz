#!/bin/bash
set -e

make
sudo cp ./tz /usr/bin/
make fclean

if [ ! -d "$HOME/.config/systemd/user/" ]; then
    mkdir -p "$HOME/.config/systemd/user/"
fi
cp ./systemd/tz.service "$HOME/.config/systemd/user/"

systemctl --user daemon-reload
systemctl --user enable tz.service
systemctl --user start tz.service

echo "Installation complete! The service should now start automatically."
exit 0
