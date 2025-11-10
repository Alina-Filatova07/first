// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "field.h"
#include "dino.h"
#include "parser.h"
#include "utils.h"

// Глобальные флаги
int no_display = 0;
int no_save = 0;
int interval = 1;

void parse_options(int argc, char *argv[], const char **input_file, const char **output_file) {
    *input_file = NULL;
    *output_file = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "no-display") == 0) {
            no_display = 1;
        } else if (strcmp(argv[i], "no-save") == 0) {
            no_save = 1;
        } else if (strcmp(argv[i], "interval") == 0 && i + 1 < argc) {
            interval = atoi(argv[++i]);
            if (interval < 0) interval = 1;
        } else if (!*input_file) {
            *input_file = argv[i];
        } else if (!*output_file) {
            *output_file = argv[i];
        }
    }
}
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Использование: %s input.txt output.txt [опции]\n", argv[0]);
        return 1;
    }

    const char *input_filename, *output_filename;
    parse_options(argc, argv, &input_filename, &output_filename);

    if (!input_filename || !output_filename) {
        fprintf(stderr, "Ошибка: не указаны имена файлов\n");
        return 1;
    }

    FILE *input_file = fopen(input_filename, "r");
    if (!input_file) {
        perror("Не удалось открыть входной файл");
        return 1;
    }

    Field field = {0};
    Dino dino = {0};
    char line[1024];
    int line_num = 0;

    // Первая строка должна быть SIZE или LOAD
    if (!fgets(line, sizeof(line), input_file)) {
        fprintf(stderr, "Файл пуст\n");
        fclose(input_file);
        return 1;
    }
    line_num++;

    // Удаляем символ новой строки
    line[strcspn(line, "\n")] = 0;

    if (strncmp(line, "//", 2) == 0) {
        fprintf(stderr, "Ошибка: первая строка не может быть комментарием\n");
        fclose(input_file);
        return 1;
    }

    if (strncmp(line, "SIZE", 4) != 0 && strncmp(line, "LOAD", 4) != 0) {
        fprintf(stderr, "Ошибка: первая команда должна быть SIZE или LOAD\n");
        fclose(input_file);
        return 1;
    }

    if (!execute_line(line, &field, &dino, line_num)) {
        fclose(input_file);
        return 1;
    }

    if (!no_display) {
        clear_screen();
        print_field(&field);
        sleep_seconds(interval);
    }

    // Основной цикл
    while (fgets(line, sizeof(line), input_file)) {
        line_num++;
        line[strcspn(line, "\n")] = 0; // убрать \n

        if (!execute_line(line, &field, &dino, line_num)) {
            fprintf(stderr, "Выполнение прервано на строке %d\n", line_num);
            free_field(&field);
            fclose(input_file);
            return 1;
        }

        if (!no_display) {
            clear_screen();
            print_field(&field);
            sleep_seconds(interval);
        }
    }

    fclose(input_file);

    printf("Сохраняю результат в '%s'.\n", output_filename); 

    if (!no_save) {
        FILE *out = fopen(output_filename, "w");
        if (out) {
            save_field_to_file(&field, out);
            fclose(out);
            printf("Файл '%s' успешно записан.\n", output_filename);
        } else {
            perror("Не удалось сохранить выходной файл");
        }
    }

    free_field(&field);
    return 0;
}



