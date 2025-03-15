#!/bin/bash
set -e 

systemctl --user stop tz.service || true
systemctl --user disable tz.service || true

rm -f "$HOME/.config/systemd/user/tz.service"

systemctl --user daemon-reload

sudo rm -f /usr/bin/tz

echo "Uninstallation complete! The service has been removed."
exit 0
