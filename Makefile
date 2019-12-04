CC=gcc
CCFLAGS=-Wall -g
LDFLAGS=

SOURCES_CLIENT=$(wildcard client/*.c)
TARGET_CLIENT=ftp-client
OBJECTS_CLIENT=$(SOURCES_CLIENT:.c=.o)

SOURCES_SERVER=$(wildcard server/*.c)
TARGET_SERVER=ftp-server
OBJECTS_SERVER=$(SOURCES_SERVER:.c=.o)

COMPNAME=nicolas_boeckh_ftp

all: $(TARGET_CLIENT) $(TARGET_SERVER)

$(TARGET_CLIENT): $(OBJECTS_CLIENT)
		$(CC) -o client/$@ $^ $(LDFLAGS)

$(TARGET_SERVER): $(OBJECTS_SERVER)
		$(CC) -o server/$@ $^ $(LDFLAGS)

client/%.o: %.c %.h
		$(CC) $(CCFLAGS) -c $<

server/%.o: %.c %.h
		$(CC) $(CCFLAGS) -c $<

client/%.o: %.c
		$(CC) $(CCFLAGS) -c $<

server/%.o: %.c
		$(CC) $(CCFLAGS) -c $<

clean:
		rm -f */*.o client/$(TARGET_CLIENT) server/$(TARGET_SERVER)

wc: 
		wc -l */*.c */*.h

zip:
		zip $(COMPNAME).zip */*.c */*.h Makefile README.md

delzip:
		rm $(COMPNAME).zip
