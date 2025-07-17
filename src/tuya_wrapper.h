#ifndef TUYAREPORTER_TUYA_WRAPPER
#define TUYAREPORTER_TUYA_WRAPPER

#include "utils.h"
#include <tuyalink_core.h>
#include <libubus.h>
int tuya_start(tuya_mqtt_context_t *client, char *deviceSecret, char *deviceId, struct ubus_context *uctx);
int tuya_loop(tuya_mqtt_context_t *client);
void tuya_stop(tuya_mqtt_context_t *client);

#endif