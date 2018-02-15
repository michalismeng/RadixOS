#include <string.h>

size_t strlen(const char* ch)		// return the number of characters the string has, not including the trailing \0
{
	size_t i = 0;
	while (ch[i] != '\0')
		i++;

	return i;
}

int strcmp(const char* s1, const char* s2)
{
    while(*s1 && (*s1==*s2))
        s1++,s2++;
    return *(const unsigned char*)s1-*(const unsigned char*)s2;
}

int strcmp_insensitive(const char* s1, const char* s2)
{
	int res = 0;

	while (!(res = tolower(*s1) - tolower(*s2)) && *s2)
		s1++, s2++;

	if (res < 0)
		res = -1;
	else if (res > 0)
		res = 1;

	return res;
}

void strcpy(char* destination, char* source)
{
	char* s = source;

	while ((*destination = *s))
		destination++, s++;
}

uint8_t strcpy_s(char* destination, size_t destsz, const char* source)
{
	size_t len = strlen(source);

	// TODO: More error-checking.
	if (len > destsz)
		return 1;

	strcpy(destination, source);
	return 0;
}

char* strchr(char* str, int c)
{
	while (*str != 0)
		if (*str++ == c)
			return str - 1;

	return 0;		// entry not found
}

char* strupper(char* str)
{
	char* temp = str;

	while (*str != 0)
	{
		*str = toupper(*str);
		str++;
	}

	return temp;
}