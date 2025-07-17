#ifndef TUYAREPORTER_UBUS_WRAPPER
#define TUYAREPORTER_UBUS_WRAPPER
#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include "utils.h"

int get_esp_devices_json(struct ubus_context *ctx, char **out);
int set_esp_on(struct ubus_context *ctx, char **out, char *port, int pin);
int set_esp_off(struct ubus_context *ctx, char **out, char *port, int pin);
int get_esp_sensor(struct ubus_context *ctx, char **out, char *port, int pin, char *model, char *sensor);
#endif