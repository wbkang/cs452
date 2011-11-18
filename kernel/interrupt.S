.section asm_functions
.global asm_handle_swi
.ident "human compiler (TM)"
@ this code handles swi system call
asm_handle_swi:
	@ <) swi was raised so we are in svc mode
	@ <) sp_svc: [(td_current->registers)], r4-r11, lr_svc, ...
	@ <) now lr_svc = pc_usr

	@ need a scratch register, lets use lr
	stmfd sp!, {lr}
	@ <) sp_svc: [lr], (td_current->registers), r4-r11, lr_svc, ...

	@ store user state
	ldr lr, [sp, #4] @ pointer to td_current->registers
	stmfa lr, {r0-r12, sp, lr}^ @ stmfa will skip spsr
	@ store cpsr
	mrs r0, spsr
	str r0, [lr]
	@ store pc
	ldmfd sp!, {r0}
	@ <) sp_svc: [(td_current->registers)], r4-r11, lr_svc, ...
	str r0, [lr, #4*16]

	@ restore kernel state
	add sp, sp, #4
	@ <) sp_svc: (td_current->registers), [r4], r5-r11, lr_svc, ...

	mrs r0, cpsr @ return cpsr
	ldmfd sp!, {r4-r11, pc}


@ this code switches to the usermode
@ r0 is pointer to register_set (task.h) of current task descriptor
.global asm_switch_to_usermode
asm_switch_to_usermode:
	@ save kernel state on stack
	stmfd sp!, {r0, r4-r11, lr}
	@ <) sp_svc: [(td_current->registers)], r4-r11, lr_svc, ...

	@ <) all registers are saved, can be trashed

	@@@@ new part
	@ spare sp in r2
	mov r2, sp

	@ if we are in irq, store sp_irq into sp_svc and vice versa for svc
	mrs r1, cpsr
	and r1, r1, #0x1f 	@ 0x1f is the last 5 mode bit
	cmp r1, #0x12 		@ 0x12 is the irq mode
	@ else, currently we are in svc copy sp_svc to sp_irq
	msrne cpsr, #0xd2		@ switch to irq
	@ we are in irq mode, so switch to svc and save sp_irq to sp_svc
	msreq cpsr, #0xd3
	mov sp, r2
	@@@@@ end new part

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

.global Receive
Receive:
	swi 7
	mov pc, lr

.global Reply
Reply:
	swi 8
	mov pc, lr

.global NameServerTid
NameServerTid:
	swi 9
	mov pc, lr

.global AwaitEvent
AwaitEvent:
	swi 10
	mov pc, lr

.global ExitKernel
ExitKernel:
	swi 11
	mov pc, lr

.global CrashDump
CrashDump:
	swi 12
	mov pc, lr
