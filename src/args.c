#include <argp.h>
#include <string.h>
const char *argp_program_version     = "Daemon Program 1.0";
const char *argp_program_bug_address = "audrius.bertasius@teltonika.lt";

static char doc[] =
	"Daemon program -- a program that communicates with Tuya IoT cloud to report system information";

static char args_doc[] = "";

static struct argp_option options[] = { { "product", 'p', "PRODUCT_ID", 0, "Set product ID", 0 },
					{ "device", 'd', "DEVICE_ID", 0, "Set device ID", 0 },
					{ "secret", 's', "DEVICE_SECRET", 0, "Set device secret", 0 },
					{ "daemon", 'b', 0, OPTION_ARG_OPTIONAL,
					  "Run this program as a daemon", 0 },
					{ 0 } };

/* Used by main to communicate with parse_opt. */
static struct arguments {
	char productId[128];
	char deviceId[128];
	char deviceSecret[128];
	int daemon, missingArgs;
}arguments;

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	/* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
	struct arguments *arguments = state->input;

	switch (key) {
	case 'p':
		if (arg != NULL)
			strncpy(arguments->productId, arg, 128);
		break;
	case 'd':
		if (arg != NULL)
			strncpy(arguments->deviceId, arg, 128);
		break;
	case 's':
		if (arg != NULL)
			strncpy(arguments->deviceSecret, arg, 128);
		break;
	case 'b':
		arguments->daemon = 1;
		break;

	case ARGP_KEY_END:
		if (arguments->productId[0] == 0 || arguments->deviceId[0] == 0 || arguments->deviceSecret[0] == 0){
                        argp_failure(state, 1, 0, "Required -p -d and -s. See --help for more info.");
                        arguments->missingArgs = 1;
                }
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };

int parse_args(int argc, char ***argv, char *productId, char *deviceId, char *deviceSecret, int *daemon)
{
	arguments.daemon = 0;
	arguments.missingArgs = 0;
	error_t retval = argp_parse(&argp, argc, *argv, ARGP_NO_EXIT, 0, &arguments);
        if(arguments.missingArgs != 0){
                return -1;
        }
        strncpy(productId, arguments.productId, 127);
        strncpy(deviceId, arguments.deviceId, 128);
        strncpy(deviceSecret, arguments.deviceSecret, 128);
        *daemon = arguments.daemon;
        return retval;
}