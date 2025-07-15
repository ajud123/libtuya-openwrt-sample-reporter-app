#include <stdlib.h>
#include <syslog.h>
#include <time.h>
#include <signal.h>
#include <tuyalink_core.h>
#include <tuya_error_code.h>
#include <libubus.h>
#include "utils.h"
#include "ubus_wrapper.h"
#include "tuya_wrapper.h"
#include "args.h"

static int state = 1;

void handle_exit(int signum)
{
        state = 0xFF;
        syslog(LOG_INFO, "Received signal %d, setting state to %d", signum, state);
}

int main(int argc, char **argv)
{
        char productId[128];
        char deviceId[128];
        char deviceSecret[128];
        int daemon = 0;
        
        openlog(NULL, LOG_PID | LOG_NDELAY, LOG_DAEMON);
	int args_state = parse_args(argc, &argv, productId, deviceId, deviceSecret, &daemon);
        if(args_state != 0){
                syslog(LOG_ERR, "Error when parsing arguments, code %d", args_state);
                closelog();
                return args_state;
        }

        signal(SIGINT, handle_exit);
	signal(SIGTERM, handle_exit);
	signal(SIGQUIT, handle_exit);
	signal(SIGABRT, handle_exit);


        if(daemon){
                int daemon_status = become_daemon();
                if (daemon_status < 0) {
                        syslog(LOG_ERR, "Failed to become a daemon");
                        return 1;
                } else if (daemon_status > 0) {
                        return 0;
                }
        }

        struct ubus_context *ctx = ubus_connect(NULL);
        if(ctx == NULL) {
                syslog(LOG_ERR, "Failed to connect to ubus");
        }
        tuya_mqtt_context_t client_instance;

        state = tuya_start(&client_instance, deviceSecret, deviceId);
        if(state != 0)
                syslog(LOG_ERR, "Failed to start Tuya client");

        time_t last_update = 0;
        struct memory_stats stats;
        
        while(state == OPRT_OK) {
                tuya_loop(&client_instance);
                time_t current = time(NULL);
                if(current - last_update > 10) {
                        last_update = current;
                        report_memory_info(ctx, &client_instance, &stats);
                }
        }
        syslog(LOG_INFO, "Stopping tuya reporter...");

        tuya_stop(&client_instance);
        ubus_free(ctx);
	closelog();
        return state;
}