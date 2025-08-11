# Tuya reporter

This project is a sample project to use with the [libtuya](https://github.com/ajud123/libtuya-openwrt) library for OpenWRT. It also demonstrates the use of ubus communications to poll data from other applications. Heavily based on [tuya-system-status-daemon](https://github.com/ajud123/tuya-system-status-daemon), as most of it's functionality is mirrored, including the reported data format and the data values.

The daemon reports the following system data:
* Total memory (`totalRAM`, double)
* Free memory (`freeRAM`, double)
* Network interfaces and their properties (`networkInterfaces`, array of strings)
* CPU usage (`load`, float)

The network interfaces and their properties are reported in an array format. Each member represents a JSON object, that contains the following fields:
* Interface name (`name`)
* IP address (`address`)
* Netmask (`netmask`)

The data logging happens periodically every 10 seconds. There is no configuration to change that.
To allow the program to connect to the Tuya IoT cloud platform, the following arguments must be passed: `-p <product_id>`, `-d <device_id>`, `-s <device_secret>`. A fourth optional arg `-b` can be passed to instruct the program to run as a daemon program in the background.
