#include "utils.h"
#include "tuya_wrapper.h"
#include "tuya_cacert.h"
#include "ubus_wrapper.h"
#include "action_handler.h"
#include <stdlib.h>
#include <cJSON.h>
#include <tuya_error_code.h>
#include <tuya_log.h>
#include <tuyalink_core.h>
#include <log.h>
#include <syslog.h>
#include <libubus.h>

static void syslog_wrapper(log_Event *ev)
{
	vsyslog(ev->level, ev->fmt, ev->ap);
	fflush(stdout);
}

static void on_messages(tuya_mqtt_context_t *context, void *user_data, const tuyalink_message_t *msg)
{
	TY_LOGI("on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
	switch (msg->type) {
	case THING_TYPE_ACTION_EXECUTE: {
		TY_LOGI("received action command:%s", msg->data_string);
		cJSON *json = cJSON_Parse(msg->data_string);
		handle_action(context, user_data, json);
		cJSON_Delete(json);
		break;
	}
	case THING_TYPE_ACTION_EXECUTE_RSP: {
		TY_LOGI("received action command response:%s", msg->data_string);
		cJSON *json = cJSON_Parse(msg->data_string);
		handle_action(context, user_data, json);
		cJSON_Delete(json);
		break;
	}
	default:
		break;
	}
}

int tuya_start(tuya_mqtt_context_t *client, char *deviceSecret, char *deviceId, struct ubus_context *uctx)
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
								      .on_messages   = on_messages,
								      .user_data     = uctx });
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

void tuya_stop(tuya_mqtt_context_t *client)
{
	tuya_mqtt_disconnect(client);
	tuya_mqtt_deinit(client);
}
