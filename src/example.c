#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include <math.h>
#include <getopt.h>
#include <png.h>
#include "img.h"
#include "quantize.h"

void read_img_from_png(char *filename, FlatImg *img) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Could not open file \"%s\"\n", filename);
        exit(EXIT_FAILURE);
    }
    
    uint8_t signature[8];
    
    if (!fread(signature, 1, 8, file)) {
        fprintf(stderr, "Could not read file \"%s\"\n", filename);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    if (png_sig_cmp(signature, 0, 8) != 0) {
        fprintf(stderr, "\"%s\" is not a valid PNG file\n", filename);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop png_info = png_create_info_struct(png);
    jmp_buf png_jmp;
    setjmp(png_jmp);
    png_init_io(png, file);

    png_set_sig_bytes(png, 8);
    png_read_info(png, png_info);
    
    uint32_t width, height;
    int bit_depth, color_type;
    png_get_IHDR(png, png_info, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    } else if (color_type != PNG_COLOR_TYPE_RGB) {
        fprintf(stderr, "Only RGB or palette PNGs are supported\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    assert(bit_depth <= 8);
    if (bit_depth < 8) {
        png_set_packing(png);
    }
    if (color_type & PNG_COLOR_MASK_ALPHA) {
        png_set_strip_alpha(png);
    }

    png_read_update_info(png, png_info);

    size_t row_size = png_get_rowbytes(png, png_info);
    unsigned int channels_count = png_get_channels(png, png_info);
    uint8_t *png_data = malloc(row_size * height);
    // pointers to png_data to handle interlaced rows
    uint8_t **png_data_rows = malloc(height * sizeof(png_bytep));
    for (uint32_t i = 0; i < height; i++) {
        png_data_rows[i] = png_data + i * row_size;
    }
    png_read_image(png, png_data_rows);
    free(png_data_rows);
    png_data_rows = NULL;

    flat_img_init(img, width, height);
    // png data to internal data
    uint32_t pixels_count = width * height;
    for (uint32_t i = 0; i < pixels_count; i++) {
        img->data[i * 3 ] = png_data[i * channels_count];
        img->data[i * 3 + 1] = png_data[i * channels_count + 1];
        img->data[i * 3 + 2] = png_data[i * channels_count + 2];
    }

    free(png_data);
    png_destroy_read_struct(&png, &png_info, NULL);
    fclose(file);
}

void write_img_to_png(char *filename, IndexedImg *img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Could not open file \"%s\"\n", filename);
        exit(EXIT_FAILURE);
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop png_info = png_create_info_struct(png);
    jmp_buf png_jmp;
    setjmp(png_jmp);
    png_init_io(png, file);
    
    unsigned int bit_depth = ceil(log2(img->colors_count));
    assert(bit_depth <= 8);
    // valid PNG bit depths: 1, 2, 4, 8
    if (bit_depth == 0) {
        bit_depth = 1;
    }
    if (bit_depth > 1 && bit_depth % 2 == 1) {
        bit_depth += 1;
    }
    if (bit_depth == 6) {
        bit_depth = 8;
    }

    png_set_IHDR(
        png,
        png_info,
        img->width,
        img->height,
        bit_depth,
        PNG_COLOR_TYPE_PALETTE,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE
    );

    // internal palette to png palette
    png_color* palette = png_malloc(png, img->colors_count * sizeof(png_color));
    for (uint8_t i = 0; i < img->colors_count; i++) {
        palette[i].red = img->palette[i * 3];
        palette[i].green = img->palette[i * 3 + 1];
        palette[i].blue = img->palette[i * 3 + 2];
    }
    png_set_PLTE(png, png_info, palette, img->colors_count);

    png_write_info(png, png_info);
    
    if (bit_depth < 8) {
        png_set_packing(png);
    }
    
    // pointers to img data
    png_bytep row = malloc(img->width * sizeof(png_byte));
    for (uint32_t y = 0; y < img->height; y++) {
        for (uint32_t x = 0; x < img->width; x++) {
            row[x] = img->data[y * img->width + x];
        }
        png_write_row(png, row);
    }
    free(row);
    row = NULL;
    png_write_end(png, NULL);

    png_destroy_write_struct(&png, &png_info);
    fclose(file);
}

void print_usage_and_exit(char *exec_name) {
    fprintf(stderr, "Usage: %s [options] <in.png> <out.png>\n", exec_name);
    fprintf(stderr, "  -c <colors_count>\tMaximum number of colors in palette [1-256, default: 256]\n");
    fprintf(stderr, "  -d\t\t\tUse dithering\n");
    fprintf(stderr, "  -o <octree_depth>\tMaximum octree depth (lower values can speed up process) [1-8, default: 6]\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    unsigned int max_colors_count = 256;
    unsigned int max_octree_depth = 6;
    bool use_dither = false;

    char opt;
    while ((opt = getopt(argc, argv, "c:do:")) != -1) {
        switch (opt) {
        case 'c':
            max_colors_count = atoi(optarg);
            if (max_colors_count < 1 || max_colors_count > 256) {
                fprintf(stderr, "Colors count must be between 1 and 256\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'd':
            use_dither = true;
            break;
        case 'o':
            max_octree_depth = atoi(optarg);
            if (max_octree_depth < 1 || max_octree_depth > 8) {
                fprintf(stderr, "Octree depth count must be between 1 and 8\n");
                exit(EXIT_FAILURE);
            }
            break;
        default:
            print_usage_and_exit(argv[0]);
        }
    }

    if (argc - optind != 2) {
        print_usage_and_exit(argv[0]);
    }

    char *input_filename = argv[optind];
    char *output_filename = argv[optind + 1];

    FlatImg flat_img;
    read_img_from_png(input_filename, &flat_img);
    IndexedImg indexed_img;
    img_quantize(&flat_img, max_colors_count, max_octree_depth, use_dither, &indexed_img);
    write_img_to_png(output_filename, &indexed_img);

    flat_img_clear(&flat_img);
    indexed_img_clear(&indexed_img);
}
