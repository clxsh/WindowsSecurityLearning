## 漏洞

```c
int __stdcall TriggerIntegerOverflow(void *UserBuffer, unsigned int Size)
{
  unsigned int v2; // esi
  unsigned int *v3; // edi
  int result; // eax
  unsigned int KernelBuffer[512]; // [esp+10h] [ebp-820h] BYREF
  unsigned int Count; // [esp+810h] [ebp-20h]
  CPPEH_RECORD ms_exc; // [esp+818h] [ebp-18h]

  v2 = 0;
  memset(KernelBuffer, 0, sizeof(KernelBuffer));
  ms_exc.registration.TryLevel = 0;
  v3 = (unsigned int *)UserBuffer;
  ProbeForRead(UserBuffer, 0x800u, 1u);
  _DbgPrintEx(0x4Du, 3u, "[+] UserBuffer: 0x%p\n", UserBuffer);
  _DbgPrintEx(0x4Du, 3u, "[+] UserBuffer Size: 0x%X\n", Size);
  _DbgPrintEx(0x4Du, 3u, "[+] KernelBuffer: 0x%p\n", KernelBuffer);
  _DbgPrintEx(0x4Du, 3u, "[+] KernelBuffer Size: 0x%X\n", 2048);
  _DbgPrintEx(0x4Du, 3u, "[+] Triggering Integer Overflow (Arithmetic Overflow)\n");
  if ( Size + 4 <= 0x800 )         // vulnerability
  {
    while ( v2 < Size >> 2 && *v3 != -1160728400 )
    {
      KernelBuffer[v2] = *v3++;
      Count = ++v2;
    }
    result = 0;
  }
  else
  {
    _DbgPrintEx(0x4Du, 3u, "[-] Invalid UserBuffer Size: 0x%X\n", Size);
    ms_exc.registration.TryLevel = -2;
    result = -1073741306;
  }
  return result;
}
```

当用户传入的Size在0xfffffffc~0xffffffff之间时，就会发生溢出，造成校验通过。

## 问题

x64环境下没法复现，因为`DeviceIoControl`只能传递`DWORD`类型的长度，而在x64的HEVD中使用的64bit寄存器进行的数字比较。不可能发生整形溢出。（就不再单独配环境复现了（躺

