#include "blockrecord.h"

size_t get_idx(size_t blocksize, size_t block_offset) {
  return (block_offset) / blocksize;
}

blockrecord init_blockrecord(FILE *infile, size_t blocksize, struct stat *statbuf) {

  blockrecord record = malloc(sizeof(struct blockrecord_s));
  record->blob = infile;
  record->blocksize = blocksize;
  record->nblocks = statbuf->st_size / blocksize;
  record->used_blocks = calloc(sizeof(char), record->nblocks);

  record->curidx = -1;
  record->nextidx = 0;

  record->block_buf = malloc(2 * blocksize);
  record->cur_offset = record->block_buf;
  
  return record;
}

/* Step curidx to the nextidx, and find the next unused block for nextidx. */
void find_next_available(blockrecord record) {
  record->curidx = record->nextidx;
  /* Find the next untried, unused block. */
  int i = record->curidx + 1;
  while (record->used_blocks[i]) {
    if (i == record->startblock) {
      /* We ran out of blocks to try :( */
      return NULL;
    } else if (i >= record->nblocks - 1) {
      /* Wrap around to the beginning.  This generally shouldn't happen unless the
       * image is ridiculously fragmented. */
      i = 0;
    } else { 
      i++;
    }
  }
  record->nextidx = i;
}



/* Load the next available block.  Call whenever there's not enough
 * shit for an entire scanline. */
void *next_block(blockrecord record) {
  find_next_available(record);
  /* Copy the next block into the old half of the buffer. */
  void *dest;
  if ((record->cur_offset >= (record->block_buf + record->blocksize)) 
       || record->fresh_image)
    dest = record->block_buf;
  else
    dest = record->block_buf + record->blocksize;
  record->cur_offset = dest;
  fseek(record->blob, record->curidx * record->blocksize, SEEK_SET);
  record->last_read_size = fread(dest, 1, record->blocksize, record->blob);
}

/* same as next_block but instead of swapping out the previous block for a new
 * one we swap out the current one and back up */
void current_failed(blockrecord record) {
  find_next_available(record);
  /* Copy the next block into the current half of the buffer. */
  void *dest;
  if ((record->cur_offset >= (record->block_buf + record->blocksize)) 
       || record->fresh_image)
    dest = record->block_buf + record->blocksize;
  else
    dest = record->block_buf;
  fseek(record->blob, record->curidx * record->blocksize, SEEK_SET);
  record->last_read_size = fread(dest, 1, record->blocksize, record->blob);
}

void mark_used(blockrecord record, size_t offset) {
  record->used_blocks[get_idx(record->blocksize, offset)] = 1;
}

void mark_current_used(blockrecord record) {
  record->used_blocks[record->curidx] = 1;
}

/* We've finished with the current image.  Start from a new
 * header offset and clear tried block info. */
void new_image(blockrecord record, size_t header_offset) {
  record->cur_offset = record->block_buf;
  size_t new_start_idx = get_idx(record->blocksize, header_offset);
  record->curidx = new_start_idx;
  record->nextidx = new_start_idx;
  record->startblock = new_start_idx;
  record->fresh_image = 1;
}

void free_blockrecord(blockrecord record) {
  free(record->used_blocks);
  free(record);
}
