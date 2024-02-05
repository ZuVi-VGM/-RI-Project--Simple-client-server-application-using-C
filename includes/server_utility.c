//
// Created by vitog on 28/06/2023.
//
#include "header.h"
#include "tables.h"
#include "dishes.h"
#include "utility.h"
#include "server_utility.h"

int* kdList; //Array dei kitchen device collegati
int kdListSize = 0; //Dimensione dell'array
int kdCount = 0; //Numero di KD connessi

//Recupero la lista dei tavoli dall'apposito file
//e li inserisco all'interno di una lista
//gestista nel file "includes/tables.c"
void get_tables()
{
    FILE *fp;
    char buff[100];
    int id, sala, posti;
    char position[STR_SIZE];
    int n = 0;

    fp = fopen("data/tables.txt", "r");
    if(fp == NULL){
        perror("Errore, impossibile recuperare i tavoli!\n");
        exit(1);
    }

    while(fgets(buff, 100, fp) != NULL){
        sscanf(buff, "T%d SALA%d %s %d", &id, &sala, position, &posti);
        add_table(id, sala, position, posti); //includes/tables.c
        n++;
    }

    fclose(fp);
    printf("%d tavoli caricati con successo!\n", n);
    fflush(stdout);
    print_table_list(); //funzione di debug, utile per verificare che la lista sia stata inizializzata con successo
}

//Simile alla get_tables() ma recupera
//i piatti del menù
void get_dishes()
{
    FILE *fp;
    char buff[100];
    char type;
    char piatto[50];
    int id, price;
    int n = 0;

    fp = fopen("data/menu.txt", "r");
    if(fp == NULL){
        perror("Errore, impossibile recuperare il menù!\n");
        exit(1);
    }

    while(fgets(buff, sizeof(buff), fp) != NULL){
        //printf("%s", buff);
        sscanf(buff, "%c%d - %[^-] - %d", &type, &id, piatto, &price);
        add_dish(type, id, piatto, price); //includes/tables.c
        n++;
    }

    fclose(fp);
    printf("%d piatti caricati con successo!\n", n);
    fflush(stdout);
    print_menu(); //funzione di debug, utile per verificare che la lista sia stata inizializzata con successo
}

//Recupera le prenotazioni
void get_reservations()
{
    FILE *fp;
    char buff[100];
    int tavolo, posti;
    time_t rawtime;
    unsigned int rtime;
    char nome[STR_SIZE];
    int n = 0;

    fp = fopen("data/reservations.txt", "r");
    if(fp == NULL){
        perror("Errore, impossibile recuperare i tavoli!\n");
        exit(1);
    }

    while(fgets(buff, sizeof(buff), fp) != NULL){
        sscanf(buff, "T%d %s %d %u", &tavolo, nome, &posti, &rtime);
        //printf("Recuperato: T%d %s %d %u\n", tavolo, nome, posti, rtime);
        rawtime = rtime;
        if(add_reservation(tavolo, nome, posti, rawtime) == 0) //Scarta le prenotazioni con tavoli errati (includes/tables.c)
            n++;
    }

    fclose(fp);
    printf("%d prenotazioni caricate con successo!\n", n);
    fflush(stdout);
    print_reservation_list();
}

//Gestisce l'autenticazione dei dispositivi,
//in base al tipo conserva l'identificatore
//del socket in un'apposita struttura dati
int authentication_handler(int sd, char* buff)
{
    int ret;
    ret = recv_msg(sd, buff);
    if(ret < 0)
        return ret;

    if(strcmp(buff, "cl") == 0) {
        ret = send_msg(sd, "OK\0");
        printf("Nuovo dispositivo client connesso!\n");
        fflush(stdout);

        return ret;
    } else if(strcmp(buff, "td") == 0){
        ret = new_t_dev(sd);
        if(ret < 0)
            return ret;

        printf("Nuovo dispositivo td connesso! Tavolo assegnato: %d\n", ret);
        fflush(stdout);
        sprintf(buff, "OK_%d", ret); //Comunico al td il tavolo assegnato
        ret = send_msg(sd, buff);
        return ret;
    } else if(strcmp(buff, "kd") == 0){
        add_kd(sd); //Non ho particolari vincoli sui kd

        printf("Nuovo dispositivo kd connesso!\n");
        fflush(stdout);
        ret = send_msg(sd, "OK\0");
        return ret;
    } else {
        return 500; //codice di errore per dispositivo non valido
    }
}

