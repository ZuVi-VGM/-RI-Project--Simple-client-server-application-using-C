//
// Created by vitog on 01/07/2023.
//

#include "dishes.h"

//L'array dei piatti è organizzato come <-actr->|<-pctr->|<-dctr->, ossia in ordine vengono caricati primi, secondi e dolci
struct Dish* menu; //Array contenente i piatti del menù
int dishCount = 0, actr = 0, pctr = 0, sctr = 0, dctr = 0; //Contatori utili

//Inserisce un nuovo piatto nell'array aggiornando i contatori opportuni
void _new_dish(char type, int id, char* name, int price)
{
    menu[dishCount].type = type;
    menu[dishCount].id = id;
    strcpy(menu[dishCount].name, name);
    menu[dishCount].price = price;

    if(type == 'A')
        actr++;
    else if(type == 'P')
        pctr++;
    else if(type == 'S')
        sctr++;
    else
        dctr++;

    dishCount++;
}

//Aggiunge un nuovo piatto all'array
void add_dish(char type, int id, char* name, int price)
{
    if(menu == NULL)
        menu = malloc(sizeof(struct Dish));
    else
        menu = realloc(menu, (dishCount+1)*sizeof(struct Dish));

    _new_dish(type, id, name, price);
}

//Restituisce un piatto in base a tipo/id
struct Dish* _find_dish(char type, int id)
{
    int base;
    struct Dish* ret = NULL;

    if((id-1) >= 0) {
        switch (type) {
            case 'A':
                base = (id - 1 < actr) ? 0 : -1;
                break;
            case 'P':
                base = (id - 1 < pctr) ? actr : -1;
                break;
            case 'S':
                base = (id - 1 < sctr) ? actr + pctr : -1;
                break;
            case 'D':
                base = (id - 1 < dctr) ? actr + pctr + dctr : -1;
                break;
            default:
                printf("Piatto non valido!\n");
                return NULL;
        }
    } else{
        base = -1;
    }

    if(base != -1)
        ret = &menu[(base+(id-1))];

    return ret;
}

//Stampa il menù
void print_menu()
{
    int i;

    printf("Stampo il menù:\n");
    fflush(stdout);

    for(i = 0; i < dishCount; i++)
    {
        printf("%c%d - %s\t%d\n", menu[i].type, menu[i].id, menu[i].name, menu[i].price);
        fflush(stdout);
    }
}

//Restituisce il numero di piatti
int get_dish_count(){
    return dishCount;
}

//Restituisce il piatto all'i-esima posizione dell'array
void get_dish(int i, struct Dish* dish){
    if(i >= dishCount)
        return;

    dish->type = menu[i].type;
    dish->id = menu[i].id;
    strcpy(dish->name, menu[i].name);
    dish->price = menu[i].price;
}

//Verifica l'esistenza del piatto "type-id"
int check_dish(char type, int id)
{
    int ret;
    switch(type)
    {
        case 'A':
            ret = (id > 0 && id <= actr) ? 0 : 1;
            break;
        case 'P':
            ret = (id > 0 && id <= pctr) ? 0 : 1;
            break;
        case 'S':
            ret = (id > 0 && id <= sctr) ? 0 : 1;
            break;
        case 'D':
            ret = (id > 0 && id <= dctr) ? 0 : 1;
            break;
        default:
            printf("Tipo passato non valido!\n");
            return 1;
    }

    return ret;
}

//Restituisce il prezzo del piatto "type-id"
int get_price(char type, int id)
{
    struct Dish* dish;

    dish = _find_dish(type, id);
    if(dish == NULL)
        return -1;

    return dish->price;
}

//Svuota il menù
void delete_menu()
{
    free(menu);
    printf("Menù svuotato!\n");
    fflush(stdout);
}