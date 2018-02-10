#pragma once
#include <stdbool.h>
#include "img.h"

/**
Quantize colors of true color @flat_img in a new @indexed_img.
@max_colors_count: maximum number that the quantized image may contain, ie maximum size of its palette. It makes sense
to set this to a power of 2.
@max_octree_depth: maximum depth of the octree. Must be between 1 and 8. If @param max_colors_counts is not
too low, lowering this value may speed up the quantization process without altering the output's quality.
@use_dither: if true, apply error-correction dithering. The process will be drasticly slowed down but depending on the image,
the visual result may be greatly improved.
@indexed_img must be uninitialized.
*/
void img_quantize(
    FlatImg *flat_img,
    unsigned int max_colors_count,
    unsigned int max_octree_depth,
    bool use_dither,
    IndexedImg *indexed_img
);
