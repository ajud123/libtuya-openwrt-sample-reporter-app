#include "ubus_wrapper.h"
#include "utils.h"
#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <syslog.h>

static void esp_generic_callback(struct ubus_request *req, int type, struct blob_attr *msg)
{
	(void)type; // Unused variable
        syslog(LOG_ERR, "Got method data.");
	char **json = (char **)req->priv;
        char *tmp;
        tmp = blobmsg_format_json(msg, true);
        // *json = blobmsg_format_json(msg, true);
        syslog(LOG_ERR, "Returning formatted json...");
        *json = escape_quotes(tmp);
        free(tmp);
}

int set_esp_pin_state(struct ubus_context *ctx, char **out, char *port, int pin, char *state)
{
        syslog(LOG_INFO, "Seting ESP device pin state via Ubus...");
	uint32_t id = 0;
	int rc	    = ubus_lookup_id(ctx, "esp", &id);
	if (rc) {
		syslog(LOG_ERR, "Could not find object 'esp'\n");
		return rc;
	}

        struct blob_buf b   = {};
	blob_buf_init(&b, 0);
        blobmsg_add_string(&b, "port", port);
        blobmsg_add_u32(&b, "pin", pin);


	rc = ubus_invoke(ctx, id, state, b.head, esp_generic_callback, out, 3000);
        blob_buf_free(&b);
        if(*out == NULL)
                return -1;
	return rc;
}

int set_esp_on(struct ubus_context *ctx, char **out, char *port, int pin)
{
        return set_esp_pin_state(ctx, out, port, pin, "on");
}

int set_esp_off(struct ubus_context *ctx, char **out, char *port, int pin)
{
        return set_esp_pin_state(ctx, out, port, pin, "off");
}

int get_esp_sensor(struct ubus_context *ctx, char **out, char *port, int pin, char *model, char *sensor)
{
        syslog(LOG_INFO, "Querying ESP device sensor state via Ubus...");
	uint32_t id = 0;
	int rc	    = ubus_lookup_id(ctx, "esp", &id);
	if (rc) {
		syslog(LOG_ERR, "Could not find object 'esp'\n");
		return rc;
	}

        struct blob_buf b   = {};
	blob_buf_init(&b, 0);
        blobmsg_add_string(&b, "port", port);
        blobmsg_add_u32(&b, "pin", pin);
        blobmsg_add_string(&b, "model", model);
        blobmsg_add_string(&b, "sensor", sensor);


	rc = ubus_invoke(ctx, id, "get", b.head, esp_generic_callback, out, 3000);
        blob_buf_free(&b);
        if(*out == NULL)
                return -1;
	return rc;

}

int get_esp_devices_json(struct ubus_context *ctx, char **out)
{
        syslog(LOG_INFO, "Querying ESP devices via Ubus...");
	uint32_t id = 0;
	int rc	    = ubus_lookup_id(ctx, "esp", &id);
	if (rc) {
		syslog(LOG_ERR, "Could not find object 'esp'\n");
		return rc;
	}
	rc = ubus_invoke(ctx, id, "devices", NULL, esp_generic_callback, out, 3000);
        if(*out == NULL)
                return -1;
	return rc;
}
