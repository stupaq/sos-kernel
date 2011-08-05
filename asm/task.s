
global system_hung:function system_hung.end-system_hung
system_hung:
	cli
	hlt
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
