SECTIONS 
	{
		. = 0x57e00000;
		.test : 
		{
			*(.text)
		}
		
		. = ALIGN(4);
		.rodata :
		{
			*(.data)
		}
		
		. = ALIGN(4);	
		.data :
		{
			*(.data)
		}
		
		. = ALIGN(4);
		bss_start = .;
		.bss  : { *(.bss)  *(COMMON) }
		bss_end = .;
	}
