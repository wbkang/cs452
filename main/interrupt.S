.section asm_functions
.global asm_handle_swi
.ident "human compiler (TM)"
@ this code handles swi system call
asm_handle_swi:
	@ save the usermode crap
	stmdb sp, {r0-r12,sp, lr}^
	sub sp, sp, #60

	@ save link register
	stmfd sp!, {lr}

	@ put the swi argument in r0
	ldr r0,[lr,#-4]
	bic r0, r0, #0xff000000

	@ call function
	bl handle_swi
	
	@ restore link register
	ldmfd sp!, {lr}

	@ load the usermode stuff and byebye
	ldmia sp, {r0-r12, sp, pc}^
	add sp, sp, #60

@ this code switches to the usermode
@ r0 addr
@ r1 mem
@ asm_usermode:
		