//Gestisce la disconnessione di td e kd
void handle_disconnection(int sd)
{
    td_disconnect(sd); //includes/table.c
    kd_disconnect(sd); //definita più avanti
}

//Gestisce l'esecuzione del comando find
int find_handler(int sd, char* buff)
{
    int ret;
    int people;
    char name[STR_SIZE];
    int i = 0;
    unsigned int rt;
    time_t rawtime;
    struct Table* tList;
    struct Table* temp;

    //Devo estrarre il timestamp, ottenere i tavoli disponibili e mandare i risultati
    sscanf(buff, "find %s %d %u", name, &people, &rt);
    rawtime = rt;
    tList = get_free_tables(people, rawtime);

    temp = tList;
    while(temp != NULL){
        sprintf(buff, "T%d SALA%d %s", temp->id, temp->sala, temp->position);
        i++;
        send_msg(sd, buff);
        temp = temp->next;
    }

    ret = send_msg(sd, "END\0"); //Comunico che ho inviato l'intera lista

    //distruggo la lista creata
    while(tList != NULL)
    {
        temp = tList;
        tList = tList->next;
        free(temp);
    }

    return ret;
}

//Gestisce l'esecuzione del comando book
int book_handler(int sd, char* buff)
{
    int ret;
    int people, table;
    char name[STR_SIZE];
    unsigned int rt;
    time_t rawtime;
    struct Table* tList;
    struct Table* temp;

    sscanf(buff, "book %d %s %d %u", &table, name, &people, &rt);
    rawtime = rt;
    tList = get_free_tables(people, rawtime);

    //Controllo se il tavolo è ancora disponibile
    //Un altro td potrebbe aver riservato il tavolo per lo stesso orario
    temp = tList;
    while(temp != NULL && temp->id != table)
        temp = temp->next;

    if(temp == NULL){
        ret = send_msg(sd, "Il tavolo risulta occupato!\n");
        return ret;
    }

    printf("Inserisco la prenotazione\n");

    ret = add_new_reservation(temp->id, name, people, rawtime);
    if(ret == 0)
        ret = send_msg(sd, "PRENOTAZIONE INSERITA\0");
    else
        ret = send_msg(sd, "ERRORE\0");

    //Stampo la lista delle prenotazioni aggiornata
    //Utile per il debug
    print_reservation_list();
    return ret;
}

//Gestisce l'esecuzione del comando menù
int menu_handler(int sd, char* buff)
{
    int i, dishCount, ret;
    struct Dish dish;

    dishCount = get_dish_count();

    for(i = 0; i < dishCount; i++)
    {
        get_dish(i, &dish);

        sprintf(buff, "%c%d %s %d", dish.type, dish.id, dish.name, dish.price);
        send_msg(sd, buff);
    }

    ret = send_msg(sd, "END\0");
    return ret;
}

//Gestisce l'esecuzione del comando comanda
int com_handler(int sd, char* buff)
{
    int table, ret;
    char orders[STR_SIZE];
    char* token = NULL;
    char type;
    int id, num;
    struct Plate* plates = NULL;

    //ottengo i vari piatti da inserire ed il numero del tavolo
    sscanf(buff, "comanda %d %[^\n]", &table, orders);

    //Questo controllo potrebbe essere considerato ridondante
    //ma la presenza di un id errato potrebbe
    //portare ad un segmentation fault.
    //La funzione _find_table effettua in automatico
    //il controllo sull'id del tavolo.
    if(_find_table(table) == NULL){
        ret = send_msg(sd, "ERRORE\0");
        return ret;
    }

    //controllo i piatti e creo una lista di piatti
    //tokenizzo la stringa
    token = strtok(orders, " ");
    while(token != NULL){
        if(sscanf(token, "%c%d-%d", &type, &id, &num) == 3)
            if(check_dish(type, id) == 0) //Se ci sono piatti errati li scarto
                add_new_plate(&plates, type, id, num); //include/table.c

        token = strtok(NULL, " ");
    }

    if(plates == NULL){ //Non c'erano piatti validi
        ret = send_msg(sd, "ERRORE\0");
        return ret;
    }

    //Ora che ho la lista di piatti pronta, posso creare l'ordine e associarla
    ret = add_new_order(table, plates); //includes/table.c
    if(ret != 0){
        ret = send_msg(sd, "ERRORE\0");
        return ret;
    }

    ret = send_msg(sd, "COMANDA RICEVUTA\0");
    print_order_list();
    return ret;
}

