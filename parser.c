// parser.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "command.h"
#include "utils.h"
#include "dino.h"

//Преобразует строку-направление в перечисление Direction
enum Direction parse_direction(const char *str) {
    if (!str) return DIR_NONE;
    if (strcmp(str, "UP") == 0) return DIR_UP;
    if (strcmp(str, "DOWN") == 0) return DIR_DOWN;
    if (strcmp(str, "LEFT") == 0) return DIR_LEFT;
    if (strcmp(str, "RIGHT") == 0) return DIR_RIGHT;
    return DIR_NONE;
}

//Проверяет, может ли символ c быть цветом при команде PAINT
int is_valid_char_for_paint(char c) {
    return c >= 'a' && c <= 'z';
}

//Основная функция-интерпретатор команд
int execute_line(const char *line, Field *field, Dino *dino, int line_num) {
    if (!line || !field || !dino) return 0;

    // Пропускаем пустые строки и комментарии
    if (line[0] == '\0' || line[0] == '\n' || line[0] == '\r') return 1;
    if (strncmp(line, "//", 2) == 0) return 1;

    char cmd_buf[64];
    if (sscanf(line, "%63s", cmd_buf) != 1) {
        fprintf(stderr, "Ошибка в строке %d: не распознана команда\n", line_num);
        return 0;
    }

    // SIZE
    if (strcmp(cmd_buf, "SIZE") == 0) {
        int w, h;
        if (sscanf(line, "SIZE %d %d", &w, &h) != 2) {
            fprintf(stderr, "Ошибка в строке %d: неверный формат SIZE\n", line_num);
            return 0;
        }
        if (!init_field(field, w, h)) {
            fprintf(stderr, "Ошибка в строке %d: недопустимый размер поля\n", line_num);
            return 0;
        }
        return 1;
    }

    // START
    if (strcmp(cmd_buf, "START") == 0) {
        int x, y;
        if (sscanf(line, "START %d %d", &x, &y) != 2) {
            fprintf(stderr, "Ошибка в строке %d: неверный формат START\n", line_num);
            return 0;
        }
        if (!init_dino(dino, x, y, field->width, field->height)) {
            fprintf(stderr, "Ошибка в строке %d: START вне поля\n", line_num);
            return 0;
        }
        place_dino_on_field(dino, field);
        return 1;
    }

    // MOVE
    if (strcmp(cmd_buf, "MOVE") == 0) {
        char dir_str[16];
        if (sscanf(line, "MOVE %15s", dir_str) != 1) {
            fprintf(stderr, "Ошибка в строке %d: неверный формат MOVE\n", line_num);
            return 0;
        }
        enum Direction dir = parse_direction(dir_str);
        if (dir == DIR_NONE) {
            fprintf(stderr, "Ошибка в строке %d: неизвестное направление\n", line_num);
            return 0;
        }

        // Убрать динозавра со старого места: восстановить фон (цвет или '_')
        Tile *old_tile = &field->tiles[dino->y][dino->x];
        old_tile->tile = old_tile->color ? old_tile->color : '_';

        // Новые координаты (тор)
        int nx = dino->x, ny = dino->y;
        switch (dir) {
            case DIR_UP:    ny--; break;
            case DIR_DOWN:  ny++; break;
            case DIR_LEFT:  nx--; break;
            case DIR_RIGHT: nx++; break;
        }
        nx = normalize_x(field, nx);
        ny = normalize_y(field, ny);

        Tile *target_tile = &field->tiles[ny][nx];
        char target = target_tile->tile;

        if (target == '%') {
            fprintf(stderr, "Ошибка в строке %d: динозавр упал в яму!\n", line_num);
            return 0;
        }
        if (target == '^' || target == '&' || target == '@') {
            printf("Предупреждение: движение заблокировано в строке %d\n", line_num);
            // Возвращаем динозавра на старое место
            place_dino_on_field(dino, field);
            return 1;
        }

        dino->x = nx;
        dino->y = ny;
        place_dino_on_field(dino, field);
        return 1;
    }

    // PAINT
    if (strcmp(cmd_buf, "PAINT") == 0) {
        char c;
        if (sscanf(line, "PAINT %c", &c) != 1 || !is_valid_char_for_paint(c)) {
            fprintf(stderr, "Ошибка в строке %d: PAINT требует букву a-z\n", line_num);
            return 0;
        }
        if (!dino->is_placed) {
            fprintf(stderr, "Ошибка в строке %d: динозавр не создан\n", line_num);
            return 0;
        }
        Tile *t = &field->tiles[dino->y][dino->x];
        t->color = c;
        // Не перезаписываем tile, если там не фон — динозавр может быть поверх цвета
        if (t->tile == '_' || (t->tile >= 'a' && t->tile <= 'z')) {
            t->tile = c;
        }
        return 1;
    }

    // DIG
    if (strcmp(cmd_buf, "DIG") == 0) {
        char dir_str[16];
        if (sscanf(line, "DIG %15s", dir_str) != 1) {
            fprintf(stderr, "Ошибка в строке %d: неверный формат DIG\n", line_num);
            return 0;
        }
        enum Direction dir = parse_direction(dir_str);
        if (dir == DIR_NONE) {
            fprintf(stderr, "Ошибка в строке %d: неизвестное направление\n", line_num);
            return 0;
        }

        int nx, ny;
        if (!get_neighbor_coords(field, dino->x, dino->y, dir, &nx, &ny)) {
            fprintf(stderr, "Ошибка в строке %d: невозможно вычислить соседа\n", line_num);
            return 0;
        }

        Tile *t = &field->tiles[ny][nx];
        if (t->tile == '%') {
            return 1; // уже яма — ничего не делаем
        }
        if (t->tile == '^') {
            // Гору убираем → остаётся цвет (если был), иначе '_'
            t->tile = t->color ? t->color : '_';
        } else {
            // Создаём яму, но сохраняем цвет
            // Яма — это '%', но цвет остаётся для будущего восстановления
            t->tile = '%';
        }
        return 1;
    }

    // MOUND
    if (strcmp(cmd_buf, "MOUND") == 0) {
        char dir_str[16];
        if (sscanf(line, "MOUND %15s", dir_str) != 1) {
            fprintf(stderr, "Ошибка в строке %d: неверный формат MOUND\n", line_num);
            return 0;
        }
        enum Direction dir = parse_direction(dir_str);
        if (dir == DIR_NONE) {
            fprintf(stderr, "Ошибка в строке %d: неизвестное направление\n", line_num);
            return 0;
        }

        int nx, ny;
        if (!get_neighbor_coords(field, dino->x, dino->y, dir, &nx, &ny)) {
            fprintf(stderr, "Ошибка в строке %d: невозможно вычислить соседа\n", line_num);
            return 0;
        }

        Tile *t = &field->tiles[ny][nx];
        if (t->tile == '%') {
            // Яму → гора: цвет сохраняется, tile = '^'
            t->tile = '^';
            return 1;
        }
        if (is_placeable(*t)) {
            t->tile = '^';
            return 1;
        }
        fprintf(stderr, "Ошибка в строке %d: нельзя создать гору на препятствии\n", line_num);
        return 0;
    }

    // JUMP
    if (strcmp(cmd_buf, "JUMP") == 0) {
        char dir_str[16];
        int n;
        if (sscanf(line, "JUMP %15s %d", dir_str, &n) != 2 || n <= 0) {
            fprintf(stderr, "Ошибка в строке %d: JUMP требует направление и n>0\n", line_num);
            return 0;
        }
        enum Direction dir = parse_direction(dir_str);
        if (dir == DIR_NONE) {
            fprintf(stderr, "Ошибка в строке %d: неизвестное направление\n", line_num);
            return 0;
        }

        // Убрать динозавра
        Tile *old_tile = &field->tiles[dino->y][dino->x];
        old_tile->tile = old_tile->color ? old_tile->color : '_';

        int nx = dino->x, ny = dino->y;
        int hit_obstacle = 0;

        for (int step = 1; step <= n && !hit_obstacle; step++) {
            switch (dir) {
                case DIR_UP:    ny--; break;
                case DIR_DOWN:  ny++; break;
                case DIR_LEFT:  nx--; break;
                case DIR_RIGHT: nx++; break;
            }
            nx = normalize_x(field, nx);
            ny = normalize_y(field, ny);

            char tile = field->tiles[ny][nx].tile;
            if (is_blocking_tile(tile)) {
                // Откат на 1 шаг назад
                switch (dir) {
                    case DIR_UP:    ny++; break;
                    case DIR_DOWN:  ny--; break;
                    case DIR_LEFT:  nx++; break;
                    case DIR_RIGHT: nx--; break;
                }
                nx = normalize_x(field, nx);
                ny = normalize_y(field, ny);
                hit_obstacle = 1;
                printf("Предупреждение: прыжок заблокирован в строке %d\n", line_num);
            }
        }

        Tile *landing_tile = &field->tiles[ny][nx];
        if (landing_tile->tile == '%') {
            fprintf(stderr, "Ошибка в строке %d: динозавр упал в яму при прыжке!\n", line_num);
            return 0;
        }

        dino->x = nx;
        dino->y = ny;
        place_dino_on_field(dino, field);
        return 1;
    }

    // GROW
    if (strcmp(cmd_buf, "GROW") == 0) {
        char dir_str[16];
        if (sscanf(line, "GROW %15s", dir_str) != 1) {
            fprintf(stderr, "Ошибка в строке %d: неверный формат GROW\n", line_num);
            return 0;
        }
        enum Direction dir = parse_direction(dir_str);
        if (dir == DIR_NONE) {
            fprintf(stderr, "Ошибка в строке %d: неизвестное направление\n", line_num);
            return 0;
        }

        int nx, ny;
        if (!get_neighbor_coords(field, dino->x, dino->y, dir, &nx, &ny)) return 0;

        Tile *t = &field->tiles[ny][nx];
        if (!is_placeable(*t)) {
            fprintf(stderr, "Ошибка в строке %d: нельзя вырастить дерево на занятой клетке\n", line_num);
            return 0;
        }
        t->tile = '&';
        return 1;
    }

    // CUT
    if (strcmp(cmd_buf, "CUT") == 0) {
        char dir_str[16];
        if (sscanf(line, "CUT %15s", dir_str) != 1) {
            fprintf(stderr, "Ошибка в строке %d: неверный формат CUT\n", line_num);
            return 0;
        }
        enum Direction dir = parse_direction(dir_str);
        if (dir == DIR_NONE) {
            fprintf(stderr, "Ошибка в строке %d: неизвестное направление\n", line_num);
            return 0;
        }

        int nx, ny;
        if (!get_neighbor_coords(field, dino->x, dino->y, dir, &nx, &ny)) return 0;

        Tile *t = &field->tiles[ny][nx];
        if (t->tile != '&') {
            fprintf(stderr, "Ошибка в строке %d: в указанной клетке нет дерева\n", line_num);
            return 0;
        }
        t->tile = t->color ? t->color : '_';
        return 1;
    }

    // MAKE
    if (strcmp(cmd_buf, "MAKE") == 0) {
        char dir_str[16];
        if (sscanf(line, "MAKE %15s", dir_str) != 1) {
            fprintf(stderr, "Ошибка в строке %d: неверный формат MAKE\n", line_num);
            return 0;
        }
        enum Direction dir = parse_direction(dir_str);
        if (dir == DIR_NONE) {
            fprintf(stderr, "Ошибка в строке %d: неизвестное направление\n", line_num);
            return 0;
        }

        int nx, ny;
        if (!get_neighbor_coords(field, dino->x, dino->y, dir, &nx, &ny)) return 0;

        Tile *t = &field->tiles[ny][nx];
        if (!is_placeable(*t)) {
            fprintf(stderr, "Ошибка в строке %d: нельзя создать камень на занятой клетке\n", line_num);
            return 0;
        }
        t->tile = '@';
        return 1;
    }

    // PUSH
    if (strcmp(cmd_buf, "PUSH") == 0) {
        char dir_str[16];
        if (sscanf(line, "PUSH %15s", dir_str) != 1) {
            fprintf(stderr, "Ошибка в строке %d: неверный формат PUSH\n", line_num);
            return 0;
        }
        enum Direction dir = parse_direction(dir_str);
        if (dir == DIR_NONE) {
            fprintf(stderr, "Ошибка в строке %d: неизвестное направление\n", line_num);
            return 0;
        }

        int nx, ny;
        if (!get_neighbor_coords(field, dino->x, dino->y, dir, &nx, &ny)) return 0;

        Tile *stone = &field->tiles[ny][nx];
        if (stone->tile != '@') {
            fprintf(stderr, "Ошибка в строке %d: в указанной клетке нет камня\n", line_num);
            return 0;
        }

        // Куда летит камень
        int tx = nx, ty = ny;
        switch (dir) {
            case DIR_UP:    ty--; break;
            case DIR_DOWN:  ty++; break;
            case DIR_LEFT:  tx--; break;
            case DIR_RIGHT: tx++; break;
        }
        tx = normalize_x(field, tx);
        ty = normalize_y(field, ty);

        Tile *target = &field->tiles[ty][tx];
        char target_tile = target->tile;

        if (target_tile == '^' || target_tile == '&') {
            printf("Предупреждение: камень отскочил в строке %d\n", line_num);
            return 1;
        }

        // Убираем камень
        stone->tile = stone->color ? stone->color : '_';

        if (target_tile == '%') {
            // Засыпаем яму
            target->tile = target->color ? target->color : '_';
        } else if (target_tile == '@') {
            fprintf(stderr, "Ошибка в строке %d: столкновение камней\n", line_num);
            return 0;
        } else if (target_tile == '#') {
            fprintf(stderr, "Ошибка в строке %d: камень попал в динозавра!\n", line_num);
            return 0;
        } else {
            // Ставим камень
            target->tile = '@';
        }
        return 1;
    }

    fprintf(stderr, "Ошибка в строке %d: неизвестная команда '%s'\n", line_num, cmd_buf);
    return 0;
}


