#include <stdlib.h>
#include <mm.h>
#include <string.h>

struct blockrecord_s {
  FILE *blob;
  size_t blocksize;
  size_t nblocks;
  size_t nheaders;
  /* Index of the current active block */
  size_t curidx;
  /* Index of the next block to try */
  size_t nextidx;
  /* Index of the block we started in for this image */
  size_t startblock;
  /* used_blocks[i] = 1 if block i has been used */
  char *used_blocks;
  /* A buffer containing our current and previous buffers */
  void *block_buf;
  /* Current point in the block buffer*/
  void *cur_offset;
  
  struct jpeg_decompress_struct *cinfo;
};
typedef struct blockrecord_s * blockrecord;

size_t get_idx(size_t blocksize, size_t block_offset) {
  return block_offset / blocksize;
}

blockrecord dsource_init(int fd, size_t blocksize, struct stat *statbuf
                         struct jpeg_decompress_struct *cinfo) {

  blockrecord record = malloc(sizeof(blockrecord_s));
  record->fd = fd;
  record->blocksize = blocksize;
  record->nblocks = nblocks;
  record->used_blocks = calloc(sizeof(char), nblocks);

  record->curidx = -1;
  record->nextidx = 0;

  record->block_buf = malloc(2 * blocksize);
  record->cur_offset = block_buf;
  
  record->cinfo = cinfo;
  
  return record;
}

/* Load the next available block.  Call whenever there's not enough
 * shit for an entire scanline. */
void *next_block(blockrecord record) {
  record->curidx = record->nextidx;
  /* Find the next untried, unused block. */
  int i = curidx + 1;
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
  record->curidx = record->nextidx;
  record->nextidx = i;
  /* Copy the next block into the old half of the buffer. */
  void *dest;
  if (record->cur_offset < (block_buf + record->blocksize))
    dest = block_buf;
  else
    dest = block_buf + record->blocksize;
  fseek(record->blob, record->curidx * record->blocksize, SEEK_SET);
  size_t amt_read = fread(dest, 1, record->blocksize, blob);
  /* @TODO: How do we handle EOF? */
}

void mark_used(blockrecord record, size_t offset) {
  record->used_blocks[get_idx(record, offset)] = 1;
}

void mark_current_used(blockrecord record) {
  record->used_blocs[curidx] = 1;
}

/* We've finished with the current image.  Start from a new
 * header offset and clear tried block info. */
void new_image(blockrecord record, size_t header_offset) {
  
}

void free_blockrecord(blockrecord record) {
  free(record->used_blocks);
  free(record);
}
