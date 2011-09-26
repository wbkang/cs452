.section asm_functions
.global asm_handle_swi
.ident "human compiler (TM)"
@ this code handles swi system call
asm_handle_swi:
	@ save the usermode crap
	stmfd sp, {r0-r12,sp, lr}^
	sub sp, sp, #60

	@ r0 contains the pointer to the top of the register dump
	mov r0, sp

	@ save lr_svc
	stmfd sp!, {lr}

	@ put the swi argument in r1
	ldr r1,[lr,#-4]
	bic r1, r1, #0xff000000

	@ call function
	bl handle_swi

	@ load lr_svc
	ldmfd sp!, {lr}

	@ load the usermode stuff and byebye
	ldmfd sp, {r0-r12, sp, lr}^
	add sp, sp, #60
	movs pc, lr

@ this code switches to the usermode
@ r0 addr
@ r1 mem
@ asm_usermode:


@ syscall code.
@ r0 : request id
@ r1 : arguments array
.global asm_syscall
asm_syscall:
	swi 0
	@return
	mov pc, lr
