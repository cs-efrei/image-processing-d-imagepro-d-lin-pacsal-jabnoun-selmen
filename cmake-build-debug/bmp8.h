// bmp8.h
#ifndef BMP8_H
#define BMP8_H

#include <stdio.h>
#include <stdlib.h>

// Informations des images
typedef struct {
    unsigned char header[54];       // En-tête BMP (54 octets)
    unsigned char colorTable[1024]; // Palette de couleurs (256 * 4)
    unsigned char *data;            // Données de pixels

    unsigned int width;      // Largeur de l’image
    unsigned int height;     // Hauteur de l’image
    unsigned int colorDepth; // Profondeur de couleur (doit être 8)
    unsigned int dataSize;   // Taille des données d’image
} t_bmp8;

// Fonctions :
t_bmp8 *bmp8_loadImage(const char *filename);
void bmp8_saveImage(const char *filename, t_bmp8 *img);
void bmp8_free(t_bmp8 *img);
void bmp8_printInfo(t_bmp8 *img);

// Filtres :
void bmp8_negative(t_bmp8 *img);
void bmp8_brightness(t_bmp8 *img, int value);
void bmp8_threshold(t_bmp8 *img, int threshold);


void bmp8_applyFilter(t_bmp8 *img, float **kernel, int kernelSize);

#endif
