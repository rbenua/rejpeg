#ifndef _BLOCKRECORD_H
#define _BLOCKRECORD_H

#include "jpeg-6b/jinclude.h"
#include "jpeg-6b/jpeglib.h"
#include "jpeg-6b/jerror.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "datasrc.h"

struct blockrecord_s {
  struct jpeg_source_mgr pub;
  
  JOCTET *decbuf;

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
  /* A circular buffer containing our current and previous block data */
  void *block_buf;
  /* Current point in the block buffer*/
  void *cur_offset;
  /* 1 if we haven't read in any blocks from the new image yet, 0 otherwise */
  char fresh_image;
  
  size_t last_read_size;

};
typedef struct blockrecord_s * blockrecord;

size_t get_idx(size_t blocksize, size_t block_offset);

blockrecord init_blockrecord(FILE *infile, size_t blocksize, struct stat *statbuf);

void *next_block(blockrecord record);

void mark_used(blockrecord record, size_t offset);

void mark_current_used(blockrecord record);

void new_image(blockrecord record, size_t header_offset);

void free_blockrecord(blockrecord record);

#endif
