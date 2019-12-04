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

%.o: %.c %.h
		$(CC) $(CCFLAGS) -c $<

%.o: %.c
		$(CC) $(CCFLAGS) -c $<

clean:
		rm -f *.o $(TARGET_CLIENT) $(TARGET_SERVER)

wc: 
		wc -l *.c *.h

zip:
		zip $(COMPNAME).zip *.c *.h Makefile README.md

tar:
		tar -cvzf $(COMPNAME).tar *.c *.h Makefile README.md

delzip:
		rm $(COMPNAME).zip

deltar: 
		rm $(COMPNAME).tar
