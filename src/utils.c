#include "utils.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <log.h>
#include <syslog.h>
#include <cJSON.h>
#include <tuyalink_core.h>
#include <tuya_error_code.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/* 
 * Generates a JSON string from the given memory statistics struct
 */
char *generate_memory_json_string(struct memory_stats *stats)
{
	cJSON *root = cJSON_CreateObject();

	cJSON *totalRam = cJSON_CreateObject();
	cJSON_AddNumberToObject(totalRam, "value", stats->total);
	cJSON_AddItemToObjectCS(root, "totalRAM", totalRam);

	cJSON *freeRam = cJSON_CreateObject();
	cJSON_AddNumberToObject(freeRam, "value", stats->free);
	cJSON_AddItemToObjectCS(root, "freeRAM", freeRam);

	return cJSON_PrintUnformatted(root);
}

uint32_t system_timestamp(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint32_t)tv.tv_sec;
}

static uint32_t thing_send_msgid_get(tuya_mqtt_context_t *context)
{
	return context->msgid_inc++;
}

int tuyalink_report_action_rsp(tuya_mqtt_context_t *context, char *data, int errcode)
{
	if (context == NULL || data == NULL) {
		return OPRT_INVALID_PARM;
	}

	/* Device ID */
	const char *device_id = context->config.device_id;

/* Topic */
#define TOPIC_LEN_MAX (256)
	char topic_stuff[TOPIC_LEN_MAX];
	snprintf(topic_stuff, TOPIC_LEN_MAX, "tylink/%s/%s", device_id, "thing/action/execute_response");

	/* Make payload */
	size_t payload_length = 0;
	uint32_t msgid_int    = 0;
	size_t alloc_size     = 256;
	if (data) {
		alloc_size += strlen(data);
	}
	char *payload = malloc(alloc_size);
	if (payload == NULL) {
		return OPRT_MALLOC_FAILED;
	}

	/* JSON start  */
	payload_length = snprintf(payload, alloc_size, "{");

	/* msgId */
	msgid_int = thing_send_msgid_get(context);
	payload_length += snprintf(payload + payload_length, alloc_size - payload_length, "\"msgId\":\"%d\"",
				   msgid_int);

	/* time */
	payload_length += snprintf(payload + payload_length, alloc_size - payload_length, ",\"time\":%d",
				   system_timestamp());

	/* data */
	if (data != NULL) {
		payload_length +=
			snprintf(payload + payload_length, alloc_size - payload_length, ",\"data\":%s", data);
	}

	/* code */
	payload_length +=
		snprintf(payload + payload_length, alloc_size - payload_length, ",\"code\":%d", errcode);

	/* JSON end */
	payload_length += snprintf(payload + payload_length, alloc_size - payload_length, "}");

	log_debug("publish topic:%s", topic_stuff);
	log_debug("payload size:%d, %s\r\n", payload_length, payload);
	// Tuya's mqtt client for some reason takes in uint8_t pointer
	uint16_t mqmsgid = mqtt_client_publish(context->mqtt_client, topic_stuff, (uint8_t *)payload,
					       payload_length, MQTT_QOS_0);
	free(payload);
	if (mqmsgid <= 0) {
		return OPRT_LINK_CORE_MQTT_PUBLISH_ERROR;
	}
	return (int)msgid_int;
}

/*
 * Turns the process into a daemon
 * Returns -1 on failure
 * Returns 1 if the process should exit
 * Returns 0 if the process became a daemon
 */
int become_daemon()
{
	switch (fork()) {
	case -1:
		return -1;
	case 0:
		break;
	default:
		return 1;
	}

	if (setsid() == -1)
		return -1;

	switch (fork()) {
	case -1:
		return -1;
	case 0:
		break;
	default:
		return 1;
	}

	umask(0);
	return 0;
}

/*
 * Replaces all occurences of " with \" within a string.
 * @param input must be null-terminated
 */
char *escape_quotes(char *input)
{
	int maxlen   = strlen(input);
	char *output = malloc(2 * maxlen * sizeof(char) + 1);
	int inpos    = 0;
	int outpos   = 0;
	while (input[inpos] != '\0') {
		if (input[inpos] == '"') {
			output[outpos] = '\\';
			outpos++;
		}
                output[outpos] = input[inpos];
                outpos++;
		inpos++;
	}
        output[outpos] = '\0';
	return output;
}
