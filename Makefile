# If variables aren't define, define them
CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
LDFLAGS ?= -lpthread -lrt

# Project specific flags
TARGET ?= snoop
SOURCES = snoop.c
INCLUDES = -I. -Iinclude

all:
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDES) ${SOURCES} -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o
