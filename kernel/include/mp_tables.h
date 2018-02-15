#ifndef MP_TABLES_11022018
#define MP_TABLES_11022018

#include <stdint.h>

#pragma pack(push, 1)

typedef struct mp_pointer_struct_t
{
	uint8_t sig[4];
	uint32_t mp_ptr;
	uint8_t len;
	uint8_t ver;
	uint8_t csum;
	uint8_t features1;
	uint8_t features2;
	uint8_t resv[3];

} mp_pointer_t;

typedef struct mp_config_struct_t
{
	uint8_t sig[4];
	uint16_t len;
	uint8_t ver;
	uint8_t csum;
	uint8_t oem[8];
	uint8_t prod_id[12];
	uint32_t oem_ptr;
	uint16_t oem_size;
	uint16_t entry_count;
	uint32_t lapic_addr;
	uint16_t xtable_length;
	uint8_t xtable_csum;

} mp_configuration_t;

#pragma pack(pop)

// uint8_t* bda = 0x400;
// uint32_t p;

// mp_pointer_t* mp = 0;

// if((p = *(uint16_t*)(bda + 0x0E)))
// {
// 	p <<= 4;
// 	if((mp = search_mp(p, 1024)))	// search in ebda
// 	{
// 		printf(" found mp table in ebda");
// 	}	
// 	else		// search in end of base memory
// 	{
// 		p = *(uint16_t*)(bda + 0x13) * 1024;
// 		if((mp = search_mp(p, 1024)))
// 			printf(" found mp table in high memory");
// 		else
// 		{
// 			if((mp = search_mp(0xF0000, 0x10000)))
// 				printf(" found mp in bios ROM area");
// 			else
// 				printf(" mp table not fod in ebda region");
// 		}
// 	}
// }
// else
// {
// 	printf(" ebda not present.");
// }

// if(mp)
// {
// 	if(mp->features1 == 0)
// 	{
// 		printf(" mp pointer is valid");

// 		mp_configuration_t* mp_config = (mp_configuration_t*)mp->mp_ptr;
// 		p = mp->mp_ptr + sizeof(mp_configuration_t);
// 		int num_cores = 0;

// 		for(int i = 0; i < mp_config->entry_count; i++)
// 		{
// 			uint8_t type = *(uint8_t*)p;
// 			switch(type)
// 			{
// 				case 0:	
// 					printf(" processor"); p += 20; num_cores++; break;
// 				case 2:
// 					printf(" IO APIC"); p += 8; num_cores++; break;
// 				case 1:
				
// 				case 3:
// 				case 4: 
// 					printf(" 8-byte type"); p += 8; break;
// 				default:
// 					printf(" unknown type");
// 			}
// 		}
// 	}
// 	else
// 		printf(" mp default config");
// }

// mp_pointer_t* search_mp(uint16_t* base, int length)
// {
// 	for(uint8_t* ptr = base; ptr < base + length; ptr += 16)
// 	{
// 		if(ptr[0] == '_' && ptr[1] == 'M' && ptr[2] == 'P' && ptr[3] == '_'
// 			&& sum(ptr, 16) == 0)
// 			return (mp_pointer_t*)ptr;
// 	}

// 	return 0;
// }


#endif