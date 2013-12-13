/* parse_image.c
 * 
 * This file provides the outer interface for reading a memory/disk 
 * image or binary blob and handing blocks off to the jpeg decoder to attempt
 * to reconstruct images.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <setjmp.h>
#include "jpeg-6b/jerror.h"

#include "datasrc.h"
#include "blockrecord.h"

#define DEFAULT_BLOCKSIZE 4096

struct my_error_mgr {
  struct jpeg_error_mgr pub;  /* "public" fields */

  jmp_buf setjmp_buffer;  /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

void my_error_emit(j_common_ptr cinfo){
  if(cinfo->err->msg_code == JWRN_HUFF_BAD_CODE){
    my_error_ptr err = (my_error_ptr)cinfo->err;
    longjmp(err->setjmp_buffer, 1);
  }
}

int find_jpeg_headers(struct jpeg_decompress_struct *cinfo,
                      FILE *infile, size_t blocksize, size_t **offsets) {
  *offsets = malloc(10 * sizeof(size_t));
  int count = 0;
  int size = 10;
  char inbuf[blocksize];
  size_t total_offset = 0;
  size_t read;
  while ((read = fread(inbuf, 1, blocksize, infile)) == blocksize) {
    if((*(unsigned int *)inbuf & 0x00FFFFFFF) == 0x00FFD8FF){
      printf("count is %d\n", count);
      //it's probably a jpeg.
      if(size == count){
        *offsets = realloc(*offsets, 2 * size * sizeof(size_t));
        size = size * 2;
      }
      *offsets[count] = total_offset;
      mark_used((blockrecord)cinfo->src, total_offset);
      count++;
    }
    total_offset += blocksize;
  }
  return count;
}

void attempt_decode(size_t header, size_t blocksize,
                     struct jpeg_decompress_struct *cinfo){
  printf("Decoding image starting at header %d\n", header);
  new_image(cinfo, (blockrecord) cinfo->src, header);
  jpeg_read_header(cinfo, TRUE);
  jpeg_start_decompress(cinfo);

  int row_stride = cinfo->output_width * cinfo->output_components;
  JSAMPARRAY buffer = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr)cinfo, JPOOL_IMAGE, row_stride, 1);

  while(cinfo->output_scanline < cinfo->output_height){
    const void *old_input_ptr = cinfo->src->next_input_byte;
    size_t old_left = cinfo->src->bytes_in_buffer;
    if(setjmp(((my_error_ptr)(cinfo->err))->setjmp_buffer)){
      //decoding failed - restore the thingus and try again
      //current_failed((blockrecord) cinfo->src);
      cinfo->src->next_input_byte = old_input_ptr - blocksize;
      cinfo->src->bytes_in_buffer = old_left % blocksize;
    }
    jpeg_read_scanlines(cinfo, buffer, 1);

    mark_current_used((blockrecord) cinfo->src);
  }
  printf("Ended with %d\n", ((blockrecord)cinfo->src)->curidx);
}

void usage(char **argv) {
  printf("Usage: %s file [blocksize]\n"
         "Block size defaults to %d.\n",
         argv[0], DEFAULT_BLOCKSIZE);
}
          
int main(int argc, char **argv) {
  if (argc < 2 || argc > 3) {
    usage(argv);
    exit(1);
  }

  size_t blocksize = 0;
  char *blobfile;
  /* get command line arguments */

  if (argc == 3) {
    blocksize = (size_t)atol(argv[2]);
    printf("Using block size %d\n", blocksize);
  }
  
  if (blocksize <= 0) {
    printf("No block size specified, defaulting to %d\n", DEFAULT_BLOCKSIZE);
    blocksize = DEFAULT_BLOCKSIZE;
  }

  blobfile = argv[1];

  /* Grab the image file */
  struct stat statbuf;
  FILE *infile;
  if (stat(blobfile, &statbuf) < 0) {
    char *error = strerror(errno);
    printf("Error statting image: %s\n", error);
  }
  if ((infile = fopen(blobfile, "rb")) < 0) {
    char *error = strerror(errno);
    printf("Error opening image: %s\n", error);
  }
  
  struct jpeg_decompress_struct cinfo;
  jpeg_create_decompress(&cinfo);
  
  /* Initialize the block-record data source */
  jpeg_blocks_src(&cinfo, infile, blocksize, &statbuf);

  size_t *header_offsets;
  int num_headers = find_jpeg_headers(&cinfo, infile, blocksize, &header_offsets);
  printf("Found %d headers:\n", num_headers);
  for(int i = 0; i < num_headers; i++){
    printf("%d, ", header_offsets[i]);
  }
  printf("\n");

  struct my_error_mgr err;
  cinfo.err = jpeg_std_error(&err.pub);
  err.pub.emit_message = &my_error_emit;
  
  void *image_blocks;
  for (int i = 0; i < num_headers; i++) {
    attempt_decode(header_offsets[i], blocksize, &cinfo);
    /* @TODO: chain found blocks together and dump to a file */
  }
}
