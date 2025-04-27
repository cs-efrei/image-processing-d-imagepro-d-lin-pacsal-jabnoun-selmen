// bmp24.c

#include "bmp24.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

int clamp(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}

t_pixel **bmp24_allocateDataPixels(int width, int height) {
    t_pixel **data = malloc(height * sizeof(t_pixel *));
    if (!data) return NULL;

    for (int i = 0; i < height; i++) {
        data[i] = malloc(width * sizeof(t_pixel));
        if (!data[i]) return NULL;
    }

    return data;
}

void bmp24_freeDataPixels(t_pixel **pixels, int height) {
    for (int i = 0; i < height; i++) {
        free(pixels[i]);
    }
    free(pixels);
}

t_bmp24 *bmp24_allocate(int width, int height, int colorDepth) {
    t_bmp24 *img = malloc(sizeof(t_bmp24));
    if (!img) return NULL;

    img->width = width;
    img->height = height;
    img->colorDepth = colorDepth;
    img->data = bmp24_allocateDataPixels(width, height);

    return img;
}

void bmp24_free(t_bmp24 *img) {
    if (!img) return;
    bmp24_freeDataPixels(img->data, img->height);
    free(img);
}

void file_rawRead(uint32_t position, void *buffer, uint32_t size, size_t n, FILE *file) {
    fseek(file, position, SEEK_SET);
    fread(buffer, size, n, file);
}

void file_rawWrite(uint32_t position, void *buffer, uint32_t size, size_t n, FILE *file) {
    fseek(file, position, SEEK_SET);
    fwrite(buffer, size, n, file);
}

t_bmp24 *bmp24_loadImage(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erreur ouverture fichier : %s\n", filename);
        return NULL;
    }

    t_bmp24 *img = malloc(sizeof(t_bmp24));
    if (!img) {
        fclose(file);
        return NULL;
    }

    file_rawRead(0x00, &img->header, sizeof(t_bmp_header), 1, file);
    file_rawRead(0x0E, &img->header_info, sizeof(t_bmp_info), 1, file);

    img->width = img->header_info.width;
    img->height = img->header_info.height;
    img->colorDepth = img->header_info.bits;

    if (img->colorDepth != 24) {
        printf("Erreur : image non 24 bits\n");
        fclose(file);
        free(img);
        return NULL;
    }

    img->data = bmp24_allocateDataPixels(img->width, img->height);
    fseek(file, img->header.offset, SEEK_SET);

    int rowSize = ((img->width * 3 + 3) / 4) * 4;
    uint8_t *row = malloc(rowSize);

    for (int i = img->height - 1; i >= 0; i--) {
        fread(row, 1, rowSize, file);
        for (int j = 0; j < img->width; j++) {
            img->data[i][j].blue = row[j * 3 + 0];
            img->data[i][j].green = row[j * 3 + 1];
            img->data[i][j].red = row[j * 3 + 2];
        }
    }

    free(row);
    fclose(file);
    return img;
}

void bmp24_writePixelValue(t_bmp24 *img, int x, int y, FILE *file) {
    uint8_t bgr[3] = {
        img->data[y][x].blue,
        img->data[y][x].green,
        img->data[y][x].red
    };
    fwrite(bgr, 1, 3, file);
}

void bmp24_writePixelData(t_bmp24 *img, FILE *file) {
    int rowSize = ((img->width * 3 + 3) / 4) * 4;
    int padding = rowSize - (img->width * 3);
    uint8_t pad[3] = {0, 0, 0};

    for (int y = img->height - 1; y >= 0; y--) {
        for (int x = 0; x < img->width; x++) {
            bmp24_writePixelValue(img, x, y, file);
        }
        fwrite(pad, 1, padding, file);
    }
}

void bmp24_saveImage(t_bmp24 *img, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Erreur création fichier\n");
        return;
    }

    file_rawWrite(0x00, &img->header, sizeof(t_bmp_header), 1, file);
    file_rawWrite(0x0E, &img->header_info, sizeof(t_bmp_info), 1, file);
    fseek(file, img->header.offset, SEEK_SET);

    bmp24_writePixelData(img, file);

    fclose(file);
}

