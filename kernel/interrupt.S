.section asm_functions
.global asm_handle_swi
.ident "human compiler (TM)"
@ this code handles swi system call
@ r0 is request id
@ r1 is pointer to args
asm_handle_swi:
	@ swi was raised so we are in kernel mode
	@ need to save user state
	sub lr, lr, #4
	stmfd sp!, {r0-r12, lr}
	@ kernel stack contents: [r0], r1, ..., r12, lr_usr, ...

	@ now that r2 is saved we can use it
	mrs r2, spsr @ copy spsr_usr into r2
	stmfd sp!, {r2} @ push spsr_usr on stack
	@ user stack contents: [spsr_usr], r0, ..., r12, lr_usr, ...

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

	@bl handle_swi

	movs pc, lr


@ this code switches to the usermode
@ r0 is pointer to register_set (task.h) of current task descriptor
.global asm_switch_to_usermode
asm_switch_to_usermode:
	stmfd sp!, {r0, lr} @ push lr_svc on kernel stack
	@ kernel stack contents: [r0], lr_svc, ...
	@ r0 is pointer to register_set of current task descriptor
	@ the first thing in the task descriptor is the spsr
	ldr r1, [r0] @ load the spsr from the current task descriptor into r1
	msr spsr, r1 @ set the spsr from r1
	@ now we need to restore task state from current task descriptor
	add r0, r0, #4 @ bump the pointer from spsr to r[0] in register_set
	ldmfd r0, {r0-r12, sp, lr, pc}^ @ load r0-r15 from register_set into registers and set cpsr


@ syscall code.
@ r0 : request id
@ r1 : arguments array
.global asm_syscall
asm_syscall:
	swi 0
	@return
	mov pc, lr

