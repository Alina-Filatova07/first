// dino.c
#include "dino.h"
#include <string.h>
#include "field.h"


//Инициализация Dino
int init_dino(Dino *dino, int x, int y, int max_w, int max_h) {
    if (!dino || x < 0 || y < 0 || x >= max_w || y >= max_h)
        return 0;
    dino->x = x;
    dino->y = y;
    dino->is_placed = 0;
    dino->color_under = 0;
    return 1;
}

//Функция Dino на поле
void place_dino_on_field(Dino *dino, Field *field) {
    if (!dino || !field || dino->x < 0 || dino->y < 0) return;
    int x = normalize_x(field, dino->x);
    int y = normalize_y(field, dino->y);
    Tile *t = &field->tiles[y][x];
    dino->color_under = t->tile;
    t->tile = '#';
    dino->is_placed = 1;
}