void bmp24_negative(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x].red = 255 - img->data[y][x].red;
            img->data[y][x].green = 255 - img->data[y][x].green;
            img->data[y][x].blue = 255 - img->data[y][x].blue;
        }
    }
}

void bmp24_grayscale(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            uint8_t gray = (img->data[y][x].red + img->data[y][x].green + img->data[y][x].blue) / 3;
            img->data[y][x].red = gray;
            img->data[y][x].green = gray;
            img->data[y][x].blue = gray;
        }
    }
}

void bmp24_brightness(t_bmp24 *img, int value) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x].red = clamp(img->data[y][x].red + value);
            img->data[y][x].green = clamp(img->data[y][x].green + value);
            img->data[y][x].blue = clamp(img->data[y][x].blue + value);
        }
    }
}

t_pixel bmp24_convolution(t_bmp24 *img, int x, int y, float **kernel, int kernelSize) {
    int offset = kernelSize / 2;
    float r = 0, g = 0, b = 0;

    for (int i = -offset; i <= offset; i++) {
        for (int j = -offset; j <= offset; j++) {
            int xi = x + j;
            int yi = y + i;
            if (xi < 0 || xi >= img->width || yi < 0 || yi >= img->height) continue;
            float coeff = kernel[i + offset][j + offset];
            r += img->data[yi][xi].red * coeff;
            g += img->data[yi][xi].green * coeff;
            b += img->data[yi][xi].blue * coeff;
        }
    }

    t_pixel p;
    p.red = clamp(round(r));
    p.green = clamp(round(g));
    p.blue = clamp(round(b));
    return p;
}

void bmp24_applyFilter(t_bmp24 *img, float **kernel, int kernelSize) {
    t_pixel **copy = bmp24_allocateDataPixels(img->width, img->height);

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            copy[y][x] = bmp24_convolution(img, x, y, kernel, kernelSize);
        }
    }

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x] = copy[y][x];
        }
    }

    bmp24_freeDataPixels(copy, img->height);
}

// Filtres standards
float **getBoxBlurKernel() {
    float **kernel = malloc(3 * sizeof(float *));
    for (int i = 0; i < 3; i++) {
        kernel[i] = malloc(3 * sizeof(float));
        for (int j = 0; j < 3; j++) {
            kernel[i][j] = 1.0f / 9.0f;
        }
    }
    return kernel;
}

float **getGaussianBlurKernel() {
    float values[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}
    };

    float **kernel = malloc(3 * sizeof(float *));
    for (int i = 0; i < 3; i++) {
        kernel[i] = malloc(3 * sizeof(float));
        for (int j = 0; j < 3; j++) {
            kernel[i][j] = values[i][j] / 16.0f;
        }
    }
    return kernel;
}

float **getOutlineKernel() {
    float values[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

    float **kernel = malloc(3 * sizeof(float *));
    for (int i = 0; i < 3; i++) {
        kernel[i] = malloc(3 * sizeof(float));
        for (int j = 0; j < 3; j++) {
            kernel[i][j] = values[i][j];
        }
    }
    return kernel;
}

float **getEmbossKernel() {
    float values[3][3] = {
        {-2, -1, 0},
        {-1,  1, 1},
        { 0,  1, 2}
    };

    float **kernel = malloc(3 * sizeof(float *));
    for (int i = 0; i < 3; i++) {
        kernel[i] = malloc(3 * sizeof(float));
        for (int j = 0; j < 3; j++) {
            kernel[i][j] = values[i][j];
        }
    }
    return kernel;
}

float **getSharpenKernel() {
    float values[3][3] = {
        { 0, -1,  0},
        {-1,  5, -1},
        { 0, -1,  0}
    };

    float **kernel = malloc(3 * sizeof(float *));
    for (int i = 0; i < 3; i++) {
        kernel[i] = malloc(3 * sizeof(float));
        for (int j = 0; j < 3; j++) {
            kernel[i][j] = values[i][j];
        }
    }
    return kernel;
}

void freeKernel(float **kernel, int size) {
    for (int i = 0; i < size; i++) {
        free(kernel[i]);
    }
    free(kernel);
}
