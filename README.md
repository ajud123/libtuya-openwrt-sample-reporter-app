# Tuya reporter

This project is a sample project to use with the [libtuya](https://github.com/ajud123/libtuya-openwrt) library for OpenWRT. It also demonstrates the use of ubus to provide a simple interface to request data. Heavily based on [esp-control-over-serial-ubus](https://github.com/ajud123/esp-control-over-serial-ubus), as most of it's functionality is mirrored. Requires an ESP8266 device connected with [esp_control_over_serial](https://github.com/janenasl/esp_control_over_serial) firmware installed on it.

Doesn't accept any additional arguments and is intended to be ran as an init.d daemon. Currently provides the following methods over the Tuya IoT cloud:
* `devices`. No arguments.
* `on`. Accepts the following parameters: `port`: String, `pin`: Value
* `off`. Accepts the following parameters: `port`: String, `pin`: Value
* `get`. Accepts the following parameters: `port`: String, `pin`: Value, `sensor`: String, `model`: String

For detailed information about the pins and sensor arguments, check the documentation of the linked ESP8266 firmware. All methods return a JSON object as a string.

To allow the program to connect to the Tuya IoT cloud platform, the following arguments must be passed: `-p <product_id>`, `-d <device_id>`, `-s <device_secret>`. A fourth optional arg `-b` can be passed to instruct the program to run as a daemon program in the background.
