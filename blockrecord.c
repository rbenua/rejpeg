#include <stdlib.h>
#include <mm.h>
#include <string.h>

struct blockrecord_s {
  int fd;
  size_t blocksize;
  size_t nblocks;
  size_t nheaders;
  /* Index of the current active block */
  size_t curidx;
  /* Index of the next block to try */
  size_t nextidx;
  /* used_blocks[i] = 1 if block i has been used */
  char *used_blocks;
  /* offsets of headers in the file */
  void **header_offsets;
  /* Indiceses of blocks containing headers */
  size_t *header_blocks;
  /* A buffer containing our current and previous buffers */
  void *block_buf;
  /* Current point into the block */
  void *cur_offset;
};
typedef struct blockrecord_s * blockrecord;

size_t get_idx(size_t blocksize, size_t block_offset) {
  return block_offset / blocksize;
}

blockrecord init_blockrecord(int fd, size_t nblocks, size_t blocksize,
                             void **header_offsets, size_t nheaders) {

  blockrecord record = malloc(sizeof(blockrecord_s));
  record->fd = fd;
  record->blocksize = blocksize;
  record->nblocks = nblocks;
  record->used_blocks = calloc(sizeof(char), nblocks);

  record->header_offsets = header_offsets;
  record->header_blocks = malloc(sizeof(void *) * nblocks);
  for (int i = 0; i < nheaders; i++) {
    record->header_blocks[i] = get_idx(record, header_offsets[i]);
  }
  
  record->curidx = -1;
  record->nextidx = header_blocks[0];
  
  record->block_buf = malloc(2 * blocksize);
  record->cur_offset = 

  return record;
}

void *next_block(blockrecord record) {
  

void mark_current_used(blockrecord record) {
  used_blocks
}

void next_image(blockrecord record);

void free_blockrecord(blockrecord record) {
  free(record->used_blocks);
  free(record);
}
