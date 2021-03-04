## 内核模式堆(亦称pool)

内核堆主要分为两种类型：Non-Paged pool，Paged Pool

1. Non-paged Pool：保证一直在内存中，可能在硬件中断时被访问（此时系统没法处理页访问错误）。可以通过`ExAllocatePoolWithTag`分配这种内存。
2. Page Pool：普通的内存页，可被换入换出

[PoolMon](https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/poolmon) 工具可以用来监控堆分配。

## NULL Page Dereference 漏洞的缓解

- EMET(Enhanced Mitigation Experience Toolit)简单的分配了NULL page并标记为`NOACCESS`。EMET弃用了，一部分被整合到Windows 10中的Exploit Protection中。
- 自Windows 8开始，禁止分配前64K 内存。除非启用了NTVDM

## Windows 7 SP1 7601

利用失败，无法分配NULL page

## 参考

1. [abatchy's blog | [Kernel Exploitation\] 6: NULL pointer dereference](https://www.abatchy.com/2018/01/kernel-exploitation-6)