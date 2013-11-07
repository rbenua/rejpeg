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

#define DEFAULT_BLOCKSIZE 4096

void **find_jpeg_headers(void *search_start, size_t search_len);

void *next_block();

void *attempt_decode(void *start, size_t blocksize);

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

}
