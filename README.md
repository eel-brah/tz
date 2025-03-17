# Touchpad Zoom for GNOME

## Overview

tz is a Linux program that enables zooming using a laptop's touchpad.
It utilizes `Xlib` and `XInput2` for handling touchpad gestures.
Currently, it is compatible **only with GNOME**.

## Requirements

To run it you need the following dependencies:

    - X11
    - XInput2

for Debian-based systems:
```sh
sudo apt update
sudo apt install libx11-dev libxi-dev
```

Fedora:
```sh
sudo dnf install libX11-devel libXi-devel
```

Arch Linux:
```sh
sudo pacman -S libx11 libxi
```

## Installation to run as a systemd user service (Recommended)

To install tz, run:

```sh
./install.sh
```

This script will install tz and configure it to run as a **systemd user service**.

If you need to manually specify the touchpad ID during installation, run:

```sh
./install.sh <ID>
```

where `<ID>` is the touchpad ID.

## Usage

### Using Systemd (Recommended)

tz runs as a **systemd user service**. You can manage it with the following commands:

To check if the service is running:

```sh
systemctl --user status tz
```

To stop the service:

```sh
systemctl --user stop tz
```

To start the service:

```sh
systemctl --user start tz
```

### Running Manually (Without Systemd)

If you prefer not to use systemd, you can run the program manually:

```sh
make
./tz &
```

For help:

```sh
./tz -h 
```

To specify a touchpad ID manually:

```sh
./tz -i <ID> &
```

where `<ID>` is the correct touchpad ID obtained from `xinput list`.

To stop it, you may need to manually kill the process:

```sh
pkill tz
```


## Troubleshooting

If tz appears unresponsive to touchpad gestures, you can verify which touchpad ID is being used by checking the logs:
```sh
sudo journalctl | grep tz | grep "Using touchpad"
```

To list all input devices, use:
```sh
xinput list
```

If the touchpad ID shown in the logs is incorrect, reinstall the program with the correct one:
```sh
./remove
./install.sh <ID>
```

If running without systemd:
```sh
./tz <ID> & 
```
Or at compile time:
```sh
make <ID>
```

where `<ID>` is the correct touchpad ID from `xinput list`.

## Uninstallation

To uninstall tz, run:

```sh
./remove.sh
```

## Additional Notes

- tz is currently **only compatible with GNOME**.
- Future updates may include support for other desktop environments.

## Contributing

If you'd like to contribute, feel free to submit pull requests or open issues on GitHub.

## License

This project is licensed under the [MIT License](LICENSE).

