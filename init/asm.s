
global cpu_idle:function cpu_idle.end-cpu_idle
cpu_idle:
.loop:
	nop
	hlt
	jmp .loop
	ret
.end:
