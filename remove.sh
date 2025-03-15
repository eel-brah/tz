#!/bin/bash
set -e

# Stop and disable the service before removing
systemctl stop tz.service || true
systemctl disable tz.service || true
rm /usr/bin/tz
rm /etc/systemd/system/tz.service
systemctl daemon-reload

exit 0
