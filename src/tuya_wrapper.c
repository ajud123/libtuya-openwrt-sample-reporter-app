#include "utils.h"
#include "tuya_wrapper.h"
#include "tuya_cacert.h"
#include "ubus_wrapper.h"
#include <stdlib.h>
#include <cJSON.h>
#include <tuya_error_code.h>
#include <tuya_log.h>
#include <tuyalink_core.h>
#include <log.h>
#include <syslog.h>

static void syslog_wrapper(log_Event *ev)
{
	vsyslog(ev->level, ev->fmt, ev->ap);
	fflush(stdout);
}

static void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg)
{
	(void)context; // unused variable
	(void)user_data; // unused variable
	TY_LOGI("on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
	switch (msg->type) {
	case THING_TYPE_ACTION_EXECUTE:
		TY_LOGI("received action command:%s", msg->data_string);
		cJSON *json = cJSON_Parse(msg->data_string);
		// handle_action(context, user_data, json);
		cJSON_Delete(json);
		break;
	default:
		break;
	}
}

int tuya_start(tuya_mqtt_context_t *client, char *deviceSecret, char *deviceId)
{
	log_add_callback(syslog_wrapper, NULL, 0);
	int ret = tuya_mqtt_init(client, &(const tuya_mqtt_config_t){ .host	  = "m1.tuyacn.com",
								      .port	  = 8883,
								      .cacert	  = tuya_cacert_pem,
								      .cacert_len = sizeof(tuya_cacert_pem),
								      .device_id  = deviceId,
								      .device_secret = deviceSecret,
								      .keepalive     = 100,
								      .timeout_ms    = 2000,
								      .on_connected  = NULL,
								      .on_disconnect = NULL,
								      .on_messages   = on_messages });
	if (ret != OPRT_OK) {
		log_fatal("Failed to initialise Tuya MQTT client.");
		return 1;
	}
	ret = tuya_mqtt_connect(client);
	if (ret != OPRT_OK) {
		log_fatal("Failed to connect to Tuya IoT cloud.");
		log_fatal(
			"Please check if you have an internet connection or have supplied the correct credentials.");
		return 2;
	}
	return ret;
}

int tuya_loop(tuya_mqtt_context_t *client)
{
	return tuya_mqtt_loop(client);
}

/*
 * Reports the current system's free and total memory.
 * Returns 0 on success
 * Returns 1 if getting memory statistics failed
 */
int report_memory_info(struct ubus_context *ctx, tuya_mqtt_context_t *client, struct memory_stats *stats)
{
        int status = get_memory_statistics(ctx, stats);
        if(status != 0) {
                syslog(LOG_ERR, "Failed to get memory statistics.");
                return 1;
        }
        char *msg = generate_memory_json_string(stats);
        tuyalink_thing_property_report(client, NULL, msg);
        free(msg);
        return 0;
}

/*
 * Reports the current network interfaces.
 * Returns 0 on success
 * Returns 1 if getting network interfaces failed
 */
int report_network_info(struct ubus_context *ctx, tuya_mqtt_context_t *client, struct if_list **interfaces)
{
        int status = get_network_interfaces(interfaces);
        if(status != 0){
                syslog(LOG_ERR, "Could not get network interface names.");
                delete_interface_list(interfaces);
                return 1;
        }
        struct if_list *interface = *interfaces;
        while(interface != NULL){
                status = get_network_statistics(ctx, interface);
                if(status != 0) {
                        syslog(LOG_ERR, "Could not get info for network interface %s.", interface->name);
                        return 1;
                }
                interface = interface->next;
        }
        char *msg = generate_network_json_string(interfaces);
        tuyalink_thing_property_report(client, NULL, msg);
        syslog(LOG_INFO, "Freeing JSON message.");
        free(msg);
        syslog(LOG_INFO, "Deleting interface list.");
        delete_interface_list(interfaces);
        syslog(LOG_INFO, "Deleted interface list.");
        return 0;
}

void tuya_stop(tuya_mqtt_context_t *client)
{
	tuya_mqtt_disconnect(client);
	tuya_mqtt_deinit(client);
}
