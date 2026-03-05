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

block_store_t *block_store_create()
{
	block_store_t* bs = calloc(1, sizeof(block_store_t));
	if (!bs) return NULL;

	bs->FBM = bitmap_overlay(BITMAP_SIZE_BITS, bs->data[BITMAP_START_BLOCK]);

	if (!bs->FBM) {
		free(bs);
		return NULL;
	}

	for (size_t i = 0; i < BITMAP_NUM_BLOCKS; i++) {
		block_store_request(bs, BITMAP_START_BLOCK + i);
	}

	return bs;
}

void block_store_destroy(block_store_t *const bs)
{
	if (bs) {
		if (bs->FBM) {
			free(bs->FBM);
		}
		free(bs);
	}
}

size_t block_store_allocate(block_store_t *const bs)
{
	if (!bs) return SIZE_MAX;

	size_t block = bitmap_ffz(bs->FBM);

	if (block == SIZE_MAX) return SIZE_MAX;

	bitmap_set(bs->FBM, block);

	return block;
}

bool block_store_request(block_store_t *const bs, const size_t block_id)
{
	if (!bs) return false;

	if (block_id >= BLOCK_STORE_NUM_BLOCKS) return false;

	if (bitmap_test(bs->FBM, block_id)) return false;

	bitmap_set(bs->FBM, block_id);

	return bitmap_test(bs->FBM, block_id);
}

void block_store_release(block_store_t *const bs, const size_t block_id)
{
	if (!bs) return;

	if (block_id >= BLOCK_STORE_NUM_BLOCKS) return;

	bitmap_reset(bs->FBM, block_id);
}

size_t block_store_get_used_blocks(const block_store_t *const bs)
{
	UNUSED(bs);
	return 0;
}

size_t block_store_get_free_blocks(const block_store_t *const bs)
{
	UNUSED(bs);
	return 0;
}

size_t block_store_get_total_blocks()
{
	return 0;
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
