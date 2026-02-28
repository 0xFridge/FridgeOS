[bits 32]           ; Executing in 32-bit protected mode

global _start        ; Entry point referenced by linker/bootloader
extern kmain         ; C++ kernel main

_start:
    call kmain       ; Hand off to kernel main (never returns)
.hang:
    hlt              ; If it returns, halt the CPU
    jmp .hang        ; Stay halted forever
