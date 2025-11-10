// parser.h

#ifndef PARSER_H
#define PARSER_H
#include "field.h"
#include "dino.h"
#include "command.h"

//Основная функция-интерпретатор команд
int execute_line(const char *line, Field *field, Dino *dino, int line_num);

#endif
