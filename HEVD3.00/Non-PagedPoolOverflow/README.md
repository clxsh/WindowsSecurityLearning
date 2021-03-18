## 堆喷射

CreateEvent

先填满空隙

后分配连续空间

## Event pool Inspect

```
kd> dt nt!_POOL_HEADER 863a9140 
   +0x000 PreviousSize     : 0y000001000 (0x8)
   +0x000 PoolIndex        : 0y0000000 (0)
   +0x002 BlockSize        : 0y000001000 (0x8)
   +0x002 PoolType         : 0y0000010 (0x2)
   +0x000 Ulong1           : 0x4080008
   +0x004 PoolTag          : 0xee657645
   +0x004 AllocatorBackTraceIndex : 0x7645
   +0x006 PoolTagHash      : 0xee65
kd> dt nt!_OBJECT_HEADER_QUOTA_INFO 863a9140 + 8
   +0x000 PagedPoolCharge  : 0
   +0x004 NonPagedPoolCharge : 0x40
   +0x008 SecurityDescriptorCharge : 0
   +0x00c SecurityDescriptorQuotaBlock : (null) 
kd> dt nt!_OBJECT_HEADER 863a9140 + 8 + 10
   +0x000 PointerCount     : 0n1
   +0x004 HandleCount      : 0n1
   +0x004 NextToFree       : 0x00000001 Void
   +0x008 Lock             : _EX_PUSH_LOCK
   +0x00c TypeIndex        : 0xc ''
   +0x00d TraceFlags       : 0 ''
   +0x00e InfoMask         : 0x8 ''
   +0x00f Flags            : 0 ''
   +0x010 ObjectCreateInfo : 0x87bf6640 _OBJECT_CREATE_INFORMATION
   +0x010 QuotaBlockCharged : 0x87bf6640 Void
   +0x014 SecurityDescriptor : (null) 
   +0x018 Body             : _QUAD
```

有趣的是这里的TypeIndex(0xc)，0xc是一个数组索引值。这个数组叫作`ObTypeIndexTable`，这个数组被`_OBJECT_TYPE`这种类型填满。

```
kd> dd nt!ObTypeIndexTable
83f7f900  00000000 bad0b0b0 85c3c868 85c3c7a0
83f7f910  85c3c6d8 85c3c4a8 85cdaf78 85cdaeb0
83f7f920  85cdade8 85cdad20 85cdac58 85cda5a0
83f7f930  85d01418 85d01350 85cf6418 85cf6350
83f7f940  85cfd040 85cfd208 85cfd140 85d054f8
83f7f950  85d05430 85d05368 85cf1040 85cf1230
83f7f960  85cf1168 85ce5588 85ce54c0 85ce53f8
83f7f970  85cf76e0 85cf7340 85cfef78 85cfeeb0
```

第`0xc`项就是`85d01418`。

```
kd> dt nt!_OBJECT_TYPE 85d01418 -b
   +0x000 TypeList         : _LIST_ENTRY [ 0x85d01418 - 0x85d01418 ]
      +0x000 Flink            : 0x85d01418 
      +0x004 Blink            : 0x85d01418 
   +0x008 Name             : _UNICODE_STRING "Event"
      +0x000 Length           : 0xa
      +0x002 MaximumLength    : 0xc
      +0x004 Buffer           : 0x8ac055e0  "Event"
   +0x010 DefaultObject    : (null) 
   +0x014 Index            : 0xc ''
   +0x018 TotalNumberOfObjects : 0x5a20
   +0x01c TotalNumberOfHandles : 0x5a56
   +0x020 HighWaterNumberOfObjects : 0x5b79
   +0x024 HighWaterNumberOfHandles : 0x5bd5
   +0x028 TypeInfo         : _OBJECT_TYPE_INITIALIZER
      +0x000 Length           : 0x50
      +0x002 ObjectTypeFlags  : 0 ''
      +0x002 CaseInsensitive  : 0y0
      +0x002 UnnamedObjectsOnly : 0y0
      +0x002 UseDefaultObject : 0y0
      +0x002 SecurityRequired : 0y0
      +0x002 MaintainHandleCount : 0y0
      +0x002 MaintainTypeList : 0y0
      +0x002 SupportsObjectCallbacks : 0y0
      +0x004 ObjectTypeCode   : 2
      +0x008 InvalidAttributes : 0x100
      +0x00c GenericMapping   : _GENERIC_MAPPING
         +0x000 GenericRead      : 0x20001
         +0x004 GenericWrite     : 0x20002
         +0x008 GenericExecute   : 0x120000
         +0x00c GenericAll       : 0x1f0003
      +0x01c ValidAccessMask  : 0x1f0003
      +0x020 RetainAccess     : 0
      +0x024 PoolType         : 0 ( NonPagedPool )
      +0x028 DefaultPagedPoolCharge : 0
      +0x02c DefaultNonPagedPoolCharge : 0x40
      +0x030 DumpProcedure    : (null) 
      +0x034 OpenProcedure    : (null) 
      +0x038 CloseProcedure   : (null) 
      +0x03c DeleteProcedure  : (null) 
      +0x040 ParseProcedure   : (null) 
      +0x044 SecurityProcedure : 0x840a15b6 
      +0x048 QueryNameProcedure : (null) 
      +0x04c OkayToCloseProcedure : (null) 
   +0x078 TypeLock         : _EX_PUSH_LOCK
      +0x000 Locked           : 0y0
      +0x000 Waiting          : 0y0
      +0x000 Waking           : 0y0
      +0x000 MultipleShared   : 0y0
      +0x000 Shared           : 0y0000000000000000000000000000 (0)
      +0x000 Value            : 0
      +0x000 Ptr              : (null) 
   +0x07c Key              : 0x6e657645
   +0x080 CallbackList     : _LIST_ENTRY [ 0x85d01498 - 0x85d01498 ]
      +0x000 Flink            : 0x85d01498 
      +0x004 Blink            : 0x85d01498 
```

`-b` 选项打印出了所有级别的结构体。

偏移0x28处的 `TypeInfo` 结构非常有用，其中偏移0x38存储了`CloseProcedure`的函数指针（总体偏移就是0x28+0x38=0x60），当调用`CloseHandle`释放这些事件对象时就会被调用。

但是这个 `CloseProcedure` 没法被覆盖掉。回到 `ObTypeIndexTable` 可以看到他的第一项是 `00000000` ，而我们可以覆盖`_OBJECT_HEADER.TypeIndex` 为0，通过分配NULL page，在`0 + 0x60`偏移处放置我们的payload（StealToken函数的地址）。

（在前述 NULL pointer dereferenced 实验中可以知道，在Win7 x86中可以分配得到NULL page，Win7 x64中分配失败。）

## 利用思路

在连续的Event中隔一段距离，释放8个Event（0x200/0x40）

分配0x200的Non-Page pool时分配到Event中间

溢出覆盖后面Event

Close所有的Event

## 参考

1. [HEVD Exploits – Windows 7 x86 Non-Paged Pool Overflow](https://h0mbre.github.io/HEVD_Pool_Overflow_32bit/#)