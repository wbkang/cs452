.section asm_functions
.global asm_handle_swi
.ident "human compiler (TM)"
@ this code handles swi system call
asm_handle_swi:
	@ save the usermode pc
	stmfd sp!, {lr}

	@ save the usermode crap
	stmfd sp, {r0-r12, sp, lr}^
	sub sp, sp, #60

	@ save the spsr as well
	mrs r0, spsr
	stmfd sp!, {r0}

	@ r0 contains the pointer to the top of the register dump (ordered spsr, r0-r14)
	mov r0, sp

	@ put the swi argument in r1
	ldr r1, [lr,#-4]
	bic r1, r1, #0xff000000

	@ call function
	bl handle_swi

	@ load spsr and save spsr
	ldmfd sp!, {r0}
	msr spsr, r0


	@ load the usermode stuff and say byebye
	ldmfd sp, {r0-r12, sp, lr}^
	add sp, sp, #60
	ldmfd sp!, {lr}

	movs pc, lr


@ this code switches to the usermode
@ r0 the pointer to the stack pointer
@ r1 the entry point
.global asm_switch_to_usermode
asm_switch_to_usermode:
	mrs r2, cpsr
	bic r2, r2, #0xf
	orr r2, r2, #0x10
	msr cpsr, r2

	mov sp, r0

	mov lr, pc
	bx r1
	b Exit(PLT)

@ syscall code.
@ r0 : request id
@ r1 : arguments array
.global asm_syscall
asm_syscall:
	swi 0
	@return
	mov pc, lr

.global redboot_data
redboot_data:
	.fill 15, 4, 0
redboot_data_addr:
.word redboot_data(GOTOFF)

.global asm_entrypoint
asm_entrypoint:
	ldr r0, redboot_data_addr
	stmea r0, { r0-r14 }
	@ldr r0, redboot_data_end
	@sub r0, r0, #4
	@stmfd r0, {sp, lr}
	b main

.global asm_byebye
asm_byebye:
	ldr r2, redboot_data_addr
	mov r0, #1
	mov r1, r2
	bl bwputr(PLT)
	add r2, r2, #60
	ldmea r2, { r0-r14 }
	/*
	ldr r2, &.redboot_data_end
	mov r0, #1
	mov r1, r2
	bl bwputr(PLT)
	sub r2, r2, #4
	ldmfd r2, {sp, lr}
	mov r0, #1
	mov r1, lr
	bl bwputr(PLT)
	*/
	mov r0, #0
	bx lr

