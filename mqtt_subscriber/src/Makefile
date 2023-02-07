CFLAGS:=-Wall
LDGLAGS:=-lmosquitto -luci -lcurl -ljson-c
SRCS:=$(wildcard *.c)
OBJS:=$(SRCS:.c=.o)
TARGET_EXEC:=mqtt_sub

.PHONY: all clean

all: $(TARGET_EXEC)

$(TARGET_EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDGLAGS) 

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^
	
clean:
	rm -f $(TARGET_EXEC) $(OBJS)
 
