CC=i686-w64-mingw32-g++
BUILD_DIR=build
CFLAGS=-o $(BUILD_DIR)/keylogger.exe -s -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc

VISUAL= -Dvisual

$(shell mkdir -p $(BUILD_DIR))
.PHONY: build clean
build: keylogger.cpp
	$(CC) keylogger.cpp $(CFLAGS) $(VISUAL)

clean:
	-rm -rf $(BUILD_DIR)