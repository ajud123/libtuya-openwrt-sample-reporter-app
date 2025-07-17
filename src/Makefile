BIN:=tuyareporter
LDFLAGS:= \
	-llink_core \
	-lmiddleware_implementation \
	-lplatform_port \
	-lutils_modules \
	-lblobmsg_json \
	-lubox \
	-lubus \
	-luci
CPPFLAGS:= -I$(CURDIR) -Wall -Wextra -Werror

ifdef DESKTOP_BUILD
	TUYA_SDK_LOCATION = ${shell realpath $$TUYA_SDK_LOCATION}
	CPPFLAGS += -I${TUYA_SDK_LOCATION}/include
	CPPFLAGS += -I${TUYA_SDK_LOCATION}/utils
	CPPFLAGS += -I${TUYA_SDK_LOCATION}/interface
	CPPFLAGS += -g
	LDFLAGS += -L${TUYA_SDK_LOCATION}/build/lib -Wl,-rpath,${TUYA_SDK_LOCATION}/build/lib
else
	LDFLAGS += -largp 

endif

SRC:= $(wildcard *.c)
OBJS:= $(SRC:.c=.o)

.PHONY:= all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(BIN)