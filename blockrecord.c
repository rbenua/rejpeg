#include "blockrecord.h"

size_t get_idx(size_t blocksize, size_t block_offset) {
  return (block_offset) / blocksize;
}

blockrecord init_blockrecord(FILE *infile, size_t blocksize, struct stat *statbuf) {

  blockrecord record = malloc(sizeof(struct blockrecord_s));
  record->blob = infile;
  record->blocksize = blocksize;
  record->nblocks = (statbuf->st_size + blocksize - 1) / blocksize;
  record->used_blocks = calloc(sizeof(char), record->nblocks);

  record->curidx = 0;

  record->block_buf = malloc(2 * blocksize);
  record->cur_offset = record->block_buf;
  record->decbuf = record->cur_offset;

  //record->stored_scanline_ptr = NULL;
  
  return record;
}

/* Step curidx to the nextidx, and find the next unused block for nextidx. */
int find_next_available(blockrecord record){
  mark_current_used(record);
  while(record->used_blocks[record->curidx]){
    record->curidx = (record->curidx + 1) % record->nblocks;
    if(record->curidx == record->startblock){
      return -1;
    }
  }
  return 0;
}


/* Load the next available block.  Call whenever there's not enough
 * data for an entire scanline. */
int next_block(blockrecord record) {
  printf("used block %d\n", record->curidx);
  if (find_next_available(record) < 0) {
    return -1;
  }
  /* we start (and when we call next_block, are always in) the top half of the
   * buffer. */
  if (record->pub.next_input_byte >= record->block_buf + record->blocksize) {
      record->pub.next_input_byte -= record->blocksize;
      memcpy(record->block_buf, record->block_buf + record->blocksize, record->blocksize);
  }
  fseek(record->blob, record->curidx * record->blocksize, SEEK_SET);
  record->last_read_size = fread(record->block_buf + record->blocksize, 
                                 1, record->blocksize, record->blob);
  record->pub.bytes_in_buffer = (size_t)record->block_buf + ((size_t)record->blocksize * 2) - 
                                (size_t)record->pub.next_input_byte;
  return 0;
}

/*
void current_failed(blockrecord record) {
  find_next_available(record);
  void *dest;
  if ((record->cur_offset >= (record->block_buf + record->blocksize)) 
       || record->fresh_image)
    dest = record->block_buf + record->blocksize;
  else
    dest = record->block_buf;
  fseek(record->blob, record->curidx * record->blocksize, SEEK_SET);
  record->last_read_size = fread(dest, 1, record->blocksize, record->blob);
}
*/

void mark_used(blockrecord record, size_t offset) {
  size_t idx = get_idx(record->blocksize, offset);
  record->used_blocks[idx] = 1;
  //record->startblock = idx;
}

void mark_current_used(blockrecord record) {
  record->used_blocks[record->curidx] = 1;
  record->startblock = record->curidx;
}

/* We've finished with the current image.  Start from a new
 * header offset and clear tried block info. */
void new_image(j_decompress_ptr cinfo, blockrecord record, size_t header_offset) {
  record->cur_offset = record->block_buf;
  size_t new_start_idx = get_idx(record->blocksize, header_offset);
  record->curidx = new_start_idx;
  record->startblock = new_start_idx;
  fseek(record->blob, record->curidx * record->blocksize, SEEK_SET);
  record->last_read_size = fread(record->block_buf + record->blocksize, 1, record->blocksize, 
                                 record->blob);
  record->fresh_image = 1;
  /* let libjpeg know that there's data in the buffer */
  cinfo->src->bytes_in_buffer = record->last_read_size;
  cinfo->src->next_input_byte = record->block_buf + record->blocksize;
}

void free_blockrecord(blockrecord record) {
  free(record->used_blocks);
  free(record);
}
