#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
// #include <kernel/term.h>
#include <kernel/drivers/uart16550.h>

#include <kernel/stdio.h>


void putchar (char c) {
    serial_writeb(&com1, c);
}

void puts(const char* s) {
    serial_write(&com1, s);
}

void printf(char *format, ...) { 
	char *traverse; 
	uint32_t u; 
	int32_t i;
	char *s; 
	
	//Module 1: Initializing Myprintf's arguments 
	va_list arg; 
	va_start(arg, format); 
	
	for(traverse = format; *traverse != '\0'; traverse++) 
	{ 
		if (*traverse != '%') { 
			putchar(*traverse);
            continue;
		} 
		
		traverse++; 
		
		//Module 2: Fetching and executing arguments
		switch(*traverse) 
		{ 
			case 'c' : u = va_arg(arg,int);		//Fetch char argument
						putchar(u);
						break; 
						
			case 'i':
			case 'd' : i = va_arg(arg, int32_t); 		//Fetch Decimal/Integer argument
						if(i<0) 
						{ 
							i = -i;
							putchar('-'); 
						} 
						puts(convert(i,10));
						break; 

			case 'u' : u = va_arg(arg, uint32_t); 		//Fetch Decimal/Integer argument
						puts(convert(u,10));
						break; 
						
			case 'o': u = va_arg(arg, uint32_t); //Fetch Octal representation
						puts(convert(u,8));
						break; 
						
			case 's': s = va_arg(arg,char *); 		//Fetch string
						puts(s); 
						break; 
						
			case 'x': u = va_arg(arg, uint32_t); //Fetch Hexadecimal representation
						puts(convert(u,16));
						break; 
		}	
	} 
	
	//Module 3: Closing argument list to necessary clean-up
	va_end(arg); 
} 

char *convert(unsigned int num, int base) 
{ 
	static char Representation[]= "0123456789ABCDEF";
	static char buffer[50]; 
	char *ptr; 
	
	ptr = &buffer[49]; 
	*ptr = '\0'; 
	
	do 
	{ 
		*--ptr = Representation[num%base]; 
		num /= base; 
	}while(num != 0); 
	
	return(ptr); 
}