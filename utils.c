// utils.c
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif


#include "field.h" 
#include "parser.h"
#include "utils.h"

int get_neighbor_coords(Field *field, int x, int y, enum Direction dir, int *nx, int *ny) {
    if (!field || !nx || !ny) return 0;

    *nx = x;
    *ny = y;

    switch (dir) {
        case DIR_UP:    (*ny)--; break;
        case DIR_DOWN:  (*ny)++; break;
        case DIR_LEFT:  (*nx)--; break;
        case DIR_RIGHT: (*nx)++; break;
        default: return 0;
    }

    *nx = normalize_x(field, *nx);  // ← вызов из field.c
    *ny = normalize_y(field, *ny);  // ← вызов из field.c
    return 1;
}

int is_blocking_tile(char tile) {
    return tile == '^' || tile == '&' || tile == '@';
}

int is_placeable(Tile t) {
    return t.tile == '_' || (t.tile >= 'a' && t.tile <= 'z');
}

Field* copy_field(Field *src) {
    if (!src) return NULL;

    Field *dst = malloc(sizeof(Field));
    if (!dst) return NULL;

    *dst = *src;

    dst->tiles = malloc(src->height * sizeof(Tile*));
    if (!dst->tiles) {
        free(dst);
        return NULL;
    }

    for (int y = 0; y < src->height; y++) {
        dst->tiles[y] = malloc(src->width * sizeof(Tile));
        if (!dst->tiles[y]) {
            for (int i = 0; i < y; i++) {
                free(dst->tiles[i]);
            }
            free(dst->tiles);
            free(dst);
            return NULL;
        }
        memcpy(dst->tiles[y], src->tiles[y], src->width * sizeof(Tile));
    }

    return dst;
}

void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void sleep_seconds(int seconds) {
    if (seconds <= 0) return;

#ifdef _WIN32
    Sleep(seconds * 1000);
#else
    sleep(seconds);
#endif
}


