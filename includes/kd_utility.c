//
// Created by vitog on 07/07/2023.
//
#include "header.h"
#include "utility.h"
#include "kd_utility.h"

//Gestisce l'autenticazione con il server
int authentication_handler(int sd, char* buff)
{
    int ret;
    char* msg = "kd\0";

    ret = send_msg(sd, msg);
    if(ret < 0)
        return ret;

    ret = recv_msg(sd, buff);
    if(ret <= 0)
        return ret;

    if(strcmp(buff, "OK") != 0){
        perror("Errore in fase di autenticazione!\n");
        exit(1);
    }

    return ret;
}

//Mostra la guida
void show_help()
{
    printf("take\n Prende la prossima comanda in attesa.\n");
    printf("show\n"
           " Mostra le comande in preparazione.\n");
    printf("ready com\n Marca la comanda com come pronta.\n"
           " Esempio di utilizzo: ready com1-T2\n");
    fflush(stdout);
}

//Gestisce l'esecuzione del comando take
int take(int sd, char* buff)
{
    int ret;

    ret = send_msg(sd, buff);
    if(ret < 0)
        return ret;

    //ricevo i piatti della comanda
    for(;;)
    {
        ret = recv_msg(sd, buff);
        if(ret <= 0)
            return ret;

        if(strcmp(buff, "END") == 0)
            break;

        printf("%s\n", buff);
        fflush(stdout);
    }

    return ret;
}

//Gestisce l'autenticazione del comando show
int show(int sd, char* buff)
{
    int ret;

    ret = send_msg(sd, buff);
    if(ret < 0)
        return ret;

    //ricevo i piatti della comanda
    for(;;)
    {
        ret = recv_msg(sd, buff);
        if(ret <= 0)
            return ret;

        if(strcmp(buff, "END") == 0)
            break;

        printf("%s\n", buff);
        fflush(stdout);
    }

    return ret;
}

//Gestisce l'esecuzione del comando ready
int ready(int sd, char* buff)
{
    int ret, com, table;

    ret = sscanf(buff, "ready com%d-T%d", &com, &table);
    if(ret != 2){
        printf("Errore nell'utilizzo del comando!\nready comN-TN\n");
        fflush(stdout);
        return 1;
    }

    sprintf(buff, "ready %d %d", com, table);
    ret = send_msg(sd, buff);
    if(ret < 0)
        return ret;

    //ricevo la risposta
    ret = recv_msg(sd, buff);
    if(ret <= 0)
        return ret;

    printf("%s\n", buff);
    fflush(stdout);
    return ret;
}