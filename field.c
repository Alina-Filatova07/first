// field.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "field.h"
#define MIN_SIZE 1
#define MAX_SIZE 100

//Инициализация поля: выделяет память, заполняет "_"
int init_field(Field *field, int w, int h) {
    if (!field || w < MIN_SIZE || w > MAX_SIZE || h < MIN_SIZE || h > MAX_SIZE) {
        return 0;
    }

    field->width = w;
    field->height = h;

    field->tiles = calloc(h, sizeof(Tile*));
    if (!field->tiles) return 0;

    for (int i = 0; i < h; ++i) {
        field->tiles[i] = calloc(w, sizeof(Tile));
        if (!field->tiles[i]) {
            for (int j = 0; j < i; ++j) free(field->tiles[j]);
            free(field->tiles);
            return 0;
        }
        for (int j = 0; j < w; ++j) {
            field->tiles[i][j].tile = '_';
            field->tiles[i][j].color = 0;
        }
    }
    return 1;
}

//Освобождение памяти
void free_field(Field *field) {
    if (!field || !field->tiles) return;
    for (int i = 0; i < field->height; ++i) {
        free(field->tiles[i]);
    }
    free(field->tiles);
    field->tiles = NULL;
}

//Печать поля в консоль
void print_field(const Field *field) {
    if (!field || !field->tiles) return;
    for (int y = 0; y < field->height; ++y) {
        for (int x = 0; x < field->width; ++x) {
            putchar(field->tiles[y][x].tile);
        }
        putchar('\n');
    }
}

//Сохранение поля в файл
void save_field_to_file(const Field *field, FILE *out) {
    if (!field || !out) return;
    for (int y = 0; y < field->height; ++y) {
        for (int x = 0; x < field->width; ++x) {
            fputc(field->tiles[y][x].tile, out);
        }
        fputc('\n', out);
    }
}

// Сохраняет цвет: если клетка — буква a-z
void set_tile(Field *field, int x, int y, char obj) {
    x = normalize_x(field, x);
    y = normalize_y(field, y);

    Tile *t = &field->tiles[y][x];

    // Если это цвет — сохраняем его, даже если ставим объект
    if (obj != '_' && obj >= 'a' && obj <= 'z') {
        t->color = obj;
        t->tile = obj;
    } else {
        // Объект или фон — сохраняем цвет, если он был
        if (t->tile == '_' || (t->tile >= 'a' && t->tile <= 'z')) {
            t->color = t->tile;
        }
        t->tile = obj;
    }
}

//Получение текущего символа (tile) клетки
char get_tile(const Field *field, int x, int y) {
    x = normalize_x(field, x);
    y = normalize_y(field, y);
    return field->tiles[y][x].tile;
}

// ОБЪЯВЛЕНИЯ normalize_x/y
int normalize_x(const Field *field, int x) {
    if (!field || field->width <= 0) return 0;
    int w = field->width;
    x = ((x % w) + w) % w;
    return x;
}
int normalize_y(const Field *field, int y) {
    if (!field || field->height <= 0) return 0;
    int h = field->height;
    y = ((y % h) + h) % h;
    return y;
}

