// dino.h
#ifndef DINO_H
#define DINO_H

#include "field.h"

//Структура Dino
typedef struct {
    int x;
    int y;
    char symbol;
    int is_placed;
    char color_under;
} Dino;

int init_dino(Dino *dino, int x, int y, int max_w, int max_h);
void place_dino_on_field(Dino *dino, Field *field);

#endif
