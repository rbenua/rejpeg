/* parse_image.c
 * 
 * This file provides the outer interface for reading a memory/disk 
 * image or binary blob and handing blocks off to the jpeg decoder to attempt
 * to reconstruct images.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define DEFAULT_BLOCKSIZE 4096

void **find_jpeg_headers(int fd, struct stat *statbuf, size_t *offsets);

void *next_block(int fd, size_t cur_offset);

void *attempt_decode(int fd, size_t offset, size_t blocksize,
                     struct jpeg_decompress_struct *cinfo,
                     struct jpeg_decompress_struct *cinfo_backup){
  
  rejpeg_source(int fd, size_t header_offset, size_t blocksize, cinfo);
  jpeg_read_header(cinfo, TRUE);
  jpeg_start_decompress(cinfo);

  int row_stride = cinfo->output_width * cinfo->output_components;
  JSAMPARRAY buffer = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr)cinfo, JPOOL_IMAGE, row_stride, 1);

  while(cinfo->output_scanline < cinfo->height){
    backup_info(cinfo, cinfo_backup);
    if(setjmp(jump_buffer_from_someplace)){
      //decoding failed - restore the thingus and try again
      restore_info(cinfo, cinfo_backup);
    }
    crossed_blocks = 0;
    jpeg_read_scanlines(cinfo, buffer, 1);

    if(crossed_blocks == 1 && entropy_failed(buffer)){
      restore_info(cinfo, cinfo_backup);
    }
  }
}

void usage(char **argv) {
  printf("Usage: %s [options] file\n"
         "Options:\n"
         "-b    Set block size [Default: %d]\n",
         argv[0], DEFAULT_BLOCKSIZE);
}
          
int main(int argc, char **argv) {
  if (argc < 2) {
    usage(argv);
    exit(1);
  }

  size_t blocksize = 0;
  char *blobfile;
  /* get command line arguments */

  opterr = 0;
  char c;
  while ((c = getopt(argc, argv, "b")) != -1) {
    switch (c) {
      case 'b':
        blocksize = (size_t)atol(optarg);
        break;
      case '?':
      default:
        usage(argv);
        exit(1);
    }
  }
  
  if (blocksize <= 0) {
    printf("No block size specified, defaulting to %d\n", DEFAULT_BLOCKSIZE);
    blocksize = DEFAULT_BLOCKSIZE;
  }

  blobfile = argv[optind];

  /* Grab the image file */
  struct stat statbuf;
  int fd;
  if (stat(blobfile, &statbuf) < 0) {
    char *error = strerror(errno);
    printf("Error statting image: %s\n", error);
  }
  if ((fd = open(blobfile, O_RDONLY)) < 0) {
    char *error = strerror(errno);
    printf("Error opening image: %s\n", error);
  }

  size_t *header_offsets;
  int num_headers = find_jpeg_headers(fd, &statbuf, header_offsets);
  
  struct jpeg_decompress_struct cinfo, cinfo_backup;
  jpeg_create_decompress(&cinfo);
  jpeg_create_decompress(&cinfo_backup);

  for (int i = 0; i < num_headers; i++) {
    attempt_decode(fd, header, blocksize, &cinfo, &cinfo_backup);
  }

}
