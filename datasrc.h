/*
 * datasrc.h
 *
 * Interface with the libjpeg decompression routine.
 */
#ifndef _DATASRC_H
#define _DATASRC_H

#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include "jpeg-6b/jinclude.h"
#include "jpeg-6b/jpeglib.h"
#include "jpeg-6b/jerror.h"


#include "blockrecord.h"

METHODDEF(void)
init_source (j_decompress_ptr cinfo);

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo);

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes);

METHODDEF(void)
term_source (j_decompress_ptr cinfo);

GLOBAL(void)
jpeg_blocks_src (j_decompress_ptr cinfo, FILE * infile, size_t blocksize,
                 struct stat *statbuf);

#endif
