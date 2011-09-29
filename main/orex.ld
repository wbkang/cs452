
MEMORY
{
	ram : ORIGIN = 0x00000000, LENGTH = 32M
	rom : ORIGIN = 0x60000000, LENGTH = 8M
}

SECTIONS
{

	. = 0x00218000;

	data .  : /* Initialized data. */
	{
		_DataStart = . ;
		*(.data)
		_DataEnd = . ;
	} >ram

	.bss .  : /* Uninitialized data. */
	{
		_BssStart = . ;
		*(.bss)
		_BssEnd = . ;
	} >ram

	text .  : /* The actual instructions. */
	{
		*(asm_functions)
		*(.text)
		*(.got)
		*(.got.plt)
		*(.rodata)
		*(.glue_7)
		*(.glue_7t)

	} >ram
	
	debug . :
	{
		*(.comment)
		*(.debug_line)
		*(.debug_info)
		*(.debug_abbrev)
		*(.debug_aranges)
	} > ram
	
	kernel_mem . :
	{
		_KERNEL_MEM_START = .;
	} > ram
}


ENTRY (main)
