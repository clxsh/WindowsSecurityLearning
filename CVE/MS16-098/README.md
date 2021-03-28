## Patch

get from [microsoft bulletins](https://docs.microsoft.com/en-us/security-updates/securitybulletins/2016/ms16-098)

cmd 里执行 expand 释放补丁包

```
expand -F:* Windows8.1-KB3177725-x64.msu ./extract
expand -F:* Windows8.1-KB3177725-x64.cab .
```

可以得到 patch 过的 win32k.sys 文件

## Bindiff

**不知道如何快速定位问题函数 `bFill`，只能一个个看？**

![bFill](https://raw.githubusercontent.com/clxsh/pics/master/img/bFill.png)

从左侧函数可以看出，首先将 `ECX = RAX * 3` ，后左移4位也就是 `ECX = ECX * 0x10`。总的来看就是 `ECX = RAX * 0x30`

右侧使用 `ULongMult` 进行前述的操作，`ULongMult` 函数如下，可以看到增加了溢出的检测。合理推测这里修复了溢出的问题。

![ULongMultisim](https://raw.githubusercontent.com/clxsh/pics/master/img/ULongMultisim.png)

## Reaching the Vulnerable Function

Xrefs + reverse + keywords search

## Controlling the Allocation Size

Investigate

## Kernel Pool Feng Shui

![img](https://sensepost.com/img/pages/blog/2017/exploiting-ms16-098-rgnobj-integer-overflow-on-windows-8.1-x64-bit-by-abusing-gdi-objects/animation.gif)

size

```c
// HBITMAP test = CreateBitmap(0x200, 0x01, 1, 8, NULL);    // 0x260 + 0x200 + 0x10

// HACCEL hAccel = CreateAcceleratorTableA(lpAccel, 1);     // 0x40

// HACCEL hAccel = CreateAcceleratorTableA(lpAccel, 2);     // 0x40

// HACCEL hAccel = CreateAcceleratorTableA(lpAccel, 3);     // 0x50

// HACCEL hAccel = CreateAcceleratorTableA(lpAccel, 7);     // 0x60

// HACCEL hAccel = CreateAcceleratorTableA(lpAccel, 8);     // 0x70

// HACCEL hAccel = CreateAcceleratorTableA(lpAccel, 9);     // 0x70

// HACCEL hAccel = CreateAcceleratorTableA(lpAccel, 10);    // 0x70

// HACCEL hAccel = CreateAcceleratorTableA(lpAccel, 11);    // 0x80
```

## Abusing the Bitmap GDI objects

```c
typedef struct{
 BASEOBJECT64 BaseObject; // 0x18bytes
 SURFOBJ64 SurfObj; 
 ....... 
} SURFACE64

typedef struct {
 ULONG64 hHmgr; // 8bytes
 ULONG32 ulShareCount; // 4bytes
 WORD cExclusiveLock; // 2bytes
 WORD BaseFlags; // 2bytes
 ULONG64 Tid; // 8bytes
} BASEOBJECT64;

typedef struct {
  ULONG64 dhsurf; // 0x00
  ULONG64 hsurf; // 0x08
  ULONG64 dhpdev; // 0x10
  ULONG64 hdev; // 0x18
  SIZEL sizlBitmap; // 0x20
  ULONG64 cjBits; // 0x28
  ULONG64 pvBits; // 0x30
  ULONG64 pvScan0; // 0x38
  ULONG32 lDelta; // 0x40
  ULONG32 iUniq; // 0x44
  ULONG32 iBitmapFormat; // 0x48
  USHORT iType; // 0x4C
  USHORT fjBitmap; // 0x4E
} SURFOBJ64; // sizeof = 0x50
```

 `sizlBitmap`, `pvScan0`, `hdev`

```c
#include <Windows.h>
#include <stdio.h>

int main()
{
    HBITMAP hBitmap = CreateBitmap(0xE30, 0x1, 1, 8, NULL);
    
    __debugbreak();
    return 0;
}
```

使用 [gdiobjdump](https://github.com/panoramixor/GDIObjDump) 插件

```
kd> !load gdiobjdump
kd> dv hBitmap
           hBitmap = 0x00000000`2e0502fa
kd> !gdiobjdump -h 2e0502fa
Searching for handle 2E0502FA...
NTDLL Base: 00007ffd6d6b0000
PEB: 00007ff7f2589000
Pid: 0b08
Imagename C:\Users\liu\Desktop\MS16-098.exe
GdiSharedHandleTable:		fffff90140210000 (Kernel)
					0000008d501f0000 (User)
entries found: 00000a00
retries: 00000051
GDI_TABLE_ENTRY:
	pKernelAddress: fffff901400d0000
	wProcessId: 00000b08
	wCount: 0000
	wUpper: 2e05
	wType: 4005 (GDIObjType_SURF_TYPE)
	pUserAddress: 0000000000000000

SURFOBJ at
	dhsurf:0000000000000000
	hsurf:000000002e0502fa
	dhpdev:0000000000000000
	hdev:0000000000000000
	sizlBitmap: (X)00000e30 (Y)00000001
	cjBits: 0000000000000e30
	pvBits: fffff901400d0260
	pvScan0: fffff901400d0260
	lDelta: 00000e30
	iUniq: 00001d78
	iBitmapFormat: 00000003 (BMF_8BPP)
	iType: 0000 (STYPE_BITMAP)
	fjBitmap: 0001 (BMF_TOPDOWN)

kd> dq fffff901400d0000
fffff901`400d0000  00000000`2e0502fa 00000000`00000000
fffff901`400d0010  00000000`00000000 00000000`00000000
fffff901`400d0020  00000000`2e0502fa 00000000`00000000
fffff901`400d0030  00000000`00000000 00000001`00000e30
fffff901`400d0040  00000000`00000e30 fffff901`400d0260
fffff901`400d0050  fffff901`400d0260 00001d78`00000e30
fffff901`400d0060  00010000`00000003 00000000`00000000
fffff901`400d0070  00000000`04800200 00000000`00000000
kd> !pool fffff901400d0000
Pool page fffff901400d0000 region is Unknown
fffff901400d0000 is not a valid large pool allocation, checking large session pool...
*fffff901400d0000 : large page allocation, tag is Gh05, size is 0x1090 bytes
		Pooltag Gh05 : GDITAG_HMGR_SURF_TYPE, Binary : win32k.sys
```

可以看到前18字节是`BASEOBJECT64`，后面是`SURFOBJ64`，Windows 8.1 x64 的bitmap前0x260字节是管理区域，后0xE30是像素点数据区域。

![bitmap_struct](https://raw.githubusercontent.com/clxsh/pics/master/img/bitmap_struct.png)

## Analysing and Controlling the Overflow

利用 `AddEdgeToGET` 函数覆盖

触发路径： `bFill` -> `bConstructGET` -> `AddEdgeToGET`

看到这里感觉看不懂了，转去搜索 `struct EPATHOBJ struct edge` 找到文章[2]，看起来漏洞原因与利用方式一模一样！！。。

`NT5 fillpath.c` **如何找到该文件？**

## Fixing the Overflowed Header

## Stealing SYSTEM Process Token from the EPROCESS structure

## 参照

1. [Exploiting MS16-098 RGNOBJ Integer Overflow on Windows 8.1 x64 bit by abusing GDI objects --- SensePost](https://sensepost.com/blog/2017/exploiting-ms16-098-rgnobj-integer-overflow-on-windows-8.1-x64-bit-by-abusing-gdi-objects/)
2. [从 CVE-2016-0165 说起：分析、利用和检测（上）](https://xiaodaozhi.com/exploit/32.html)
3. [从 CVE-2016-0165 说起：分析、利用和检测（中）](https://xiaodaozhi.com/exploit/42.html)
4. [从 CVE-2016-0165 说起：分析、利用和检测（下）](https://xiaodaozhi.com/exploit/56.html)
5. [图形设备接口子系统的对象解释](https://xiaodaozhi.com/win32k-gdi-object.html)
6. [Abusing GDI for ring0 exploit primitives](https://www.coresecurity.com/core-labs/articles/abusing-gdi-for-ring0-exploit-primitives)