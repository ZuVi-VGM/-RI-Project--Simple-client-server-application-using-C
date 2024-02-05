//
// Created by vitog on 28/06/2023.
//
#include "header.h"
#include "tables.h"

struct Table* tableList; //Array dei tavoli
struct Order* orderList; //Lista delle comande in attesa
struct Order* old; //Lista archivio delle comande giornaliere (la uso nella stat)
int tableCount = 0; //Numero dei tavoli
int tDev = 0; //Numero dei TD connessi

//Inizializza la struttura tableList[tableCount]
void _new_table(int id, int sala, char* posizione, int posti)
{
    tableList[tableCount].id = id;
    tableList[tableCount].sala = sala;
    strcpy(tableList[tableCount].position, posizione);
    tableList[tableCount].seat = posti;
    tableList[tableCount].td = 0;
    tableList[tableCount].resList = NULL;
    tableList[tableCount].next = NULL;

    tableCount++;
}

//Restituisce un puntatore alla struttura relativa al tavolo id
//Si presuppone che i tavoli abbiano tutti ID crescente a partire da 1
struct Table* _find_table(int id)
{
    struct Table* tmp = NULL;
    if((id-1) >= 0 && (id-1) < tableCount)
        tmp = &tableList[id-1];

    return tmp;
}

//inserisce un nuovo tavolo all'interno della lista tableList
void add_table(int id, int sala, char* posizione, int posti)
{
    //Inizializzo l'array dei tavoli o lo ridimensiono, per poi inserire i dati del tavolo
    if(tableList == NULL)
        tableList = malloc(sizeof(struct Table));
    else
        tableList = realloc(tableList, (tableCount+1)*sizeof(struct Table));

    _new_table(id, sala, posizione, posti);
}

//funzione di utilità, stampa la lista dei tavoli
void print_table_list()
{
    int i;
    printf("Stampo la lista dei tavoli:\n");
    fflush(stdout);

    if(tableList == NULL)
    {
        printf("Non ci sono tavoli!\n");
        return;
    } else {
        for(i = 0; i < tableCount; i++)
        {
            printf("T%d SALA%d %s %d\n", tableList[i].id, tableList[i].sala, tableList[i].position, tableList[i].seat);
            fflush(stdout);
        }
    }
}

//Alloca una nuova prenotazione e la inizializza
struct Reservation* _new_reservation(int tavolo, char* nome, int posti, time_t rawtime)
{
    struct Reservation* ret = malloc(sizeof(struct Reservation));

    ret->table = tavolo;
    strcpy(ret->name, nome);
    ret->seat = posti;
    ret->time = rawtime;
    ret->next = NULL;

    return ret;
}

//Inserisce la nuova prenotazione nella coda adatta (relativa al tavolo)
int add_reservation(int tavolo, char* nome, int posti, time_t rawtime)
{
    //Verifico che il tavolo sia esistente
    struct Table* table = _find_table(tavolo);

    if(table == NULL){
        //printf("Tavolo non valido! Prenotazione scartata! \n"); //debug
        //fflush(stdout);
        return 1;
    }

    struct Reservation* ret = _new_reservation(tavolo, nome, posti, rawtime);

    if(table->resList == NULL) {
        table->resList = ret; //inserisco in testa
        //printf("Prenotazione aggiunta in testa al tavolo %d\n", table->id);
        //printf("%d %s %d\n", ret->table, ret->name, ret->seat);
        //fflush(stdout);
    } else {
        //Assumendo che la lista è salvata ordinata, aggiungo in coda
        struct Reservation* tmp = table->resList;
        while(tmp->next != NULL) //mi fermo sull'ultimo elemento della lista
            tmp = tmp->next;

        tmp->next = ret;

        //printf("Prenotazione aggiunta in coda al tavolo %d\n", table->id); //debug
        //printf("%d %s %d\n", ret->table, ret->name, ret->seat);
        //fflush(stdout);
    }

    return 0;
}

//Stampa la lista delle prenotazioni
void print_reservation_list()
{
    int i;
    printf("Stampo la lista delle prenotazioni:\n");
    fflush(stdout);

    struct tm* ltime;

    for(i = 0; i < tableCount; i++) {
        struct Reservation* tmp = tableList[i].resList;
        if(tmp == NULL){
            printf("T%d Non ci sono prenotazioni!\n", tableList[i].id);
            fflush(stdout);
            //tmp = tmp->next; //tavolo successivo
            continue;
        }

        while(tmp != NULL) {
            ltime = localtime(&(tmp->time));
            printf("T%d %s %d %d-%d-%d %d\n", tmp->table, tmp->name, tmp->seat, ltime->tm_mday, ltime->tm_mon+1, ltime->tm_year+1900, ltime->tm_hour);
            fflush(stdout);
            tmp = tmp->next; //prenotazione successiva
        }

    }
}

