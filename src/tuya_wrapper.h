#ifndef TUYAREPORTER_TUYA_WRAPPER
#define TUYAREPORTER_TUYA_WRAPPER

#include "utils.h"
#include <tuyalink_core.h>
#include <libubus.h>
int tuya_start(tuya_mqtt_context_t *client, char *deviceSecret, char *deviceId);
int tuya_loop(tuya_mqtt_context_t *client);
int report_memory_info(struct ubus_context *ctx, tuya_mqtt_context_t *client, struct memory_stats *stats);
int report_network_info(struct ubus_context *ctx, tuya_mqtt_context_t *client, struct if_list **interfaces);
void tuya_stop(tuya_mqtt_context_t *client);

#endif