#include <utility.h>
#include <string.h>

char hexes[] = "0123456789ABCDEF";
char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

void uitoa(uint32_t val, char* buffer, uint8_t base)
{
	uint16_t i = 0;
	do
	{
		buffer[i] = hexes[val % base];
		i++;
	} while ((val /= base) != 0);

	buffer[i] = 0;

	//reverse

	uint16_t length = strlen(buffer);
	i = 0;
	for (; i < length / 2; i++)
	{
		char temp = buffer[i];
		buffer[i] = buffer[length - 1 - i];
		buffer[length - 1 - i] = temp;
	}

	buffer[length] = 0;		// not required but to be sure. buffer[i] = 0 above is enough and is not affected by the reverse process.
}

void uitoalpha(uint32_t val, char* buffer)
{
	uint16_t i = 0;
	do
	{
		buffer[i] = alphabet[val % 26];
		i++;
	} while ((val /= 26) != 0);

	buffer[i] = 0;

	uint16_t length = strlen(buffer);

	i = 0;
	for (; i < length / 2; i++)
	{
		char temp = buffer[i];
		buffer[i] = buffer[length - 1 - i];
		buffer[length - 1 - i] = temp;
	}

	//buffer[length] = 0;
}

void itoa(int32_t val, char* buffer, uint8_t base)
{
	if (base <= 1 || base >= 17)
		return;

	uint16_t i = 0;
	int16_t sign = 1;

	if (val < 0)
	{
		sign = -1;
		val = -val;
	}

	do
	{
		buffer[i] = hexes[val % base];
		i++;
	} while ((val /= base) != 0);

	if (base == 16)
	{
		buffer[i++] = 'x';
		buffer[i++] = '0';
	}

	if (sign == -1)
		buffer[i++] = '-';

	buffer[i] = 0;

	//reverse

	uint16_t length = strlen(buffer);
	i = 0;
	for (; i < length / 2; i++)
	{
		char temp = buffer[i];
		buffer[i] = buffer[length - 1 - i];
		buffer[length - 1 - i] = temp;
	}

	buffer[length] = 0;		// not required but to be sure. buffer[i] = 0 above is enough and is not affected by the reverse process.
}

uint32_t pow(uint32_t base, uint32_t exp)	// IMPROVEEEEE
{
	uint32_t res = 1;;
	for (uint32_t i = 0; i < exp; i++)
		res *= base;
	return res;
}

uint32_t ceil_division(uint32_t value, uint32_t divisor)
{
	uint32_t div = value / divisor;
	uint32_t rem = value % divisor;

	if (rem != 0)
		div++;

	return div;
}

uint32_t atoui(char* input)
{
	if (input[0] == '0' && tolower(input[1]) == 'x')	// hex
		return atoui_hex(input + 2, strlen(input) - 2);
	else
		return atoui_dec(input, strlen(input));
}

uint32_t atoui_hex(char* input, uint16_t length)
{
	uint32_t res = 0;

	for (uint32_t i = 0; i < length; i++)
	{
		uint32_t mult = input[i] - '0';

		if (isalpha(input[i]))
			mult = 10 + tolower(input[i]) - 'a';	// convert

		res += mult * pow(16, length - 1 - i);
	}

	return res;
}

uint32_t atoui_dec(char* input, uint16_t length)
{
	uint32_t res = 0;

	for (uint32_t i = 0; i < length; i++)
		res += (input[i] - '0') * pow(10, length - 1 - i);

	return res;
}

void memset(void* base, uint8_t val, uint32_t length)
{
	char* ptr = (char*)base;

	uint32_t index = length % sizeof(uint32_t);

	for (uint32_t j = 0; j < index; j++)
	{
		*ptr = val;
		ptr++;
		length--;
	}

	uint32_t* new_ptr = (uint32_t*)ptr;
	length /= sizeof(uint32_t);

	//create the 32 bit value so that the last loop works
	uint32_t val32 = val & 0xFF;
	val32 |= (val << 8) & 0xFF00;
	val32 |= (val << 16) & 0xFF0000;
	val32 |= (val << 24) & 0xFF000000;

	for (uint32_t i = 0; i < length; i++)	// must be strictly less than length
		new_ptr[i] = val32;
}

void memcpy(void* dest, void* source, uint32_t length)
{
	char* ptr_s = (char*)source;
	char* ptr_d = (char*)dest;

	uint32_t index = length % sizeof(uint32_t);

	for (uint32_t j = 0; j < index; j++)
	{
		*ptr_d = *ptr_s;
		ptr_s++;
		ptr_d++;
		length--;
	}

	uint32_t* new_ptr_s = (uint32_t*)ptr_s;
	uint32_t* new_ptr_d = (uint32_t*)ptr_d;
	length /= sizeof(uint32_t);

	for (uint32_t i = 0; i < length; i++)	// must be strictly less than length
		new_ptr_d[i] = new_ptr_s[i];
}