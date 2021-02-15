## win7

为了让程序继续正常运行，运行完payload需要进行栈帧平衡。即`BufferOverflowStackIoctlHandler`函数结尾那样。

```assembly
add rsp, 28h
ret
```

## Q&A

1. WinDbg(调试机中的DebugView) 不显示 `DbgPrint` 的信息

   在被调试机中的DebugView程序中，选中Capture->Enable Verbose Kernel Output，两侧就都能显示了。（会打印过多的信息！）

2. HEVD 驱动调试符号载入

   添加HEVD.pdb的路径。而后在Debug->Modules中找到并选中HEVD，然后点击reload。关闭该窗口后会显示其执行了如下命令

   ```
   .reload /f @"HEVD.sys"
   ```




## Ref

1. [Token Stealing Payload 注解](https://www.abatchy.com/2018/01/kernel-exploitation-2)
2. VS Studio 添加ASM [链接1](http://lallouslab.net/2016/01/11/introduction-to-writing-x64-assembly-in-visual-studio/) [链接2](https://zhuanlan.zhihu.com/p/31918676)