//Gestisce il comando conto
int conto_handler(int sd, char* buff)
{
    int table, ret;
    int conto = 0, price;
    struct Table* t;
    struct Order* tmp;
    struct Plate* tmp1;

    sscanf(buff, "conto %d", &table);

    t = _find_table(table);
    if(t == NULL){
        ret = send_msg(sd, "ERRORE\0");
        return ret;
    }

    tmp = t->orders;
    while(tmp != NULL){
        tmp1 = tmp->plates;
        while(tmp1 != NULL){
            price = tmp1->num*get_price(tmp1->type, tmp1->id);
            sprintf(buff, "%c%d %d %d", tmp1->type, tmp1->id, tmp1->num, price);
            send_msg(sd, buff);
            conto += price;
            tmp1 = tmp1->next;
        }
        tmp = tmp->next;
    }

    sprintf(buff, "Totale: %d", conto);
    send_msg(sd, buff);
    ret = send_msg(sd, "END\0");

    //Svuoto le liste degli ordini relative al tavolo table
    reset_order_list(t->id); //includes/table.c
    print_order_list(); //includes/table.c

    return ret;
}

//Salva l'identificatore del socket relativo
//ad un kd nell'apposita struttura dati
void add_kd(int sd)
{
    int i;

    if(kdList == NULL) {
        kdList = malloc(sizeof(int));
        kdList[0] = sd;
        kdCount++;
        kdListSize++;
        return;
    } else {
        //Controllo se ho spazio nell'array
        for (i = 0; i < kdListSize; i++)
            if (kdList[i] == -1) {
                kdList[i] = sd;
                kdCount++;
                break;
            }
        if (i != kdListSize)
            return;

        //Aggiungo alla fine
        kdList = realloc(kdList, (kdListSize + 1) * sizeof(int));
        kdList[kdListSize++] = sd;
        kdCount++;
    }
}

//Gestisco la disconnessione di un kd
void kd_disconnect(int sd){
    int i;

    for(i = 0; i < kdListSize; i++)
        if(kdList[i] == sd){
            kdList[i] = -1;
            kdCount--;
            break;
        }
}

//Gestisco il comando take
int take_handler(int sd, char* buff)
{
    int ret;

    struct Order* order;
    struct Plate* tmp;

    //Recupero il primo ordine e lo sposto nel tavolo
    order = take_order(sd);
    if(order == NULL)
    {
        send_msg(sd, "Non ci sono comande in attesa!\0");
        ret = send_msg(sd, "END\0");

        return ret;
    }

    //invio i dati della comanda ed i piatti
    sprintf(buff, "com%d T%d", order->id, order->table);
    send_msg(sd, buff);

    tmp = order->plates;
    while(tmp != NULL){
        sprintf(buff, "%c%d %d", tmp->type, tmp->id, tmp->num);
        send_msg(sd, buff);
        tmp = tmp->next;
    }

    ret = send_msg(sd, "END\0");
    print_order_list();
    return ret;
}

