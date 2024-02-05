//
// Created by vitog on 27/06/2023.
//

#ifndef PROJECT_TABLES_H
#define PROJECT_TABLES_H

#include "header.h"

struct Reservation{
    int table; //E' ridondante ma può tornare utile in fase di stampa/recupero/salvataggio dati
    char name[STR_SIZE];
    int seat;
    time_t time;
    struct Reservation* next;
};

struct Plate{
    char type;
    int id;
    int num;
    struct Plate* next;
};

struct Order{
    int table;
    int kd;
    int id;
    enum Status {a, p, s} state;
    //lista dei piatti con quantità
    struct Plate* plates;
    struct Order* next;
};

struct Table{
    int id;
    int sala;
    char position[STR_SIZE];
    int seat;
    int td;
    int order_count;
    struct Reservation* resList;
    struct Order* orders;
    struct Table* next; //Utile per restituire i tavoli disponibili in fase di prenotazione
};

void _new_table(int, int, char*, int);
struct Table* _find_table(int);
void add_table(int, int, char*, int);
void print_table_list();
struct Reservation* _new_reservation(int, char*, int, time_t);
int add_reservation(int, char*, int, time_t);
void print_reservation_list();
struct Table* _new_table_struct(int);
void _update_table_list(struct Table**, int);
struct Table* get_free_tables(int, time_t);
int add_new_reservation(int, char*, int, time_t);
int new_t_dev(int);
void td_disconnect(int);
struct Plate* _new_plate(char, int, int);
struct Order* _new_order(int, int, struct Plate*);
void add_new_plate(struct Plate**, char, int, int);
int add_new_order(int, struct Plate*);
void print_order_list();
void print_orders(char);
void print_table_orders(int);
void _table_order_add(struct Order*, int, int);
struct Order* take_order(int);
void _update_order_list(struct Order**, struct Order*);
struct Order* get_orders(int);
int order_ready(int, int, int);
void reset_order_list(int);
int check_commands();
int get_table_count();
void delete_reservations();
void delete_orders();
void delete_tables();


#endif //PROJECT_TABLES_H
