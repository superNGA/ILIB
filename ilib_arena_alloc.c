//=========================================================================
//                      ilib Arean Allocator
//=========================================================================
// by      : INSANE
// created : 22/10/2026
//
// purpose : Simple single-header arena-allocator
//-------------------------------------------------------------------------
#include "ilib_arena_alloc.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>


__attribute__((warn_unused_result)) static void* malloc_block(unsigned long long size);
__attribute__((warn_unused_result)) static void* mmap_block(unsigned long long size);
__attribute__((warn_unused_result)) static unsigned long long align_llu(unsigned long long x, unsigned long long align);


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ilib_aa_init(struct ilib_arena *arena, int max_arenas, long long block_size)
{
	arena->max_blocks  = max_arenas;
	arena->block_count = 0;
	arena->blocks      = NULL;
	arena->it          = 0;
	arena->flags       = 0;

	if (block_size <= 0) {
		arena->block_size = sysconf(_SC_PAGESIZE);
		arena->flags     |= ilibaf_page_sized_block;
	}
	else {
		arena->block_size = block_size;
	}
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void* ilib_aa_alloc(struct ilib_arena *arena, unsigned long long alloc_size, unsigned long long align)
{
	assert(arena->block_count >= 0 && "bad arena received. Most probably uninitialized");
	if (alloc_size > arena->block_size || alloc_size == 0)
		return NULL;

	unsigned long long aligned_it = align_llu(arena->it, align == 0 ? 1 : align);
	unsigned long long alloc_end_it = aligned_it + alloc_size;

	// If after alignment and alloc_size, iterator reaches next block, we must allocate from the next block
	// itself.
	int cur_block = arena->it / arena->block_size;
	int alloc_end_block = (alloc_end_it - 1) / arena->block_size;
	assert((alloc_end_block < arena->max_blocks || arena->max_blocks == ILIB_AA_INFINITE_ARENAS) && "block count overflown in arena");
	if (alloc_end_block >= arena->max_blocks && arena->max_blocks != ILIB_AA_INFINITE_ARENAS) // Reached max arena limit
		return NULL;

	if (alloc_end_block > cur_block || cur_block >= arena->block_count) {
		aligned_it = align_llu(aligned_it, arena->block_size);

		void *new_block = (arena->flags & ilibaf_page_sized_block) ? mmap_block(arena->block_size) : malloc_block(arena->block_size);
		if (new_block == NULL || new_block == MAP_FAILED)
			return NULL;

		void **new_arenas = realloc(arena->blocks, (arena->block_count + 1) * sizeof(void*));
		if (new_arenas == NULL)
			return NULL;

		arena->blocks = new_arenas;
		arena->blocks[arena->block_count] = new_block;
		arena->block_count += 1;
	}

	arena->it = aligned_it + alloc_size;
	int target_blk_index = aligned_it / arena->block_size;
	int blk_offset = aligned_it % arena->block_size;
	return (char*)arena->blocks[target_blk_index] + blk_offset;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ilib_aa_unalloc(struct ilib_arena *arena, unsigned long long bytes)
{
	assert(arena->it >= bytes && "Trying to free more than arena capacity");
	if (arena->it >= bytes)
		arena->it -= bytes;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ilib_aa_free(struct ilib_arena *arena)
{
	for (int i = 0; i < arena->block_count; ++i) {
		if (arena->flags & ilibaf_page_sized_block)
			munmap(arena->blocks[i], arena->block_size);
		else
			free(arena->blocks[i]);
	}

	arena->it = 0;
	arena->block_count = 0;
	free(arena->blocks); arena->blocks = NULL;
	return;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
unsigned long long ilib_aa_capacity(struct ilib_arena *arena)
{
	return arena->block_size * arena->block_count;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
unsigned long long ilib_aa_bytes_used(struct ilib_arena *arena)
{
	return arena->it;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
__attribute__((warn_unused_result)) static unsigned long long align_llu(unsigned long long x, unsigned long long align)
{
	if (x == 0)
		return x;

	if ((x % align) == 0)
		return x;

	return x - (x % align) + align;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
__attribute__((warn_unused_result)) static void* malloc_block(unsigned long long size)
{
	return malloc(size);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
__attribute__((warn_unused_result)) static void* mmap_block(unsigned long long size)
{
	return mmap(NULL, size,
			PROT_READ | PROT_WRITE, 
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}
