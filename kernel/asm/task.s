extern current_task
extern current_thread
extern current_directory

global switch_context:function switch_context.end-switch_context
switch_context:
	mov eax, [current_thread]

	; save state
	mov [eax+0], esp
	mov [eax+4], ebp
	mov [eax+8], ebx
	mov [eax+12], esi
	mov [eax+16], edi

	pushf
	pop ecx
	mov [eax+20], ecx

	; switch thread (must be done now, because after that we may lose stack)
	mov eax, [esp+4]
	mov [current_thread], eax

	; get new task
	mov eax, [esp+8]
	; if new_task is null then don't switch
	cmp eax, 0
	je .thread

.task:
	; else switch to new task
	mov [current_task], eax

	; switch address space
	mov eax, [eax+0]
	mov [current_directory], eax
	; set page directory physical
	mov eax, [eax+0]
	mov cr3, eax						; (stack vapors now)

.thread:
	; get the new thread struct
	mov eax, [current_thread]

	; restore state (in first line we obtain proper stack)
	mov esp, [eax+0]
	mov ebp, [eax+4]
	mov ebx, [eax+8]
	mov esi, [eax+12]
	mov edi, [eax+16]

	mov eax, [eax+20]
	push eax
	popf

	; entry point of the new thread is current return address
	ret
.end:

global cpu_idle:function cpu_idle.end-cpu_idle
cpu_idle:
.loop:
	nop
	hlt
	jmp .loop
	ret
.end:

global read_eip:function read_eip.end-read_eip
read_eip:
	pop eax
	jmp eax
.end:
