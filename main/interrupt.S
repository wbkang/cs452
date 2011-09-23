.section asm_functions
.global asm_handle_swi
.ident "human compiler (TM)"
@ this code handles swi system call
asm_handle_swi:
	@ save r0-r12, lr
	stmfd sp!, {r0-r12, lr}

	@ put the swi argument in r0
	ldr r0,[lr,#-4]
	bic r0, r0, #0xff000000

	@ save the kernel sp to r1
	mov r1, sp

	@ system mode transition
	mrs r2, cpsr	
	orr r2, r2, #0x1f
	msr cpsr, r2

	@ now we have user sp
	@ save user sp, lr into kernel stack
	stmfd r1!, {sp,lr}

	@ use kernel sp for the syscall
	mov sp, r1
	
	@ call function
	bl handle_swi

	@ load user sp into r1, load lr and restore the user sp
	ldmfd sp!, {r1, lr}
	mov sp, r1

	@ go back to supervisor mode. 
	mrs r0, cpsr
	bic r0,r0,#12
	msr cpsr, r0
	
	@ now in usermode
	@ r2 still contains the final kernel sp
	ldmfd sp!, {r0-r12, pc}^

@ this code switches to the usermode
@ r0 addr
@ r1 mem
@ asm_usermode:
		
