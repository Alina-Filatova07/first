// command.h
#ifndef COMMAND_H
#define COMMAND_H

enum CommandType {
    CMD_NONE,
    CMD_SIZE, //размер
    CMD_START,
    CMD_MOVE, //направление
    CMD_PAINT, //закрасить
    CMD_DIG, //создать яму %
    CMD_MOUND, //создать гору ^
    CMD_JUMP, 
    CMD_GROW, //создать дерево &
    CMD_CUT, //удалить дерево &
    CMD_MAKE, //создать камень @
    CMD_PUSH, //подвинуть камень
    CMD_EXEC, //загрузить и выполнить команды из файла
    CMD_LOAD, //загрузить состояние поля
    CMD_UNDO, //отменить последние действие
    CMD_IF, //проверяет клетку на адекватность
    CMD_COMMENT
};

enum Direction {
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

#endif


