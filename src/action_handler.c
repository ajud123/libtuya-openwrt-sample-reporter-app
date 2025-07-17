#include "action_handler.h"
#include <tuyalink_core.h>
#include <cJSON.h>
#include <string.h>
#include <log.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include "utils.h"
#include "ubus_wrapper.h"

struct action {
	char *name;
	int (*handler)(tuya_mqtt_context_t *context, void *user_data, cJSON *inputParams);
};

static int action_get_devices(tuya_mqtt_context_t *context, void *user_data, cJSON *inputParams)
{
	(void)context;
	(void)user_data;
	(void)inputParams;
	char *devices;
	struct ubus_context *ctx = (struct ubus_context *)context->config.user_data;
	if(get_esp_devices_json(ctx, &devices) != 0)
                return -1;
	char resp[256];
	snprintf(resp, 256, "{\"actionCode\": \"devices\", \"outputParams\": { \"devices\": \"%s\"}}", devices);
	if (tuyalink_report_action_rsp(context, resp, 0) < 0) {
		return -2;
	}

	free(devices);
	return 0;
}

static int get_pin_data(tuya_mqtt_context_t *context, cJSON *inputParams, int *pin, char **port)
{
	if (inputParams == NULL) {
		log_error("Error while handling %s: NULL provided for input params", __FUNCTION__);
		return 1;
	}

	cJSON *portptr = cJSON_GetObjectItemCaseSensitive(inputParams, "port");
	if (portptr == NULL) {
		log_error("Error while handling %s: \"port\" parameter not provided", __FUNCTION__);
		tuyalink_report_action_rsp(context, NULL, 1);
		return 1;
	}
	*port = cJSON_GetStringValue(portptr);

	cJSON *pinptr = cJSON_GetObjectItemCaseSensitive(inputParams, "pin");
	if (pinptr == NULL || cJSON_IsNumber(pinptr) == 0) {
		log_error("Error while handling %s: \"pin\" parameter not provided", __FUNCTION__);
		tuyalink_report_action_rsp(context, NULL, 2);
		return 2;
	}
	*pin = pinptr->valueint;
	return 0;
}

static int action_on(tuya_mqtt_context_t *context, void *user_data, cJSON *inputParams)
{
	(void)context;
	(void)user_data;
	(void)inputParams;

	int pin;
	char *port;

	if (get_pin_data(context, inputParams, &pin, &port) != 0)
		return -1;

	struct ubus_context *ctx = (struct ubus_context *)context->config.user_data;
	char *ubusJson;
	set_esp_on(ctx, &ubusJson, port, pin);
	char resp[256];
	snprintf(resp, 256, "{\"actionCode\": \"devices\", \"outputParams\": { \"devices\": \"%s\"}}", ubusJson);
	if (tuyalink_report_action_rsp(context, resp, 0) < 0) {
		return -2;
	}
	free(ubusJson);

	return 0;
}

static int action_off(tuya_mqtt_context_t *context, void *user_data, cJSON *inputParams)
{
	(void)context;
	(void)user_data;
	(void)inputParams;

	int pin;
	char *port;

	if (get_pin_data(context, inputParams, &pin, &port) != 0)
		return -1;

	struct ubus_context *ctx = (struct ubus_context *)context->config.user_data;
	char *ubusJson;
	set_esp_off(ctx, &ubusJson, port, pin);
	char resp[256];
	snprintf(resp, 256, "{\"actionCode\": \"devices\", \"outputParams\": { \"devices\": \"%s\"}}", ubusJson);
	if (tuyalink_report_action_rsp(context, resp, 0) < 0) {
		return -2;
	}
	free(ubusJson);

	return 0;
}

static int action_get(tuya_mqtt_context_t *context, void *user_data, cJSON *inputParams)
{
	(void)context;
	(void)user_data;
	(void)inputParams;

	int pin;
	char *port;
	char *model;
	char *sensor;

	if (get_pin_data(context, inputParams, &pin, &port) != 0)
		return -1;

	cJSON *sensorptr = cJSON_GetObjectItemCaseSensitive(inputParams, "sensor");
	if (sensorptr == NULL) {
		log_error("Error while handling %s: \"model\" parameter not provided", __FUNCTION__);
		tuyalink_report_action_rsp(context, NULL, 3);
		return 3;
	}
	sensor = cJSON_GetStringValue(sensorptr);

	cJSON *modelptr = cJSON_GetObjectItemCaseSensitive(inputParams, "model");
	if (modelptr == NULL) {
		log_error("Error while handling %s: \"model\" parameter not provided", __FUNCTION__);
		tuyalink_report_action_rsp(context, NULL, 4);
		return 4;
	}
	model = cJSON_GetStringValue(modelptr);

	struct ubus_context *ctx = (struct ubus_context *)context->config.user_data;
	char *ubusJson;
	get_esp_sensor(ctx, &ubusJson, port, pin, model, sensor);
	char resp[256];
	snprintf(resp, 256, "{\"actionCode\": \"devices\", \"outputParams\": { \"devices\": \"%s\"}}", ubusJson);
	if (tuyalink_report_action_rsp(context, resp, 0) < 0) {
		return -2;
	}
	free(ubusJson);

	return 0;
}

static const struct action supported_actions[] = { { .name = "devices", .handler = action_get_devices },
						   { .name = "on", .handler = action_on },
						   { .name = "off", .handler = action_off },
						   { .name = "get", .handler = action_get },
						   { .name = 0 } };

/*
 * Handles a given action by the `actionCode` object in the given JSON structure
 * Returns -1 if it fails to find the action code
 * Returns -2 if it fails to convert the action code to a string
 * Returns -3 if the given action was not found
 * Otherwise, returns the status code of the specific action handler
 */
int handle_action(tuya_mqtt_context_t *context, void *user_data, cJSON *data)
{
	cJSON *actionptr = cJSON_GetObjectItemCaseSensitive(data, "actionCode");
	if (actionptr == NULL) {
		log_error("Failed to find action code.");
		return -1;
	}
	char *action = cJSON_GetStringValue(actionptr);
	if (action == NULL) {
		log_error("Failed to get string value for the given action");
		return -2;
	}

	const struct action *current;
	for (int i = 0; supported_actions[i].name != 0; i++) {
		current = &supported_actions[i];
		(void)context;
		(void)user_data;
		if (strcmp(current->name, action) == 0) {
			log_info("Found action for %s", current->name);
			cJSON *inputParams = cJSON_GetObjectItemCaseSensitive(data, "inputParams");
			return current->handler(context, user_data, inputParams);
		}
	}
	log_info("Unknown action %s", action);
	return -3;
}
