
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
		*(.data*)
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
		_TextStart = .;
		*(asm_functions)
		*(.text)
		_TextEnd = .;
		*(.got)
		*(.got.plt)
		*(.rodata)
		*(.rodata.str1.4)
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
		_USER_MEM_START = .;
	} > ram
	
	_USER_MEM_END = 0x1f00000; 
	_KERNEL_MEM_END = 0x2000000;
}


ENTRY (main)
