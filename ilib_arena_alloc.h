//=========================================================================
//                      ilib Arean Allocator
//=========================================================================
// by      : INSANE
// created : 22/10/2026
//
// purpose : Simple single-header arena-allocator
//-------------------------------------------------------------------------
#ifndef ILIB_ARENA_ALLOC_H
#define ILIB_ARENA_ALLOC_H


#define ILIB_AA_INFINITE_ARENAS (-1)


//
// Alignment is ignored across chunk boundaries.
//
// NULL is returned for alloc_size > block_size of the arena
//
// initializing arena with block_size of <= 0 uses system's _SC_PAGESIZE for block_size
//
// initializing arena with max_arena == -1 means "infinite" blocks for that arena
//



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
enum ilib_arena_flags {
	ilibaf_page_sized_block = (1 << 0),
};


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
struct ilib_arena {
	unsigned long long block_size; 
	int max_blocks;
	int block_count;
	void** blocks;
	unsigned long long it; // holds total number of bytes used across all blocks. That means, it > block_size if arena_count > 1
	int flags;
};


void ilib_aa_init(struct ilib_arena *arena, int max_arenas, long long block_size);
void* ilib_aa_alloc(struct ilib_arena *arena, unsigned long long alloc_size, unsigned long long align);
void ilib_aa_unalloc(struct ilib_arena *arena, unsigned long long bytes);
void ilib_aa_free(struct ilib_arena *arena);
unsigned long long ilib_aa_capacity(struct ilib_arena *arena);
unsigned long long ilib_aa_bytes_used(struct ilib_arena *arena);


#endif // ILIB_ARENA_ALLOC_H
