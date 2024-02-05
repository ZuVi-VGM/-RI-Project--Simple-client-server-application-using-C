//
// Created by vitog on 01/07/2023.
//

#ifndef PROJECT_DISHES_H
#define PROJECT_DISHES_H

#include "header.h"

struct Dish{
    char type;
    int id;
    char name[STR_SIZE];
    int price;
    //struct Dish* next;
};

void _new_dish(char, int, char*, int);
void add_dish(char, int, char*, int);
struct Dish* _find_dish(char, int);
void print_menu();
int get_dish_count();
void get_dish(int, struct Dish*);
int check_dish(char, int);
int get_price(char, int);
void delete_menu();

#endif //PROJECT_DISHES_H
