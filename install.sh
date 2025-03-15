#!/bin/bash
set -e

make
cp ./tz /usr/bin/
cp ./systemd/tz.service /etc/systemd/system/ 

# Enable the service for autostart
systemctl daemon-reload
systemctl enable tz.service
systemctl start tz.service || true

exit 0
