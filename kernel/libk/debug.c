#include <debug.h>
#include <screen.h>
#include <string.h>

void printf_base(char* fmt, va_list arg_start)
{
	size_t length = strlen(fmt);

	uint32_t val;
	int32_t ival;
	uint8_t cval;
	char* ptr;
	char buffer[20];

	for (uint16_t i = 0; i < length; i++)
	{
		switch (fmt[i])
		{
		case '%':

			switch (fmt[i + 1])
			{
			case 'u':
				val = va_arg(arg_start, uint32_t);
				uitoa(val, buffer, 10);
                Print(buffer);
				break;
			case 'i':
				ival = va_arg(arg_start, int32_t);
				itoa(ival, buffer, 10);
				Print(buffer);
				break;
			case 'h':
				val = va_arg(arg_start, uint32_t);
				uitoa(val, buffer, 16);
				Print("0x");
				Print(buffer);
				break;
			case 'x':
				val = va_arg(arg_start, uint32_t);
				uitoa(val, buffer, 16);
				Print(buffer);
				break;
			case 's':
				ptr = va_arg(arg_start, char*);
				Print(ptr);
				break;
			case 'c':
				cval = va_arg(arg_start, uint32_t);
				Printch(cval);
				break;
			// case 'l':
			// 	lval = va_arg(arg_start, uint64_t);
			// 	uitoa(lval, buffer, 10);
			// 	Print(buffer);
			// 	break;
			case 'b':
				val = va_arg(arg_start, uint32_t);
				uitoa(val, buffer, 2);
				Print(buffer);
			default:
				break;
			}

			i++;
			break;

		default:
			Printch(fmt[i]);
			break;
		}
	}
}

void printf(char* fmt, ...)
{
	va_list l;
	va_start(l, fmt);	// spooky stack magic going on here

	printf_base(fmt, l);

	va_end(l);
}

void printfln(char* fmt, ...)
{
	va_list l;
	va_start(l, fmt);	// spooky stack magic going on here

	printf_base(fmt, l);

	va_end(l);

	printf("\n");
}

void PANIC(char* str)
{
	asm("cli");
	printfln("\n%s", str);
	asm("hlt");
}

void ASSERT(int expr)
{
	if (expr)
		PANIC("Assertion failed!!");
}

void DEBUG(char* str)
{
	//uint16_t temp = color;
	//SetForegroundColor(RED);

	printf("\nDEBUG: %s\n", str);

	//SetColor(temp >> 8, temp);
}

void WARNING(char * str)
{
	//uint16_t temp = color;
	//SetForegroundColor(DARK_GREEN);

	printf("WARNING: %s\n", str);

	//SetColor(temp >> 8, temp);
}