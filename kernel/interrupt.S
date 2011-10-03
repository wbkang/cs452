.section asm_functions
.global asm_handle_swi
.ident "human compiler (TM)"
@ this code handles swi system call
asm_handle_swi:
	@ <) swi was raised so we are in svc mode
	@ <) sp_svc: [r0 (td_current->registers)], r1-r12, lr_svc, ...
	@ <) now lr_svc = pc_usr

	@ inject pc_usr into td_current->registers.r[15]
	ldr ip, [sp] @ get a pointer to td_current->registers
	str lr, [ip, #4*16] @ inject, freeing lr_svc for use

	@ store user state
	stmfa ip, {r0-r12, sp, lr}^ @ stmfa will skip the first item (spsr)

	@ inject spsr into td_current->registers.spsr
	mrs lr, spsr @ copy
	str lr, [ip] @ inject

	@ <) task state is completely saved

	@ restore kernel state
	ldmfd sp!, {r0-r12, pc} @ restore kernel state


@ this code switches to the usermode
@ r0 is pointer to register_set (task.h) of current task descriptor
.global asm_switch_to_usermode
asm_switch_to_usermode:
	@ save kernel state on stack
	stmfd sp!, {r0-r12, lr}
	@ <) sp_svc: [r0 (td_current->registers)], r1-r12, lr_svc, ...

	@ <) all registers are saved, can be trashed
	@ <) we will be restoring user state, but will need a scratch register. why not use lr_svc?

	@ restore spsr
	ldr lr, [r0] @ load the spsr from td_current->registers, it is the first item
	msr spsr, lr @ set the spsr

	@ restore registers
	mov lr, r0 @ copy the pointer to td_current->registers into lr_svc
	ldmib lr, {r0-r12, sp, lr}^ @ load r0-r15 from register_set into registers and set cpsr
	ldr lr, [lr, #4*16] @ load pc from register_set

	@ resume task
	movs pc, lr @ go to there


@ syscall code.

.global Create
Create:
	swi 0
	mov pc, lr

.global MyTid
MyTid:
	swi 1
	mov pc, lr

.global MyParentsTid
MyParentsTid:
	swi 2
	mov pc, lr

.global Pass
Pass:
	swi 3
	mov pc, lr

.global Exit
Exit:
	swi 4
	mov pc, lr

.global malloc
malloc:
	swi 5
	mov pc, lr

.global asm_Send
asm_Send:
	swi 6
	mov pc, lr

.global Recieve
Recieve:
	swi 7
	mov pc, lr

.global Reply
Reply:
	swi 8
	mov pc, lr
