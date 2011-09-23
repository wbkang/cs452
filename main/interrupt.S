.section asm_functions
.global asm_handle_swi
.ident "human compiler (TM)"
@ this code handles swi system call
asm_handle_swi:
	@ save link register
	stmfd sp!, {lr}
	@ save the usermode crap
	stmfd sp!, {r0-r12, sp, lr}^

	@ put the swi argument in r0
	ldr r0,[lr,#-4]
	bic r0, r0, #0xff000000

	@ call function
	bl handle_swi
	
	@ restore link register
	ldmfd sp!, {lr}
	@ load the usermode stuff and byebye
	ldmfd sp!, {r0-r12, sp, pc}^

@ this code switches to the usermode
@ r0 addr
@ r1 mem
@ asm_usermode:
		
