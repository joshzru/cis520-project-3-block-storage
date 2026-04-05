#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

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
			bitmap_destroy(bs->FBM);
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

/// <summary>
/// Reads the contents of a block into a buffer
/// </summary>
/// <param name="bs">The block store whose contents are read from</param>
/// <param name="block_id">The block id to read from</param>
/// <param name="buffer">The buffer to read into</param>
/// <returns>The number of bytes successfully read</returns>
size_t block_store_read(const block_store_t* const bs, const size_t block_id, void* buffer)
{
	//if the block store does not exist, fail
	if (!bs) return 0;

	//if the id out of range, fail
	if (block_id >= BLOCK_STORE_NUM_BLOCKS) return 0;

	//if the buffer does not exist, fail
	if (!buffer) return 0;

	//read memory into the buffer
	memcpy(buffer, bs->data[block_id], BLOCK_SIZE_BYTES);

	//return number of bytes read
	return BLOCK_SIZE_BYTES;
}

/// <summary>
/// Writes the contents of a buffer to a block
/// </summary>
/// <param name="bs">The block store to be written in</param>
/// <param name="block_id">The id of the block to write into</param>
/// <param name="buffer">The buffer to write from</param>
/// <returns>The number of bytes successfully written</returns>
size_t block_store_write(block_store_t* const bs, const size_t block_id, const void* buffer)
{
	//if the block store does not exist, fail
	if (!bs) return 0;

	//if the id out of range, fail
	if (block_id >= BLOCK_STORE_NUM_BLOCKS) return 0;

	//if the buffer does not exist, fail
	if (!buffer) return 0;

	//write memory into the block
	memcpy(bs->data[block_id], buffer, BLOCK_SIZE_BYTES);

	//return number of bytes written
	return BLOCK_SIZE_BYTES;
}

/// <summary>
/// Deserializes the contents of a file to a block_store_t
/// </summary
/// <param name="filename">The name of the file to read from</param>
/// <returns>A pointer to the deserialized block_store_t</returns>
block_store_t *block_store_deserialize(const char *const filename)
{
	// If the filename doesn't exist, fail
	if (!filename) {
		return NULL;
	}
	
	// Allocate the struct to return
	block_store_t *bs = malloc(sizeof(block_store_t));
	if (!bs) {
		return NULL;
	}

	// Open the file with read permissions
	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		free(bs);
		return NULL;
	}

	// Only need to read the data, not the pointer to the bitmap, that'll be created later
	size_t total_size = sizeof(bs->data);
	uint8_t *data_ptr = (uint8_t *)&bs->data[0][0];
	size_t bytes_read = 0;

	// Loop until we've read all the data
	while (bytes_read < total_size) {
		ssize_t result = read(fd, data_ptr + bytes_read, total_size - bytes_read);
		if (result < 0) {
			// If the read was interrupted, continue
			if (errno == EINTR) {
				continue;
			}
			perror("Error on read in block_store_deserialize");
			free(bs);
			close(fd);
			return NULL;
		}
		bytes_read += result;
	}	

	close(fd);

	// Create the bitmap pointing to the 127th block
	bs->FBM = bitmap_overlay(BITMAP_SIZE_BITS, bs->data[BITMAP_START_BLOCK]);

	return bs;
}

/// <summary>
/// Serializes a block_store_t to a file
/// </summary>
/// <param name="bs">The block store to serialize</param>
/// <param name="filename">The name of the file to write to</param>
/// <returns>The size of the file in bytes</returns>
size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
	// If inputs are invalid, we can't serialize, so return 0
	if (!bs || !filename) {
		return 0;
	}

	// Open the file with read permissions, create it if it doesn't exist; if it does exist, overwrite it
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	// If open failed, we can't write to it, so return 0
	if (fd < 0) {
		return 0;
	}

	// Only need to write the data, not the pointer to the bitmap
	size_t total_size = sizeof(bs->data);
	const uint8_t *data_ptr = (const uint8_t *)&bs->data[0][0];
	size_t bytes_written = 0;

	// Loop until we've written all of the data
	while (bytes_written < total_size) {
		ssize_t result = write(fd, data_ptr + bytes_written, total_size - bytes_written);
		// On failure, return how much has been written
		if (result < 0) {
			// If the write was interrupted, continue
			if (errno == EINTR) {
				continue;
			}
			perror("Error on write in block_store_serialize");
			close(fd);
			return bytes_written;
		}
		bytes_written += result;
	}

	close(fd);
	return bytes_written;
}
