MBOOT_PAGE_ALIGN    equ 1<<0		; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1		; Provide kernel with memory info
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
	; load a GDT with a base address of KERNEL_HIGHMEM
	; for the code (0x08) and data (0x10) segments
	lgdt [trickgdt]
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	jmp 0x08:higherhalf	; jump to the higher half kernel

higherhalf:
    mov esp, stack		; set up stack
    push stack_bottom	; push a pointer to kernel stack bottom
    push esp			; push a pointer to kernel stack (top)
    push ebx			; push a pointer to the multiboot info structure
    mov ebp, 0			; ebp=0 here we terminate stack traces
    call kmain


section .setup
trickgdt:
	dw gdt.end - gdt - 1	; size of the GDT
	dd gdt					; linear address of GDT

gdt:
	dd 0, 0										; null gate
	db 0xFF, 0xFF, 0, 0, 0, 0x9A, 0xCF, 0x1	; code selector 0x08: base 0x010000000, limit 0xFFFFFFFF, type 0x9A, granularity 0xCF
	db 0xFF, 0xFF, 0, 0, 0, 0x92, 0xCF, 0x1	; data selector 0x10: base 0x010000000, limit 0xFFFFFFFF, type 0x92, granularity 0xCF
.end:


section .bss
stack_bottom:
    resb 131072			; stack size
stack:
