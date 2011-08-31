extern current_thread

global switch_thread:function switch_thread.end-switch_thread
switch_thread:
	mov eax, [current_thread]

	mov [eax+0], esp
	mov [eax+4], ebp
	mov [eax+8], ebx
	mov [eax+12], esi
	mov [eax+16], edi

	pushf
	pop ecx
	mov [eax+20], ecx

	mov eax, [esp+4]

	mov [current_thread], eax

	mov esp, [eax+0]
	mov ebp, [eax+4]
	mov ebx, [eax+8]
	mov esi, [eax+12]
	mov edi, [eax+16]

	mov eax, [eax+20]
	push eax
	popf

	ret
.end:

global save_thread_state:function save_thread_state.end-save_thread_state
save_thread_state:
	mov eax, [esp+4]

	mov [eax+0], esp
	mov [eax+4], ebp
	mov [eax+8], ebx
	mov [eax+12], esi
	mov [eax+16], edi

	pushf
	pop ecx
	mov [eax+20], ecx

	ret
.end:

global load_thread_state:function load_thread_state.end-load_thread_state
load_thread_state:
	mov eax, [esp+4]

	mov esp, [eax+0]
	mov ebp, [eax+4]
	mov ebx, [eax+8]
	mov esi, [eax+12]
	mov edi, [eax+16]

	mov eax, [eax+20]
	push eax
	popf

	ret
.end:

