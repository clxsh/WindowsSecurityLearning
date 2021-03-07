`NtMapUserPhysicalPages`和`DeviceIoControl`中间不要调用任何可能产生系统调用的函数。

## 参考

1. [j00ru//vx tech blog | nt!NtMapUserPhysicalPages and Kernel Stack-Spraying Techniques](https://j00ru.vexillium.org/2011/05/windows-kernel-stack-spraying-techniques/)