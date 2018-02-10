## How it works

This implemention of color quantization builds an octree dividing the image color space into clusters, and then reduces it by folding first the leaves with lower pixel counts until the desired number of color is reached. The remaining leaves are the palette colors.
The leaves to reduce are located through heap sort, stage by stage, starting with the deepest level. A heap is filled with all nodes of a given level that are immediate parents of leaves, and sorted on the pixel count. When the heap is empty, a new one is built with leaves parent from the upper level, etc.

## About dithering

Error-diffusion dithering using a Sieria Lite matrix is available. When applied, instead of browsing the (post-reduction) octree to find the palette color matching an original color, we have to compare the original color to each one of the palette in order to find the closest.
This is because error diffusion will create new pixel values not part of the original, ie new colors that may not belong to any of the clusters in the octree. This results in a major slow down, as what was before a 8-step operation (the maximum depth of the octree) performed on each pixels becomes a 256-step operation.

## Results

TODO

## Caveats

Reducing an octree node means folding all its leaves onto iself. It is not possible to "partially" fold a node and preserve some of its leaves. As a consequence, it may not be possible to fill the palette with the exact maximum number of colors desired. This is especially blatant when setting the maximum number of colors lower than 8, which will always yield a palette of only 1 color.

## Useful links

On color quantization: https://www.imagemagick.org/Usage/quantize/  
On dithering : http://www.tannerhelland.com/4660/dithering-eleven-algorithms-source-code/  
