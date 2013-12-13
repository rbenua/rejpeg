rejpeg
======

Reconstruct jpeg images from out-of-order blocks on disk

Usage: ./rejpeg image_file [blocksize]

Assumes blocks are 4k large by default.

rejpeg will scan the image file for jpeg headers (assumed to be aligned to
blocks) and then try to decode blocks until one fails to decode, back up
and move on to the next unused blocks then keep trying to decode scanlines
until we hit the end of the image.