//Ritorna il puntatore ad una struttura Table
struct Table* _new_table_struct(int i)
{
    struct Table* new = malloc(sizeof(struct Table));
    new->id = tableList[i].id;
    strcpy(new->position, tableList[i].position);
    new->sala = tableList[i].sala;
    new->order_count = 0;
    new->resList = NULL;
    new->seat = tableList[i].seat;
    new->orders = NULL;
    new->next = NULL;

    return new;
}

//Aggiorna una lista di tavoli
void _update_table_list(struct Table** list, int i)
{
    if(*list == NULL)
    {
        *list = _new_table_struct(i);
    } else {
        struct Table* tmp = *list;
        while(tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = _new_table_struct(i);
    }
}

//Crea una lista di tavoli liberi per una determinata data
struct Table* get_free_tables(int people, time_t rawtime)
{
    int i;
    struct Table* ret = NULL;
    time_t now;
    time(&now);

    //Controllo che la data sia futura
    if(difftime(rawtime, now) < 0)
    {
        printf("Data non valida.");
        fflush(stdout);
        return ret;
    }

    for(i = 0; i < tableCount; i++)
    {
        if(tableList[i].seat < people)
            continue;

        if(tableList[i].resList == NULL)
        {
            _update_table_list(&ret, i);
        } else {
            struct Reservation* tmp = tableList[i].resList;
            while(tmp != NULL && (difftime(rawtime, tmp->time) > 0))
                tmp = tmp->next;

            //se ho percorso tutta la lista o mi sono fermato perché ho superato la data richiesta, il tavolo è libero
            if(tmp == NULL || (difftime(rawtime,tmp->time) < 0))
                _update_table_list(&ret, i);
        }
    }

    return ret;
}

//Aggiunge una nuova prenotazione
int add_new_reservation(int table, char* name, int seat, time_t rawtime)
{
    struct Table* tab = _find_table(table);

    if(tab == NULL){
        printf("Tavolo non valido! Prenotazione scartata! \n");
        fflush(stdout);
        return 1;
    }

    struct Reservation* ret = _new_reservation(table, name, seat, rawtime);

    if(tab->resList == NULL) {
        tab->resList = ret; //inserisco in testa
    } else {
        //Aggiungo in ordine
        struct Reservation* tmp = tab->resList;
        struct Reservation* tmp1 = NULL;
        while(tmp != NULL && difftime(rawtime, tmp->time) > 0) //mi fermo sull'ultimo elemento della lista o quando ho superato la data richiesta
        {
            tmp1 = tmp;
            tmp = tmp->next;
        }

        //se tmp è NULL sto inserendo in coda, altrimenti sto inserendo in testa o nel mezzo
        if(tmp == tab->resList){
            //Sono fermo sulla testa, va inserito effettivamente in testa (significa che difftime <0)
            tab->resList = ret;
            ret->next = tmp;
        } else {
            //inserisco in coda
            tmp1->next = ret;
            ret->next = tmp;
        }
    }

    return 0;
}

//Aggiunge un nuovo TD
int new_t_dev(int sd)
{
    int i;
    if(tDev == tableCount)
        return -1;

    for(i = 0; i < tableCount; i++)
        if(tableList[i].td == 0)
        {
            tableList[i].td = sd;
            tDev++;
            break;
        }

    return tableList[i].id;
}

//Gestisce la disconnessione di un TD
void td_disconnect(int sd)
{
    int i;

    for(i = 0; i < tableCount; i++)
        if(tableList[i].td == sd)
        {
            tableList[i].td = 0;
            tDev--;
            break;
        }
}

//Crea un nuovo piatto (utile per gli ordini)
struct Plate* _new_plate(char type, int id, int num)
{
    struct Plate* ret = malloc(sizeof(struct Plate));
    ret->type = type;
    ret->id = id;
    ret->num = num;
    ret->next = NULL;

    return ret;
}

//Crea un nuovo ordine
struct Order* _new_order(int table, int id, struct Plate* plates)
{
    struct Order* ret = malloc(sizeof(struct Order));
    ret->table = table;
    ret->kd = -1;
    ret->id = id;
    ret->state = a;
    ret->plates = plates;
    ret->next = NULL;

    return ret;
}

//Aggiunge un piatto alla lista plates
void add_new_plate(struct Plate** plates, char type, int id, int price)
{
    struct Plate* ret = _new_plate(type, id, price);

    if(*plates == NULL) {
        *plates = ret;
    } else {
        struct Plate* temp = *plates;
        while(temp->next != NULL)
            temp = temp->next;

        temp->next = ret;
    }
}

//Aggiunge un nuovo ordine dal tavolo "table" nella lista delle comande in attesa
int add_new_order(int table, struct Plate* plates)
{
    struct Table* tab = _find_table(table);

    if(tab == NULL){
        printf("Tavolo non valido!\n");
        fflush(stdout);
        return 1;
    }

    struct Order* new = _new_order(table, tab->order_count+1, plates);

    if(orderList == NULL){
        orderList = new;
    } else {
        struct Order* temp = orderList;
        while(temp->next != NULL)
            temp = temp->next;

        temp->next = new;
    }

    tab->order_count++;
    return 0;
}

//Stampa la lista degli ordini
void print_order_list()
{
    struct Order* tmp = orderList, *oldtmp;
    struct Plate* tmp1;
    int i, j = 0;

    printf("Stampo la lista delle comande: \n");


    printf("Comande in attesa:\n");

    if(tmp == NULL)
        printf("-\n");

    while(tmp != NULL){
        printf("T%d com%d\n", tmp->table, tmp->id);
        tmp1 = tmp->plates;
        while(tmp1 != NULL)
        {
            printf("%c%d %d\n", tmp1->type, tmp1->id, tmp1->num);
            tmp1 = tmp1->next;
        }
        tmp = tmp->next;
    }

    for(i = 0; i < tableCount; i++)
    {
        if(tableList[i].orders != NULL)
        {
            j++;
            printf("T%d\n", tableList[i].id);
            tmp = tableList[i].orders;
            while(tmp != NULL){
                printf("com%d ", tmp->id);
                if(tmp->state == p)
                    printf("<in preparazione>\n");
                else
                    printf("<in servizio>\n");

                tmp1 = tmp->plates;
                while(tmp1 != NULL){
                    printf("%c%d %d\n", tmp1->type, tmp1->id, tmp1->num);
                    tmp1 = tmp1->next;
                }
                tmp = tmp->next;
            }
        }
    }

    if(j == 0){
        printf("Non ci sono comande in servizio o in preparazione!\n");
        fflush(stdout);
    }

    if(old == NULL)
        return;

    j = 0;
    printf("Storico giornata:\n");
    oldtmp = old;

    while(oldtmp != NULL)
    {
        printf("T%d com%d\n", oldtmp->table, oldtmp->id);
        fflush(stdout);
        tmp1 = oldtmp->plates;
        while(tmp1 != NULL){
            printf("%c%d %d\n", tmp1->type, tmp1->id, tmp1->num);
            fflush(stdout);
            tmp1 = tmp1->next;
        }
        oldtmp = oldtmp->next;
    }
}

//Stampa la lista degli ordini nello stato "type"
void print_orders(char type)
{
    struct Order* tmp = orderList;
    struct Plate* tmp1;
    int state, i, j = 0;

    if(type == 'a')
    {
        if(tmp == NULL){
            printf("Non ci sono comande in attesa!\n");
            fflush(stdout);
            return;
        }

        while(tmp != NULL){
            printf("com%d T%d\n", tmp->id, tmp->table);
            tmp1 = tmp->plates;
            while(tmp1 != NULL){
                printf("%c%d %d\n", tmp1->type, tmp1->id, tmp1->num);
                tmp1 = tmp1->next;
            }
            tmp = tmp->next;
        }
    } else if(type == 'p' || type == 's'){
        state = (type == 'p') ? 1 : 2;

        for(i = 0; i < tableCount; i++)
        {
            if(tableList[i].orders != NULL){
                tmp = tableList[i].orders;
                while(tmp != NULL)
                {
                    if(tmp->state == state){
                        j++;
                        printf("com%d T%d\n", tmp->id, tmp->table);
                        tmp1 = tmp->plates;
                        while(tmp1 != NULL)
                        {
                            printf("%c%d %d\n", tmp1->type, tmp1->id, tmp1->num);
                            tmp1 = tmp1->next;
                        }
                    }
                    tmp = tmp->next;
                }
            }
        }

        if(j == 0){
            printf("Non ci sono comande in ");
            if(state == 1)
                printf("preparazione!\n");
            else
                printf("servizio!\n");

            fflush(stdout);
        }
    }
}

//Stampa la lista degli ordini del tavolo "table"
void print_table_orders(int table){
    struct Table* t;
    struct Order* tmp;
    struct Plate* tmp1;
    t = _find_table(table);
    int i = 0;

    if(t == NULL){
        printf("Tavolo non valido!\n");
        fflush(stdout);
        return;
    }

    if(t->orders == NULL){
        printf("Non ci sono comande in servizio o in preparazione!\n");
        fflush(stdout);
    }

    tmp = t->orders;
    while(tmp != NULL)
    {
        printf("com%d ", tmp->id);
        if(tmp->state == s)
            printf("<in servizio>\n");
        else
            printf("<in preparazione>\n");

        fflush(stdout);

        tmp1 = tmp->plates;
        while(tmp1 != NULL){
            printf("%c%d %d\n", tmp1->type, tmp1->id, tmp1->num);
            tmp1 = tmp1->next;
        }

        fflush(stdout);
        tmp = tmp->next;
    }

    tmp = orderList;
    while(tmp != NULL)
    {
        if(tmp->table == t->id)
        {
            printf("com%d <in attesa>\n", tmp->id);
            tmp1 = tmp->plates;
            while(tmp1 != NULL){
                printf("%c%d %d\n", tmp1->type, tmp1->id, tmp1->num);
                fflush(stdout);
                tmp1 = tmp1->next;
            }
            i++;
        }
        tmp = tmp->next;
    }

    if(i == 0) {
        printf("Non ci sono comande in attesa!\n");
        fflush(stdout);
    }
}

//Aggiunge un ordine al tavolo "table", gestito dal KD "kd" (l'ordine passa da "attesa" a "preparazione")
void _table_order_add(struct Order* order, int table, int kd)
{
    struct Table* t = _find_table(table);
    struct Order* temp;

    if(t == NULL)
    {
        printf("Tavolo non trovato!\n");
        free(order);
        return;
    }

    order->kd = kd;
    order->state = p;
    order->next = NULL; //lo aggiungo in coda

    if(t->orders == NULL) {
        t->orders = order;
        return;
    } else {
        temp = t->orders;
        while (temp->next != NULL)
            temp = temp->next;

        temp->next = order;
    }
}

//Prende il primo ordine dalla lista degli ordini in attesa e lo assegna al kd, spostandolo nella lista
//relativa al tavolo
struct Order* take_order(int kd)
{
    struct Order* head;

    if(orderList == NULL)
        return NULL;

    head = orderList;
    orderList = orderList->next;
    _table_order_add(head, head->table, kd);

    return head;
}

//Serve a creare una copia dell'ordine "order" nella lista "orders" -> comando show
void _update_order_list(struct Order** orders, struct Order* order)
{
    struct Order* new = malloc(sizeof(struct Order));
    struct Order* tmp;
    new->id = order->id;
    new->table = order->table;
    new->kd = order->kd;
    new->state = order->state;
    new->next = NULL;
    new->plates = order->plates; //qui può andar bene passare il puntatore alla lista dei piatti, basta non deallocarli dopo

    if(*orders == NULL) {
        *orders = new;
    } else {
        tmp = *orders;
        while (tmp->next != NULL)
            tmp = tmp->next;

        tmp->next = new;
    }
}

//Crea una lista degli ordini -> comando show
struct Order* get_orders(sd)
{
    struct Order* orders, *tmp;
    int i;

    orders = NULL;
    for(i = 0; i < tableCount; i++)
    {
        //Controllo la lista degli ordini di ogni tavolo e recupero quelli in preparazione
        //associati al kd sd
        tmp = tableList[i].orders;
        while(tmp != NULL)
        {
            if(tmp->kd == sd && tmp->state == p)
                _update_order_list(&orders, tmp);
            tmp = tmp->next;
        }
    }

    return orders;
}

//Setta l'ordine come pronto
int order_ready(int kd, int com, int table)
{
    struct Table* t = _find_table(table);
    struct Order* temp;

    if(t == NULL){
        printf("Tavolo non trovato!\n");
        fflush(stdout);
        return 1;
    }

    temp = t->orders;
    //Controllo se nel tavolo c'è effettivamente la comanda com ed è in attesa
    while(temp != NULL)
    {
        if(temp->id == com && temp->state == p && temp->kd == kd){
            temp->state = s;
            return 0;
        }

        temp = temp->next;
    }

    printf("Comanda non valida!\n");
    fflush(stdout);
    return 1;
}

//Resetta la lista di ordini di un tavolo (dopo il comando conto)
//Gli ordini del tavolo ancora in attesa vengono scartati, gli altri finiscono nell'archivio
void reset_order_list(int table)
{
    struct Order* orders, *tmp, *del;
    struct Order* oldtmp;
    struct Plate* tmp1;

    printf("Resetto le comande del tavolo %d\n", table);
    //Resetto le comande relative al tavolo che si trovano in servizio o in preparazione
    //e le salvo in una lista "old"
    if(old == NULL) {
        old = tableList[table - 1].orders;
    } else {
        //creo un puntatore alla coda
        oldtmp = old;
        while (oldtmp->next != NULL)
            oldtmp = oldtmp->next;

        oldtmp->next = tableList[table - 1].orders;
    }

    tableList[table-1].orders = NULL;
    //tableList[table-1].order_count = 0; //posso decommentarlo se volessi far ripartire gli id delle comande da 0

    //Resetto le comande relative al tavolo che si trovano in attesa
    //Queste qui le scarto, non sono state servite quindi è inutile conservarle
    //orders -> puntatore principale
    //tmp -> puntatore elemento precedente
    //del -> puntatore roba da eliminare
    //tmp1 -> puntatore ai piatti
    orders = orderList; //inizializzo orders in testa
    tmp = NULL;

    while(orders != NULL){ //scorro TUTTA la lista
        if(orders->table == table){
            //Devo eliminare l'elemento
            if(orders == orderList){
                //elimino in testa
                orderList = orderList->next;
            }

            //Sia che io debba eliminare dalla testa, sia che io debba eliminare in mezzo che in coda
            //devo fare una free su orders e cambiare il puntatore next dell'elemento precedente a orders->next (a meno che non sono in testa)
            del = orders;
            if(tmp != NULL)
                tmp->next = orders->next;

            while(del->plates != NULL){
                tmp1 = del->plates;
                del->plates = del->plates->next;
                free(tmp1);
            }

            orders = orders->next; //sposto orders in avanti
            free(del);

            //In questo caso non devo spostare tmp in avanti
            continue;
        }

        tmp = orders; //Faccio puntare tmp all'elemento precedente
        orders = orders->next; //Sposto orders al successivo
    }
}

//Controlla se ci sono comande in preparazione o in attesa
int check_commands(){
    int i, ret = 0;
    struct Order* tmp;
    if(orderList != NULL)
        return 1;

    for(i = 0; i < tableCount; i++)
    {
        if(tableList[i].orders != NULL){
            tmp = tableList[i].orders;
            while(tmp != NULL){
                if(tmp->state == p){
                    ret++;
                    break;
                }
                tmp = tmp->next;
            }
            if(ret != 0)
                break;
        }
    }

    return ret;
}

//Ritorna il numero di tavoli
int get_table_count(){
    return tableCount;
}

//Svuola la lista delle prenotazioni
void delete_reservations(){
    int i;
    struct Reservation* tmp;

    for(i = 0; i < tableCount; i++){
        while(tableList[i].resList != NULL){
            tmp = tableList[i].resList;
            tableList[i].resList = tableList[i].resList->next;
            free(tmp);
        }
    }

    printf("Prenotazioni svuotate!\n");
    fflush(stdout);
}

//Svuota le liste degli ordini
//Essendo eseguita dopo la chiamata di stop basterebbe svuotare la lista "old"
//Potrebbe comunque essere utile avere una funzione che le svuota tutte
void delete_orders(){
    int i;
    struct Order* tmp;
    struct Plate* tmp1;

    //Ordini in attesa
    while(orderList != NULL){
        while(orderList->plates != NULL){
            tmp1 = orderList->plates;
            orderList->plates = orderList->plates->next;
            free(tmp1);
        }
        tmp = orderList;
        orderList = orderList->next;
        free(tmp);
    }

    //Ordini in preparazione/in servizio
    for(i = 0; i < tableCount; i++){
        while(tableList[i].orders != NULL){
            while(tableList[i].orders->plates != NULL)
            {
                tmp1 = tableList[i].orders->plates;
                tableList[i].orders->plates = tableList[i].orders->plates->next;
                free(tmp1);
            }
            tmp = tableList[i].orders;
            tableList[i].orders = tableList[i].orders->next;
            free(tmp);
        }
    }

    //Archivio
    while(old != NULL){
        while(old->plates != NULL){
            tmp1 = old->plates;
            old->plates = old->plates->next;
            free(tmp1);
        }
        tmp = old;
        old = old->next;
        free(tmp);
    }

    printf("Liste delle comande svuotate!\n");
    fflush(stdout);
}

//Svuota la lista dei tavoli
void delete_tables(){
    free(tableList);
    printf("Lista dei tavoli svuotata!\n");
    fflush(stdout);
}