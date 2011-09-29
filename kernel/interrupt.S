.section asm_functions
.global asm_handle_swi
.ident "human compiler (TM)"
@ this code handles swi system call
asm_handle_swi:
	@ load the pointer to the register
	stmfd sp, {r0, r1}
	ldr r0, [sp]

	@ save spsr
	mrs r1, spsr
	str r1, [r0]
	add r0, r0, #4

	@ save all the usermode register except r0 and r1
	stmfa r0, {r0-r12, sp, lr}^
	add r0, r0, #60
	stmfa r0!, {lr}

	ldr r2, [sp]
	ldr r0, [sp,#-8]
	str r0, [r2,#4]
	ldr r1, [sp,#-4]
	str r1, [r2,#8]

	@ put the swi argument in r0
	ldr r0, [lr,#-4]
	bic r0, r1, #0xff000000

	add sp, sp, #8

	bx lr


@ this code switches to the usermode
@ r0 the pointer to the stack pointer
@ r1 the entry point
.global asm_switch_to_usermode
asm_switch_to_usermode:
	@ i save r0 and lr
	stmfd sp!, {r0, lr}

	@ load spsr and save spsr
	mrs r1, cpsr
	bic r1, r1, #0xf
	orr r1, r1, #0x10
	@ldr r1, [r0]
	msr cpsr, r1

	add lr, r0, #64

	@ load the usermode stuff and say byebye

	ldmfd lr, {r0-r12, sp, lr, pc}^
	@ldr lr, [lr,#4]
	@mov pc, lr


@ syscall code.
@ r0 : request id
@ r1 : arguments array
.global asm_syscall
asm_syscall:
	swi 0
	@return
	mov pc, lr

