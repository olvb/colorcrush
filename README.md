## Slim down your color space

Octrees are used in color quantization to divide the image color space into nested clusters. The deepest clusters are the leaves and they represent only one color. Upper nodes represent the average color of their children, taking into account the pixel count of each color. The octree is pruned until the desired number of color is reached, the remaining leaves forming the palette colors.

The method used to decide where to reduce the octree is roughly the one described in [this imagemagick document][1]. An error value is assigned to each node, and using heap sort, nodes with higher error values are folded in priority. The error value of a node is the sum of the differences between its average color and the color of its children (themselves also being average colors unless they are leaves). The difference between two colors is evaluated as the sum of squared differences between each channel, weighted by rough YUV-ish coefficients.

[1]: https://www.imagemagick.org/script/quantize.php

## Dithering

Dithering uses an error-diffusion [Floyd Steinberg matrix][2]. When applied, instead of browsing the pruned octree to find the palette color matching an original color, the original color must be compared to each color in the palette in order to find the closest match, using the same color difference formula described previously. This is because error diffusion will create new pixel values not included in the original image, therefore new colors that may not belong to any of the clusters in the octree.

Note that even without using dither, looking for the closest palette color yields better results than browsing the octree. The downside of this method is a major slowdown, as what was before a 8-step operation (the maximum depth of the octree) performed on each pixel becomes a 256-step operation.

[2]: http://www.tannerhelland.com/4660/dithering-eleven-algorithms-source-code/

## Results

*Click for full-scale image*

### Photographic image:

| Original | 256 colors | 256 colors dithered | ImageMagick |
| :------: | :--------: | :-----------------: | :---------: |
| [![][night_crop]][night] | [![][night_256_crop]][night_256] | [![][night_256_d_crop]][night_256_d] | [![][night_imgk_crop]][night_imgk] |
| 859 kB   | 231 kB     | 287 kB              | 336 kB      |

[night_crop]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/nightshot_cropped.png
[night_256_crop]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/nightshot_256_cropped.png
[night_256_d_crop]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/nightshot_256_d_cropped.png
[night_imgk_crop]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/nightshot_imgk_cropped.png
[night]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/nightshot.png
[night_256]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/nightshot_256.png
[night_256_d]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/nightshot_256_d.png
[night_imgk]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/nightshot_imgk.png

### Synthesized image:

| Original | 256 colors | 256 colors dithered | ImageMagick |
| :------: | :--------: | :-----------------: | :---------: |
| [![][art_crop]][art] | [![][art_256_crop]][art_256] | [![][art_256_d_crop]][art_256_d] | [![][art_imgk_crop]][art_imgk] |
| 454 kB   | 89 kB      | 130 kB              | 195 kB      |

[art_crop]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_cropped.png
[art_256_crop]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_256_cropped.png
[art_256_d_crop]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_256_d_cropped.png
[art_imgk_crop]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_imgk_cropped.png
[art]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial.png
[art_256]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_256.png
[art_256_d]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_256_d.png
[art_imgk]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_imgk.png

*ImageMagick command: `convert in.png -colors 256 png8:out.png`*

*Sample images from [imagecompression.info](https://imagecompression.info/test_images/)*

## Caveats

Reducing an octree node means folding all its leaves onto itself. It is not possible to "partially" fold a node and preserve some of its leaves. As a consequence, it may not be possible to fill the palette with the exact number of colors desired. This becomes obvious when setting a maximum number of colors lower than 8, which will always yield a palette of only 1 color: the average color of the image.

Limiting the octree depth can provide a speed gain (negligible when using dither), but also impact the chosen palette color. On some images (such as the second sample image), setting a lower depth may give results that are more visually convincing.
