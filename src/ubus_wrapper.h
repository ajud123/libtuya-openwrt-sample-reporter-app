#ifndef TUYAREPORTER_UBUS_WRAPPER
#define TUYAREPORTER_UBUS_WRAPPER
#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include "utils.h"
#include "net_interfaces.h"

int get_memory_statistics(struct ubus_context *ctx, struct memory_stats *out);
int get_network_statistics(struct ubus_context *ctx, struct if_list *out);

#endif