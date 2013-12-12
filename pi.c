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

void *attempt_decode(int fd, size_t offset, size_t blocksize);

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
  
  for (int i = 0; i < num_headers; i++) {
    int decode_failed;
    decode_failed = attempt_decode(fd, header, blocksize, cinfo);
    if (decode_failed) {
      /* back up a scanline, swap input buffers */
    }
  }
}
