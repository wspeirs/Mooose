ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

virtual  = 0xc0100000; /* 3 GB + 1 MB*/
physical = 0x00100000; /* 1 MB */

/*
The kernel will live at 3GB + 1MB in the virtual
address space, which will be mapped to 1MB in the
physical address space.
*/

SECTIONS
{
    .text virtual : AT(physical)
    {
        code = .; _code = .; __code = .; /* C/C++ convention */

	*(.text)

        *(.gnu.linkonce.t*)

         . = ALIGN(4096);
    }

    .rodata : AT(physical + (rodata - code))
    {
	rodata = .; _rodata = .; __rodata = .;

	*(.rodata*)

	*(.gnu.linkonce.r*)

	. = ALIGN(4096);
    }

    .data : AT(physical + (data - code))
    {
	data = .; _data = .; __data = .;

    	*(.data)

	__CTOR_LIST__ = .; 
	LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2) 
	*(.ctors) 
	LONG(0) 
	__CTOR_END__ = .; 
	
	__DTOR_LIST__ = .; 
	LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2) 
	*(.dtors) 
	LONG(0) 
	__DTOR_END__ = .; 

        *(.gnu.linkonce.d*)

         . = ALIGN(4096);
    }

    .bss : AT(physical + (bss - code))
    {
	bss = .; _bss = .; __bss = .;

	*(.bss)
	*(.bss.*)
	*(COMMON)
	*(.gnu.linkonce.b*)

	. = ALIGN(4096);
    }

    end = .; _end = .; __end = .;
}
