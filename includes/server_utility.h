//
// Created by vitog on 28/06/2023.
//

#ifndef PROJECT_SERVER_UTILITY_H
#define PROJECT_SERVER_UTILITY_H

void get_tables();
void get_dishes();
void get_reservations();
int authentication_handler(int, char*);
void handle_disconnection(int);
int find_handler(int, char*);
int book_handler(int, char*);
int menu_handler(int, char*);
int com_handler(int, char*);
int conto_handler(int, char*);
void add_kd(int);
void kd_disconnect(int);
int take_handler(int, char*);
int show_handler(int, char*);
int ready_handler(int, char*);
void stat_handler(char* buff);
int stop_handler();
void _notify_all();
void _save_reservation();

#endif //PROJECT_SERVER_UTILITY_H
