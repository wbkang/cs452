.section asm_functions
.global asm_handle_swi
.ident "human compiler (TM)"
@ this code handles swi system call
@ r0 is request id
@ r1 is pointer to args
asm_handle_swi:
	@ swi was raised so we are in kernel mode
	@ kernel stack contents: [td_current->registers], lr_svc, ...
	@ now lr_svc = pc_usr

	stmfd sp, {r2} @ store r2 on kernel stack to free it

	@ kernel stack contents: r2, [td_current->registers], lr_svc, ...
	ldr r2, [sp] @ r2 contains pointer to td_current->registers
	str lr, [r2, #64] @ put pc_usr into td_current->registers

	@ store all registers
	stmfa r2, {r0-r12, sp, lr}^ @ stmfa will skip the first address (spsr)

	@ note r2 in td_current->registers contains garbage, write back real r2
	ldr lr, [sp, #-4] @ grab the real r2 off of kurnel stack

	str lr, [r2, #12] @ put real r2 into td_current->registers.r[2]

	@ last thing to save is the spsr
	mrs lr, spsr @ copy spsr_usr into r2
	str lr, [r2] @ put spsr_usr into td_current->registers

	@ now user state is saved, need to branch to lr_svc
	ldmfd sp!, {r0, lr}
	ldmfd sp, {r1-r12, sp}
	@add sp, sp, #60
	bx lr

@ this code switches to the usermode
@ r0 is pointer to register_set (task.h) of current task descriptor
.global asm_switch_to_usermode
asm_switch_to_usermode:
	stmfd sp, {r1-r12, sp}
	sub sp, sp, #52
	stmfd sp!, {r0, lr} @ push lr_svc on kernel stack
	@ kernel stack contents: [td_current->registers], lr_svc, ...

	@ lr_svc is saved so use as scratch
	mov lr, r0 @ make lr point to register_set of current task descriptor
	@ the first thing in the task descriptor is the spsr
	ldr r1, [lr] @ load the spsr from the current task descriptor into r1
	msr spsr, r1 @ set the spsr from r1
	@ now we need to restore task state from current task descriptor
	add lr, lr, #4 @ bump the pointer from spsr to r[0] in register_set
	@ldmfd r0, {r0-r12, sp, lr, pc}^ @ load r0-r15 from register_set into registers and set cpsr
	ldmfd lr, {r0-r12, sp, lr}^ @ load r0-r15 from register_set into registers and set cpsr
	ldr lr, [lr, #60]
	movs pc, lr

@ syscall code.
@ r0 : request id
@ r1 : arguments array
.global asm_syscall
asm_syscall:
	swi 0
	@return
	mov pc, lr

