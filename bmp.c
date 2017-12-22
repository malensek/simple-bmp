#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct file_header {
    unsigned char header_field[2];
    uint32_t bitmap_size;
    unsigned char reserved1[2];
    unsigned char reserved2[2];
    uint32_t offset;
} __attribute__((__packed__));

struct dib_header {
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t color_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t raw_sz;
    int32_t hres;
    int32_t vres;
    uint32_t colors;
    uint32_t important_colors;
} __attribute__((__packed__));

int main(int argc, char *argv[]) {

    FILE *file = NULL;
    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("fopen");
    }

    struct file_header * header = malloc(sizeof(struct file_header));
    fread(header, sizeof(struct file_header), 1, file);

    printf("field: %c%c\n", header->header_field[0], header->header_field[1]);
    printf("sz: %d\n", header->bitmap_size);
    printf("offset: %d\n", header->offset);

    struct dib_header * dib = malloc(sizeof(struct dib_header));
    fread(dib, sizeof(struct dib_header), 1, file);

    printf("header_sz: %d\n", dib->header_size);
    printf("width: %d\n", dib->width);
    printf("height: %d\n", dib->height);
    printf("color planes: %d\n", dib->color_planes);
    printf("bpp: %d\n", dib->bits_per_pixel);
    printf("comp: %d\n", dib->compression);
    printf("sz: %d\n", dib->raw_sz);

    fseek(file, header->offset, SEEK_SET);

    uint32_t * bitmap = malloc(sizeof(uint32_t) * dib->width * dib->height);

    size_t bytes_per_pixel = dib->bits_per_pixel / 8;
    int rowsz = (int) floor((dib->bits_per_pixel * dib->width + 31) / 32);
    rowsz *= 4;
    int padding = rowsz - dib->width * bytes_per_pixel;
    char *pad = calloc(sizeof(char), padding);
    printf("rowsz: %d, padding: %d\n", rowsz, padding);
    int x, y;
    for (y = 0; y < dib->height; ++y) {
        for (x = 0; x < dib->width; ++x) {
            uint32_t bgr;
            fread(&bgr, bytes_per_pixel, 1, file);

            int z = dib->height - (y + 1);
            int idx = z * dib->width + x;

            printf("[%i]   (%d, %d) = %x\n", idx, x, z, bgr);
            bitmap[idx] = bgr;
        }
        fseek(file, padding, SEEK_CUR);
    }

    int i;
    for (i = 0; i < dib->width * dib->height; ++i) {
        unsigned char r = (bitmap[i] & 0xFF0000) >> 16;
        unsigned char g = (bitmap[i] & 0x00FF00) >> 8;
        unsigned char b = (bitmap[i] & 0x0000FF);
    }

    FILE *out = fopen("test.bmp", "w");
    dib->bits_per_pixel = 32;
    fwrite(header, sizeof(struct file_header), 1, out);
    fwrite(dib, sizeof(struct dib_header), 1, out);
    padding = 0;

    int idx;
    for (y = dib->height - 1; y >= 0; --y) {
        for (x = 0; x < dib->width; ++x) {
            int z = dib->height - (y + 1);
            int idx = z * dib->width + x;

            fwrite(&bitmap[idx], sizeof(uint32_t), 1, out);
        }
        if (padding > 0) {
            fwrite(pad, sizeof(char), padding, out);
        }
    }

    return 0;
}
