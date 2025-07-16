BIN:=tuyareporter
LDFLAGS:= \
	-llink_core \
	-lmiddleware_implementation \
	-lplatform_port \
	-lutils_modules \
	-largp \
	-lblobmsg_json \
	-lubox \
	-lubus \
	-luci
CPPFLAGS:= -I$(CURDIR) -Wall -Wextra -Werror
SRC:= $(wildcard *.c)
OBJS:= $(SRC:.c=.o)

.PHONY:= all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(BIN)