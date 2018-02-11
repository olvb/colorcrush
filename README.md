## How it works

Octrees are used in color quantization to divide the image color space into nested clusters. The deepest clusters are the leaves and they represent only one color. The upper nodes represent the average color of their children, taking into account the pixel count of each color. The octree is reduced until the desired number of color is reached, the remaining leaves forming the palette colors.

The method used to decide in which order to the octree is roughly the one described in [this imagemagick document][1]: an error value is assigned to each node, and using heap sort, nodes with higher error values are decimated in priority. The error value of a node is the sum of the difference between its average color and its children's colors (which is itself an average color unless they are leaves). The difference between to color is evaluted to the sum of the squares of the difference between each channel, ponderated by rough YUV-ish coefficients (which give smoother images).

[1]: https://www.imagemagick.org/script/quantize.php

## Dithering

Dithering uses an error-diffusion [Floyd Steinberg matrix][2]. When applied, instead of browsing the (post-reduction) octree to find the palette color matching an original color, the original color must be compared to each one of the palette in order to find the closest, using the same color difference formula described previously. This is because error diffusion will create new pixel values not part of the original, ie new colors that may not belong to any of the clusters in the octree.

Note that even without using dither, looking for the closest palette color would yield better results than browsing the octree. The downside is a major slow down, as what was before a 8-step operation (the maximum depth of the octree) performed on each pixel becomes a 256-step operation.

[2]: http://www.tannerhelland.com/4660/dithering-eleven-algorithms-source-code/

## Results

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
| [![][art_crop]][night] | [![][art_256_crop]][art_256] | [![][art_256_d_crop]][art_256_d] | [![][art_imgk_crop]][art_imgk] |
| 454 kB   | 89 kB      | 130 kB              | 195 kB      |

[art_crop]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_cropped.png
[art_256_crop]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_256_cropped.png
[art_256_d_crop]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_256_d_cropped.png
[art_imgk_crop]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_imgk_cropped.png
[art]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial.png
[art_256]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_256.png
[art_256_d]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_256_d.png
[art_imgk]: https://raw.githubusercontent.com/olivierbbb/colorcrush/master/samples/artificial_imgk.png

*Sample images from [imagecompression.info](https://imagecompression.info/test_images/)*

## Caveats

Reducing an octree node means folding all its leaves onto iself. It is not possible to "partially" fold a node and preserve some of its leaves. As a consequence, it may not be possible to fill the palette with the exact maximum number of colors desired. This is especially blatant when setting a maximum number of colors lower than 8, which will always yield a palette of only 1 color.

Limiting the octree depth can give a performance gain (neglectable when using dither), but also impact the chosen palette color. On some images (such as our second sample image), setting a lower depth may give results more visually pleasing.