//Gestisco il comando show
int show_handler(int sd, char* buff)
{
    int ret;

    struct Order* orders, *tmp;
    struct Plate* tmp1;

    //Recupero la lista di ordini in preparazione del kd
    orders = get_orders(sd);
    if(orders == NULL)
    {
        send_msg(sd, "Non ci sono comande in preparazione!\0");
        ret = send_msg(sd, "END\0");

        return ret;
    }

    //Invio gli ordini ed i piatti
    tmp = orders;

    while(tmp != NULL){
        //invio i dati della comanda ed i piatti
        sprintf(buff, "com%d T%d", tmp->id, tmp->table);
        send_msg(sd, buff);

        tmp1 = tmp->plates;
        while(tmp1 != NULL){
            sprintf(buff, "%c%d %d", tmp1->type, tmp1->id, tmp1->num);
            send_msg(sd, buff);
            tmp1 = tmp1->next;
        }

        tmp = tmp->next;
    }

    ret = send_msg(sd, "END\0");

    //Svuoto la lista -> non serve eliminare i piatti (il puntatore è quello della lista originale)
    while(orders != NULL){
        tmp = orders;
        orders = orders->next;
        free(tmp);
    }

    print_order_list();
    return ret;
}

//Gestisco il comando ready
int ready_handler(int sd, char* buff)
{
    int ret, com, table;

    sscanf(buff, "ready %d %d", &com, &table);

    //Controllo se il tavolo esiste e se la comanda esiste la setto
    ret = order_ready(sd, com, table);
    if(ret == 0)
        ret =send_msg(sd, "COMANDA IN SERVIZIO\0");
    else
        ret = send_msg(sd, "ERRORE\0");

    print_order_list();
    return ret;
}

//Gestisco il comando stat
void stat_handler(char* buff)
{
    int table, ret;
    char type;

    ret = sscanf(buff, "stat %c%d",  &type, &table);

    if(ret == 1) {
        //check sul tipo
        if(type != 'a' && type != 'p' && type != 's'){
            printf("Tipo non valido!\nstat a|p|s\n");
            fflush(stdout);
            return;
        }

        print_orders(type);
    } else if(ret == 2) {
        print_table_orders(table);
    } else {
        print_order_list();
    }
}

//Gestisco il comando stop
int stop_handler(){
    //CONTROLLO SE NON HO COMANDE IN ATTESA O IN PREPARAZIONE
    if(check_commands() != 0){
        printf("Ci sono ancora delle comande in attesa o in preparazione!\n");
        fflush(stdout);
        return 1;
    }
    //NOTIFICO
    _notify_all();
    //SALVO LE PRENOTAZIONI
    _save_reservation();
    //DISTRUGGO STRUTTURE DATI INIZIALIZZATE:
    delete_menu();//MENU includes/dishes.c
    delete_reservations();//PRENOTAZIONI includes/table.c
    delete_orders();//ORDINI E PIATTI includes/table.c
    delete_tables();//TAVOLI includes/table.c
    printf("Bye!\n");
    fflush(stdout);
    return 0;
}

//Notifico i td ed i kd della chiusura
void _notify_all(){
    int i;
    struct Table* t;
    int table_count = get_table_count();

    for(i = 0; i < table_count; i++)
    {
        t = _find_table(i+1);
        if(t->td != -1)
        {
            send_msg(t->td, "CLOSE\0");
            close(t->td);
        }
    }

    for(i = 0; i < kdListSize; i++)
    {
        if(kdList[i] != -1)
        {
            send_msg(kdList[i], "CLOSE\0");
            close(kdList[i]);
        }
    }

    printf("Notifiche inviate!\n");
    fflush(stdout);
}

//Salvo le prenotazioni
void _save_reservation(){
    int i, table_count = get_table_count();
    struct Table* t;
    struct Reservation* tmp;

    FILE *fp;

    remove("data/reservations.txt");

    fp = fopen("data/reservations.txt", "w");
    if(fp == NULL){
        perror("Errore nell'apertura in scrittura file Prenotazioni.txt.\n");
        fflush(stdout);
        exit(1);
    }

    for(i = 0; i < table_count; i++){
        t = _find_table(i+1);
        if(t->resList != NULL){
            tmp = t->resList;
            while(tmp != NULL)
            {
                if(difftime(tmp->time, time(NULL)) > 0)
                    fprintf(fp, "T%d %s %d %u\n", tmp->table, tmp->name, tmp->seat, (unsigned)tmp->time);

                tmp = tmp->next;
            }
        }
    }

    fclose(fp);
    printf("Prenotazioni salvate!\n");
    fflush(stdout);
}