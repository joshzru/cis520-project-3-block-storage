#include <stdio.h>
#include <stdint.h>

#include "bitmap.h"
#include "block_store.h"
// include more if you need


// You might find this handy. I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

struct block_store {
	uint8_t data[BLOCK_STORE_NUM_BLOCKS][BLOCK_SIZE_BYTES]; // 2-dimensional static arrays should have contiguous memory (I think)
	bitmap_t *FBM; // 'FBM->data' points to a section of 'data' (specifically, the beginning of block 127)
};

/// <summary>
/// Creates a new block store
/// </summary>
/// <returns>A pointer to the new block store</returns>
block_store_t *block_store_create()
{
	//Allocates memory to the block store and initializes it to zeros
	block_store_t* bs = calloc(1, sizeof(block_store_t));
	if (!bs) return NULL;

	//Sets bitmap field to overlay of bitmap with size BITMAP_SIZE_BYTES at index BITMAP_START_BLOCK
	bs->FBM = bitmap_overlay(BITMAP_SIZE_BITS, bs->data[BITMAP_START_BLOCK]);

	if (!bs->FBM) {
		free(bs);
		return NULL;
	}

	//Marks used blocks with block_store_request
	for (size_t i = 0; i < BITMAP_NUM_BLOCKS; i++) {
		block_store_request(bs, BITMAP_START_BLOCK + i);
	}

	//return pointer to block store
	return bs;
}

/// <summary>
/// Destroys a block store and marks it as allocated in the bitmap
/// </summary>
/// <param name="bs">The block store to destroy</param>
void block_store_destroy(block_store_t *const bs)
{
	//if the block store exists, free its associated memory
	if (bs) {
		if (bs->FBM) {
			free(bs->FBM);
		}
		free(bs);
	}
}

/// <summary>
/// Finds the first free block in the block store and marks it as allocated in the bitmap
/// </summary>
/// <param name="bs">The blockstore to search through</param>
/// <returns>SIZE_MAX on error, allocated block's id on success</returns>
size_t block_store_allocate(block_store_t *const bs)
{
	//if the block store does not exist, fail
	if (!bs) return SIZE_MAX;

	//locates first free block
	size_t block = bitmap_ffz(bs->FBM);

	//if first free block is SIZE_MAX, fail
	if (block == SIZE_MAX) return SIZE_MAX;

	//mark as allocated
	bitmap_set(bs->FBM, block);

	//return block's id
	return block;
}

/// <summary>
/// Marks a specific block as allocated in a bitmap
/// </summary>
/// <param name="bs">The block store containing the bitmap to mark in</param>
/// <param name="block_id">The requested block id</param>
/// <returns>True on success, false on failure</returns>
bool block_store_request(block_store_t *const bs, const size_t block_id)
{
	//if the block store does not exist, fail
	if (!bs) return false;

	//if requested id is out of range, fail
	if (block_id >= BLOCK_STORE_NUM_BLOCKS) return false;

	//if block requested was previously allocated, fail
	if (bitmap_test(bs->FBM, block_id)) return false;

	//mark the block at the requested id
	bitmap_set(bs->FBM, block_id);

	//Returns true to show a successful mark
	return bitmap_test(bs->FBM, block_id);
}

/// <summary>
/// Marks a specific block as free in the bitmap
/// </summary>
/// <param name="bs">The block store containing the bitmap to free in</param>
/// <param name="block_id">The block id to free</param>
void block_store_release(block_store_t *const bs, const size_t block_id)
{
	//if the block store does not exist, fail
	if (!bs) return;

	//if the id out of range, fail
	if (block_id >= BLOCK_STORE_NUM_BLOCKS) return;

	//frees the block at the given id
	bitmap_reset(bs->FBM, block_id);
}


/// <summary>
/// Returns the number of blocks currently alloctated in the block store.
/// </summary>
/// <param name="bs">The block store to return the number of currently allocated blocks from</param>
/// <returns>The number of blocks allocated in the store</returns>
size_t block_store_get_used_blocks(const block_store_t *const bs)
{
	//if the block store does not exist, fail
	if (!bs) return SIZE_MAX;

	//return the allocated block total
	return bitmap_total_set(bs->FBM);
}

/// <summary>
/// Returns the number of blocks currently free in the block store.
/// </summary>
/// <param name="bs">The block store to return the number of currently free blocks from</param>
/// <returns>The number of free blocks in the store</returns>
size_t block_store_get_free_blocks(const block_store_t *const bs)
{
	//if the block store does not exist, fail
	if (!bs) return SIZE_MAX;

	//return free blocks (total - allocated)
	return block_store_get_total_blocks() - bitmap_total_set(bs->FBM);
}

/// <summary>
/// Returns total number of blocks in the block store
/// </summary>
/// <returns>The total number of blocks in the block store</returns>
size_t block_store_get_total_blocks()
{
	return BLOCK_STORE_NUM_BLOCKS;
}

size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
	UNUSED(bs);
	UNUSED(block_id);
	UNUSED(buffer);
	return 0;
}

size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
	UNUSED(bs);
	UNUSED(block_id);
	UNUSED(buffer);
	return 0;
}

block_store_t *block_store_deserialize(const char *const filename)
{
	UNUSED(filename);
	return NULL;
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
	UNUSED(bs);
	UNUSED(filename);
	return 0;
}
