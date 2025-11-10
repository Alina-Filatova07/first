// field.h
#ifndef FIELD_H
#define FIELD_H
#include <stdio.h>

typedef struct {
    char tile;
    char color;
} Tile;

typedef struct Field{
    int width, height;
    Tile **tiles;
} Field;

int init_field(Field *field, int w, int h); //Инициализация поля: выделяет память, заполняет "_"
void free_field(Field *field); //Освобождение памяти
void print_field(const Field *field); //Печать поля в консоль
void save_field_to_file(const Field *field, FILE *out); //Сохранение поля в файл
void set_tile(Field *field, int x, int y, char obj); //Установка символа объекта (сохраняет цвет фона)
char get_tile(const Field *field, int x, int y); //Получение текущего символа (tile) клетки

// ОБЪЯВЛЕНИЯ normalize_x/y
int normalize_x(const Field *field, int x);
int normalize_y(const Field *field, int y);

#endif


