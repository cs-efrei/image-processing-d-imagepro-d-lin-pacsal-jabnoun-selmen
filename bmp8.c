#include "bmp8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

t_bmp8 *bmp8_loadImage(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("Erreur : Impossible d'ouvrir %s\n", filename);
        return NULL;
    }

    t_bmp8 *img = (t_bmp8 *)malloc(sizeof(t_bmp8));
    if (!img) {
        printf("Erreur : Allocation échouée\n");
        fclose(f);
        return NULL;
    }

    fread(img->header, 1, 54, f);
    fread(img->colorTable, 1, 1024, f);

    img->width       = *(unsigned int *)&img->header[18];
    img->height      = *(unsigned int *)&img->header[22];
    img->colorDepth  = *(unsigned short *)&img->header[28];
    img->dataSize    = *(unsigned int *)&img->header[34];

    if (img->colorDepth != 8) {
        printf("Erreur : L'image n'est pas en 8 bits.\n");
        fclose(f);
        free(img);
        return NULL;
    }

    if (img->dataSize == 0) {
        img->dataSize = img->width * img->height;
    }

    img->data = (unsigned char *)malloc(img->dataSize);
    fread(img->data, 1, img->dataSize, f);

    fclose(f);
    return img;
}

void bmp8_saveImage(const char *filename, t_bmp8 *img) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        printf("Erreur : Impossible d'écrire dans %s\n", filename);
        return;
    }

    if (img->dataSize == 0) {
        img->dataSize = img->width * img->height;
    }

    unsigned int fileSize = 54 + 1024 + img->dataSize;
    *(unsigned int *)&img->header[2] = fileSize;
    *(unsigned int *)&img->header[34] = img->dataSize;

    fwrite(img->header, 1, 54, f);
    fwrite(img->colorTable, 1, 1024, f);
    fwrite(img->data, 1, img->dataSize, f);

    fclose(f);
}

void bmp8_free(t_bmp8 *img) {
    if (img) {
        free(img->data);
        free(img);
    }
}

void bmp8_printInfo(t_bmp8 *img) {
    printf("Image Info:\n");
    printf("Width: %u\n", img->width);
    printf("Height: %u\n", img->height);
    printf("Color Depth: %u\n", img->colorDepth);
    printf("Data Size: %u\n", img->dataSize);
}

void bmp8_negative(t_bmp8 *img) {
    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = 255 - img->data[i];
    }
}

void bmp8_brightness(t_bmp8 *img, int value) {
    for (unsigned int i = 0; i < img->dataSize; i++) {
        int pixel = img->data[i] + value;
        if (pixel > 255) pixel = 255;
        if (pixel < 0) pixel = 0;
        img->data[i] = (unsigned char)pixel;
    }
}

void bmp8_threshold(t_bmp8 *img, int threshold) {
    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = (img->data[i] >= threshold) ? 255 : 0;
    }
}

void bmp8_applyFilter(t_bmp8 *img, float **kernel, int kernelSize) {
    int offset = kernelSize / 2;
    unsigned char *copy = (unsigned char *)malloc(img->dataSize);
    memcpy(copy, img->data, img->dataSize);

    for (unsigned int y = offset; y < img->height - offset; y++) {
        for (unsigned int x = offset; x < img->width - offset; x++) {
            float sum = 0.0;
            for (int i = -offset; i <= offset; i++) {
                for (int j = -offset; j <= offset; j++) {
                    int xi = x + j;
                    int yi = y + i;
                    int idx = yi * img->width + xi;
                    sum += kernel[i + offset][j + offset] * copy[idx];
                }
            }
            int idx = y * img->width + x;
            if (sum > 255) sum = 255;
            if (sum < 0) sum = 0;
            img->data[idx] = (unsigned char)sum;
        }
    }

    free(copy);
}
