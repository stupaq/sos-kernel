MBOOT_PAGE_ALIGN    equ 1<<0    ; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1    ; Provide kernel with memory info
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

bits 32

section .text

global mboot

extern code
extern bss
extern end

mboot:
	dd MBOOT_HEADER_MAGIC
	dd MBOOT_HEADER_FLAGS
	dd MBOOT_CHECKSUM

	dd mboot

	dd  code			; Start of kernel '.text' (code) section.
	dd  bss				; End of kernel '.data' section.
	dd  end				; End of kernel.
	dd  start			; Kernel entry point (initial EIP).

global start
extern kmain

start:
    mov esp, stack		; set up stack
    push esp			; push a pointer to kernel stack
    push ebx			; push a pointer to the multiboot info structure
    mov ebp, 0			; ebp=0 here we terminate stack traces
    call kmain

section .bss
    resb 131072			; stack size

stack:
