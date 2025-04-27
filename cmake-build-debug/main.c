// main.c

#include "bmp8.h"
#include "bmp24.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    t_bmp8 *img8 = NULL;
    t_bmp24 *img24 = NULL;
    int choix, valeur;
    char chemin[256];

    while (1) {
        printf("\n=== Menu ===\n");
        printf("1. Ouvrir une image\n");
        printf("2. Sauvegarder l'image\n");
        printf("3. Appliquer un filtre\n");
        printf("4. Afficher les informations\n");
        printf("5. Quitter\n> ");
        scanf("%d", &choix);

        if (choix == 1) {
            printf("Chemin du fichier : ");
            scanf("%s", chemin);

            img8 = bmp8_loadImage(chemin);
            if (img8) {
                if (img24) {
                    bmp24_free(img24);
                    img24 = NULL;
                }
                bmp8_printInfo(img8);
                printf("Image 8 bits ouvert\n");
            } else {
                img24 = bmp24_loadImage(chemin);
                if (img24) {
                    if (img8) {
                        bmp8_free(img8);
                        img8 = NULL;
                    }
                    printf("Image 24 bits ouvert\n");
                    printf("Largeur: %d, Hauteur: %d, Profondeur: %d\n", img24->width, img24->height, img24->colorDepth);
                } else {
                    printf("Fichier invalide\n");
                }
            }

        } else if (choix == 2) {
            if (img8) {
                bmp8_saveImage(img8, "output_8bit.bmp");
                printf("Image 8 bits sauvegardee\n");
            } else if (img24) {
                bmp24_saveImage(img24, "output_24bit.bmp");
                printf("Image 24 bits sauvegardée\n");
            } else {
                printf("Erreur : aucune image chargée\n");
            }

        } else if (choix == 3) {
            printf("\n--- Filtre à appliquer ---\n");
            printf("1. Négatif\n");
            printf("2. Luminosité\n");
            printf("3. Seuillage (seulement pour 8 bits)\n");
            printf("4. Flou (24 bits uniquement)\n");
            printf("5. Flou Gaussien (24 bits uniquement)\n");
            printf("6. Contours (24 bits uniquement)\n");
            printf("7. Relief (24 bits uniquement)\n");
            printf("8. Netteté (24 bits uniquement)\n");
            printf("> ");
            scanf("%d", &valeur);

            switch (valeur) {
                case 1:
                    if (img8) bmp8_negative(img8);
                    else if (img24) bmp24_negative(img24);
                    else printf("Erreur : aucune image.\n");
                    break;

                case 2: {
                    int lum;
                    printf("Valeur de luminosité : ");
                    scanf("%d", &lum);
                    if (img8) bmp8_brightness(img8, lum);
                    else if (img24) bmp24_brightness(img24, lum);
                    else printf("Erreur : aucune image.\n");
                    break;
                }

                case 3:
                    if (img8) {
                        int seuil;
                        printf("Seuil (0-255) : ");
                        scanf("%d", &seuil);
                        bmp8_threshold(img8, seuil);
                    } else {
                        printf("Erreur : seuillage seulement disponible pour les images 8 bits.\n");
                    }
                    break;

                case 4:
                    if (img24) {
                        float **kernel = getBoxBlurKernel();
                        bmp24_applyFilter(img24, kernel, 3);
                        freeKernel(kernel, 3);
                        printf("Filtre Flou appliqué.\n");
                    } else {
                        printf("Erreur : flou disponible seulement pour 24 bits.\n");
                    }
                    break;

                case 5:
                    if (img24) {
                        float **kernel = getGaussianBlurKernel();
                        bmp24_applyFilter(img24, kernel, 3);
                        freeKernel(kernel, 3);
                        printf("Filtre Flou Gaussien mis\n");
                    } else {
                        printf("Attention flou gaussien disponible seulement pour 24 bits\n");
                    }
                    break;

                case 6:
                    if (img24) {
                        float **kernel = getOutlineKernel();
                        bmp24_applyFilter(img24, kernel, 3);
                        freeKernel(kernel, 3);
                        printf("Détection des contours appliquée.\n");
                    } else {
                        printf("Attention détection contours disponible seulement pour 24 bits\n");
                    }
                    break;

                case 7:
                    if (img24) {
                        float **kernel = getEmbossKernel();
                        bmp24_applyFilter(img24, kernel, 3);
                        freeKernel(kernel, 3);
                        printf("Filtre Relief appliqué\n");
                    } else {
                        printf("Attention relief disponible seulement pour 24 bits\n");
                    }
                    break;

                case 8:
                    if (img24) {
                        float **kernel = getSharpenKernel();
                        bmp24_applyFilter(img24, kernel, 3);
                        freeKernel(kernel, 3);
                        printf("Filtre Netteté appliqué.\n");
                    } else {
                        printf("Erreur : netteté disponible seulement pour 24 bits.\n");
                    }
                    break;

                default:
                    printf("Choix invalide.\n");
                    break;
            }

        } else if (choix == 4) {
            if (img8) {
                printf("Image 8 bits :\n");
                bmp8_printInfo(img8);
            } else if (img24) {
                printf("Image 24 bits :\n");
                printf("- Largeur : %d px\n", img24->width);
                printf("- Hauteur : %d px\n", img24->height);
                printf("- Profondeur : %d bits\n", img24->colorDepth);
            } else {
                printf("Erreur : aucune image chargée.\n");
            }

        } else if (choix == 5) {
            printf("Fermeture du programme.\n");
            break;
        } else {
            printf("Choix invalide.\n");
        }
    }

    // Libération mémoire finale
    if (img8) bmp8_free(img8);
    if (img24) bmp24_free(img24);

    return 0;
}
