; boot.asm
[org 0x7C00]        ; BIOS loads the boot sector to physical 0x7C00
bits 16             ; Real mode: 16-bit instructions

start:
    cli                     ; No interrupts until segments/stack are sane
    xor ax, ax
    mov ds, ax              ; Zero data segment
    mov es, ax              ; Zero extra segment
    mov ss, ax              ; Zero stack segment
    mov sp, 0x7C00          ; Stack grows down from boot area

    mov [boot_drive], dl    ; Remember BIOS drive number (passed in DL)

    mov bx, KERNEL_LOAD_ADDR ; ES:BX -> destination (ES=0)
    mov ah, 0x02            ; BIOS disk read
    mov al, KERNEL_SECTORS  ; Number of sectors to read
    mov ch, 0x00            ; Cylinder 0
    mov cl, 0x02            ; Sector 2 (sector 1 is this bootloader)
    mov dh, 0x00            ; Head 0
    mov dl, [boot_drive]    ; Drive number
    int 0x13                ; Read from disk into memory
    jc disk_error           ; Halt if disk read failed

    in al, FAST_A20_PORT    ; Fast A20 gate enable (avoid wrap at 1MB)
    or al, FAST_A20_ENABLE_BIT
    out FAST_A20_PORT, al

    lgdt [gdt_descriptor]   ; Load flat code/data segments
    mov eax, cr0
    or eax, CR0_PE_BIT      ; Set PE bit -> protected mode
    mov cr0, eax
    jmp CODE_SEL:protected_mode ; Far jump flushes pipeline into 32-bit

disk_error:
    hlt                     ; Stop forever on disk failure
    jmp disk_error

bits 32
protected_mode:
    mov ax, DATA_SEL        ; Use flat data segment for all selectors
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, STACK_TOP      ; Set up 32-bit stack
    jmp KERNEL_LOAD_ADDR    ; Continue at kernel entry (loaded earlier)

bits 16

KERNEL_LOAD_ADDR equ 0x1000    ; Physical load address for kernel
KERNEL_SECTORS   equ 16        ; How many sectors to read
STACK_TOP        equ 0x90000   ; 32-bit stack top (below 1MB)

FAST_A20_PORT       equ 0x92   ; Fast A20 gate control port
FAST_A20_ENABLE_BIT equ 0x02   ; Bit to enable A20

CODE_SEL equ 0x08             ; GDT selector for code
DATA_SEL equ 0x10             ; GDT selector for data

GDT_CODE_DESC equ 0x00CF9A000000FFFF ; 32-bit flat code segment
GDT_DATA_DESC equ 0x00CF92000000FFFF ; 32-bit flat data segment

CR0_PE_BIT equ 0x1

gdt_start:
    dq 0x0000000000000000    ; Null descriptor (required)
    dq GDT_CODE_DESC         ; Code segment
    dq GDT_DATA_DESC         ; Data segment
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

boot_drive: db 0

times 510-($-$$) db 0
dw 0xAA55           ; boot signature