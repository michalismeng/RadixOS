#include <mem_manager_phys.h>
#include <utility.h>
#include <debug.h>
#include <spinlock.h>

// private data

#define PHYS_MEM_BLOCKS_PER_BYTE	8		// this is used in our bitmap structure
#define PHYS_MEM_BLOCK_SIZE			4096	// block size in bytes (use same as page size for convenience)
#define PHYS_MEM_BLOCK_ALIGN		PHYS_MEM_BLOCK_SIZE

static uint32_t phys_mem_memory_size = 0;
static uint32_t phys_mem_used_blocks = 0;
static uint32_t phys_mem_max_blocks = 0;
static uint32_t* phys_mem_bitmap = 0;

static spinlock_t phys_mem_lock = 0;

//PRIVATE - AUX FUNCTIONS

void mmap_set(int bit)
{
	phys_mem_bitmap[bit / 32] |= 1 << (bit % 32);
}

void mmap_unset(int bit)
{
	phys_mem_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

int mmap_test(int bit)
{
	return phys_mem_bitmap[bit / 32] & (1 << (bit % 32));
}

uint32_t phys_mem_get_memory_size()
{
	return phys_mem_memory_size;
}

uint32_t phys_mem_get_block_use_count()
{
	return phys_mem_used_blocks;
}

uint32_t phys_mem_get_block_count()
{
	return phys_mem_max_blocks;
}

uint32_t phys_mem_get_free_blocks_count()
{
	return phys_mem_max_blocks - phys_mem_used_blocks;
}

// returns first free block
uint32_t mmap_first_free(uint32_t blk_index)
{
	for (uint32_t i = blk_index; i < phys_mem_get_block_count() / 32; i++)	// 32 blocks per uint32_t
	{
		if (phys_mem_bitmap[i] != 0xFFFFFFFF)	// at least one bit is off
		{
			int bit = 1;
			for (int j = 0; j < 32; j++)
			{
				if (!(phys_mem_bitmap[i] & bit))	// bit not set
					return i * 32 + j;

				bit <<= 1;
			}
		}
	}

	// set_last_error(ENOMEM, PMEM_OUT_OF_MEM, EO_PMMNGR);
	return 0;
}

// interface functions

void phys_mem_init(uint32_t size, physical_addr base)
{
	// No locks required here
	
	printfln("Initializing physical memory manager with: %u KB of memory", size);
	phys_mem_memory_size = size;
	phys_mem_bitmap = (uint32_t*)base;
	phys_mem_max_blocks = size * 1024 / PHYS_MEM_BLOCK_SIZE;
	phys_mem_used_blocks = phys_mem_max_blocks;

	// by default all memory is in use
	memset(phys_mem_bitmap, 0xff, phys_mem_get_bitmap_size());
}

error_t phys_mem_reserve_region(uint32_t base, uint32_t length)
{
	if (length % PHYS_MEM_BLOCK_SIZE != 0 || base % PHYS_MEM_BLOCK_SIZE != 0)
		PANIC("physical memory align error");

	uint32_t aligned_addr = base / PHYS_MEM_BLOCK_SIZE;
	uint32_t aligned_size = length / PHYS_MEM_BLOCK_SIZE;

	acquire_lock(&phys_mem_lock);

	for (int i = 0; i < aligned_size; i++)
	{
		if(!mmap_test(aligned_addr))
		{
			mmap_set(aligned_addr++);
			phys_mem_used_blocks++;
		}
	}

	release_lock(&phys_mem_lock);

	return ERROR_OK;
}

error_t phys_mem_free_region(uint32_t base, uint32_t length)
{
	if (length % PHYS_MEM_BLOCK_SIZE != 0 || base % PHYS_MEM_BLOCK_SIZE != 0)
		PANIC("physical memory align error");

	uint32_t aligned_addr = base / PHYS_MEM_BLOCK_SIZE;
	uint32_t aligned_size = length / PHYS_MEM_BLOCK_SIZE;

	acquire_lock(&phys_mem_lock);

	for (int i = 0; i < aligned_size; i++)
	{
		if(mmap_test(aligned_addr))
		{
			mmap_unset(aligned_addr++);			// unset it to make it available
			phys_mem_used_blocks--;				// reduce blocks in use
		}		
	}

	release_lock(&phys_mem_lock);

	return ERROR_OK;
}

physical_addr phys_mem_alloc_above(physical_addr addr)
{
	if (phys_mem_get_free_blocks_count() <= 0)		// out of memory
	{
		// set_last_error(ENOMEM, PMEM_OUT_OF_MEM, EO_PMMNGR);
		return 0;
	}

	uint32_t addr_aligned = addr / 4096 * 4096;

	acquire_lock(&phys_mem_lock);

	uint32_t frame = mmap_first_free(ceil_division(addr_aligned, (4096 * 32)));

	if (frame == 0)		// out of memory
		return 0;

	mmap_set(frame);		// here we set directly without mmap_test as this is done in mmap_first_free

	physical_addr ret_addr = frame * PHYS_MEM_BLOCK_SIZE;
	phys_mem_used_blocks++;

	release_lock(&phys_mem_lock);

	return ret_addr;
}

physical_addr phys_mem_alloc()
{
	return phys_mem_alloc_above(0);
}

physical_addr phys_mem_alloc_above_1mb()
{
	return phys_mem_alloc_above(0x100000);
}

int phys_mem_dealloc(physical_addr block)
{
	int frame = block / PHYS_MEM_BLOCK_SIZE;

	acquire_lock(&phys_mem_lock);
	
	mmap_unset(frame);
	phys_mem_used_blocks--;

	release_lock(&phys_mem_lock);

	return 0;
}

uint32_t phys_mem_get_bitmap_size()
{
	return phys_mem_get_block_count() / PHYS_MEM_BLOCKS_PER_BYTE;
}