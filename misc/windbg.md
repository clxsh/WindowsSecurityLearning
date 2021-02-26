## Windbg

### 安装

[WinDbg Preview/WinDbg](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools)

符号路径，设置环境变量`_NT_SYMBOL_PATH=cache*c:\MySymbols;srv*https://msdl.microsoft.com/download/symbols`(命令通知调试器使用符号服务器从位于 https://msdl.microsoft.com/download/symbols 的存储中获取符号，并将这些符号缓存在 `c:\MySymbols` 目录中。)

### 窗口界面

| 名称                  | 热键   | 用途                                     |
| --------------------- | ------ | ---------------------------------------- |
| Command               | Atl+1  | 输入命令、显示命令结果和调试信息输出     |
| Watch                 | Atl+2  | 观察指令全局变量、局部变量和寄存器的信息 |
| Locals                | Atl+3  | 自动显示当前函数的所有局部变量           |
| Registers             | Atl+4  | 观察和修改寄存器的值                     |
| Memory                | Atl+5  | 观察和修改内存数据                       |
| Call Stack            | Atl+6  | 栈中记录的函数调用序列                   |
| Disassembly           | Atl+7  | 反汇编                                   |
| Scratch Pad           | Atl+8  | 白板，可以用来做调试笔记等               |
| Processes and Threads | Atl+9  | 显示所有调试目标的列表，包括进程和线程等 |
| Command Browser       | Ctrl+N | 执行和浏览命令                           |

### WinDbg的命令分类

Windbg主要分为3大类的调试命令:

- 标准命令 (Standard Command): 这类命令对于所有的调试目标都试用，比如常见的`k`命令;
- 元命令 (Meta-Command)： 这类目标主要针对特定的目标所做的扩展命令，比如常见的`.sympath`命令。因为这类命令前面都有一个`.`,所以也叫作`Dot-Command`；
- 扩展命令 (Extension Command)：标准命令和元命令都是Windbg内建的命令，而扩展命令是实现在动态加载的DLL中。这类命令前面都有一个`!`, 比如常用`analyze -v`.

提一个很实用的命令`.hh`，用来在Windbg中打开帮助文档，比如使用`.hh k`则帮助文档会打开到索引`k`命令处。

```
tab键       自动完成命令
.hh         查看指定命令的帮助手册
.reload     重新加载符号文件 
.restart    重新启动调试目标 

bp $exentry 在程序入口点设置断点，$exentry是一个伪寄存器 
bp 0x00401030              在地址0x00401030处设置断点 
bp MyTestModule!MyTestFunc 在MyTestModule模块中的MyTestFunc函数处设置断点，前提是该模块符号已经加载 
bp MyTestModule!MyTestClass::SetValue        在模块MyTestModule的MyTestClass类成员函数SetValue处设置断点 
bp @@C++(MyTestModule!MyTestClass::SetValue) 与上面一样，语法不同，C++语法，上面的为MASM语法 
bl 查看设置的断点 
be 激活断点 
bd 禁用断点 
bc 删除某个断点 
ba              设置访问断点 
ba r 1 0044108c 在内存0044108c的位置开始的下一个字节的读断点 
ba w4@@C++(&i)  给变量i地址下4个字节的写断点 

g               运行程序，相当于F5 
gu              返回函数调用处，相当于shift+F11 
pt              运行到ret之前

u               查看当前正要执行的代码 
k               查看当前调用堆栈 
~*kb           显示所有进程调用堆栈

~           查看调试进程中的线程信息 
!teb        线程环境块
~.          当前线程信息
~#          导致当前异常或调试事件的线程信息
~[Number]s  线程切换

a                                修改当前指令，输入修改的指令按Enter结束 
s –a 00400000 L53000 “Wrong”     以ASCII码的形式从00400000处开始往后53000个字节搜索字符串“Wrong” 
db 400000       以二进制的方式显示内存地位为400000开始的内容 
dd 400000       以DWORD类型查看
dqp rsp
d               按上一次的d命令的方式来显示，如果不带参数，则从上一次显示结束的地方继续显示 
?i              查看局部变量i的值，会以10进制和16进制同时显示 
eb 0012ff78 'a' 'b'      从内存地址0012ff78开始依次写入后面的值 
r                        用于查看或者修改寄存器或伪寄存器 
r $peb                   $peb是一个伪寄存器，调试器将它定义为当前进程的进程环境块地址 
dt                       用于查看结构体内容 
!address 400000          查看指定内存地址的信息 
dv                       查看当前作用域下局部变量的类型和值

.ecxr                    当前异常的上下文信息
!analyze -v              详细显示当前异常信息，常用于分析dmp文件

|             所有进程列表
|.            当前进程信息
|#            导致当前异常或调试事件的进程信息
|[Number]s    进程切换
!peb          进程环境块

!locks              查看进程中有些锁处于锁定状态
!cs -l              查看处于锁定状态的关键区
!handle 000000c0 f  查看句柄000000c0的信息
```

`bp $exentry;g;bp kernel32!GetCommandlineA;g;g poi(@esp)`直接到`GETCommandlineA`函数后面，‘main’就在近处。

[wangray/WinDBG-for-GDB-users](https://github.com/wangray/WinDBG-for-GDB-users)

https://blog.csdn.net/china_jeffery/article/details/78966483

https://blog.csdn.net/CJF_iceKing/article/details/51955540

https://www.cnblogs.com/kekec/archive/2012/11/14/2755924.html

## useful command

```
dqp rsp
dqs rsp
dt
pt
gu
ld
ln
```

## TODO

1. 内核调试中，调试用户态程序