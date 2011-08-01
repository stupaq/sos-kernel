global gdt_flush:function gdt_flush.end-gdt_flush
gdt_flush:
    mov eax, [esp+4]	; Get the pointer to the GDT, passed as a parameter.
    lgdt [eax]			; Load the new GDT pointer

    mov ax, 0x10		; 0x10 is the offset in the GDT to our data segment
    mov ds, ax			; Load all data segment selectors
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush		; 0x08 is the offset to our code segment,
    					; segmentation already works (code is in other place)
.flush:
    ret
.end:

global tss_flush:function tss_flush.end-tss_flush
tss_flush:
    mov ax, 0x2B		; Load the index of our TSS structure - The index is
						; 0x28, as it is the 5th selector and each is 8 bytes
						; long, but we set the bottom two bits (making 0x2B)
						; so that it has an RPL of 3, not zero.
    ltr ax				; Load 0x2B into the task state register.
    ret
.end: