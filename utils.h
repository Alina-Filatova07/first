// utils.h
#ifndef UTILS_H
#define UTILS_H
#include "command.h"
#include "field.h"

// Объявления функций:
int get_neighbor_coords(Field *field, int x, int y, enum Direction dir, int *nx, int *ny);
int is_blocking_tile(char tile);
int is_placeable(Tile t);
Field* copy_field(Field *src);

void clear_screen(void);
void sleep_seconds(int seconds);

#endif