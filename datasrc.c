/*
 * datasrc.c
 *
 * Interface with the libjpeg decompression routine.
 * Modified from the libjpeg file: jpeg-6b/jdatasrc.c
 */
#include "datasrc.h"

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
  blockrecord src = (blockrecord) cinfo->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  src->fresh_image = 1;
}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 */

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
  blockrecord src = (blockrecord) cinfo->src;

  if (next_block(src) < 0) {
    src->decbuf[0] = (JOCTET) 0xFF;
    src->decbuf[1] = (JOCTET) JPEG_EOI;
    src->pub.bytes_in_buffer = 2;
    src->pub.next_input_byte = src->decbuf;
    return TRUE;
  }
  size_t nbytes = src->last_read_size;
  src->decbuf = src->cur_offset;
  
  if (nbytes <= 0) {
    if (src->fresh_image)	/* Treat empty input file as fatal error */
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
    WARNMS(cinfo, JWRN_JPEG_EOF);
    /* Insert a fake EOI marker */
    src->decbuf[0] = (JOCTET) 0xFF;
    src->decbuf[1] = (JOCTET) JPEG_EOI;
    nbytes = 2;
  }

  //src->pub.next_input_byte = src->decbuf;
  //src->pub.bytes_in_buffer = nbytes;
  if (src->fresh_image == 1) {
    src->fresh_image = 0;
    return TRUE;
  } else {
    /* If this isn't the first block we've read in, we need to kick back out
     * to the main loop to check if it's full of shit */
    return FALSE;
  }
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * If skipping would require reading another block: Grab another block and 
 * seek to the appropriate offset.  Otherwise, just jump forward in the block
 * (most likely case)
 */
METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  blockrecord src = (blockrecord) cinfo->src;

  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      (void) fill_input_buffer(cinfo);
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}



/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 */

METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}


/*
 * Initialize the block reader data source
 */

GLOBAL(void)
jpeg_blocks_src (j_decompress_ptr cinfo, FILE * infile, size_t blocksize,
                 struct stat *statbuf)
{
  blockrecord src;

  if (cinfo->src == NULL) {	/* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *) init_blockrecord(infile, blocksize, statbuf);
    src = (blockrecord) cinfo->src;
    src->decbuf = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  blocksize * SIZEOF(JOCTET));
  }

  src = (blockrecord) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->pub.bytes_in_buffer = 0; /* We fill this manually in new_image() */
  src->pub.next_input_byte = NULL; /* as above. */
}
