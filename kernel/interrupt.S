.section asm_functions
.global asm_handle_swi
.ident "human compiler (TM)"
@ this code handles swi system call
asm_handle_swi:
	stmfd sp, {r0, r1}

	ldr r0, [sp]
	bl dump_registers

	ldr lr, [sp,#4]
	add sp, sp, #4
	bx lr

	ldr r0, [sp]
	mrs r1, spsr
	str r1, [r0]
	add r0, r0, #64
	stmfa r0, {r0-r12, sp, lr}^
	str lr, [r0, #4]

	ldr r0, [sp]
	ldr r1, [sp,#-8]
	str r1, [r0,#4]
	ldr r1, [sp,#-4]
	str r1, [r0,#8]

	ldr lr, [sp,#4]
	add sp, sp, #4
	bx lr


@ this code switches to the usermode
@ r0 the pointer to the stack pointer
.global asm_switch_to_usermode
asm_switch_to_usermode:

	@ start testing
	mov r0, #0
	mov r1, #1
	mov r2, #2
	mov r3, #3
	mov r4, #ff
	stmfd sp, {r0-r3}
	ldr r3, [sp, #-4]
	ldr r2, [sp, #-8]
	ldr r1, [sp, #-12]
	ldr r0, [sp, #-16]
	bl dump_registers(PLT)
	@ end testing


	@ i save r0 and lr
	stmfd sp!, {r0, lr}
	ldr r1, [r0]
	msr spsr, r1
	add r0, r0, #64
	ldmfa r0, {r0-r12, sp, lr, pc}^


@ syscall code.
@ r0 : request id
@ r1 : arguments array
.global asm_syscall
asm_syscall:
	swi 0
	@return
	mov pc, lr

