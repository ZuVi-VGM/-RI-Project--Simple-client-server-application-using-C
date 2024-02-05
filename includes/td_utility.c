//
// Created by vitog on 05/07/2023.
//

#include "header.h"
#include "utility.h"
#include "td_utility.h"

//Gestisce l'autenticazione con il server
int authentication_handler(int sd, char* buff)
{
    int ret, tab;
    char* msg = "td\0";
    char b[STR_SIZE];

    ret = send_msg(sd, msg);
    if(ret < 0)
        return ret;

    ret = recv_msg(sd, buff);
    if(ret <= 0)
        return ret;

    sscanf(buff, "%[^_]_%d", b, &tab);
    if(strcmp(b, "OK") != 0){
        perror("Errore in fase di autenticazione!\n");
        exit(1);
    }

    return tab;
}

//Mostra la guida
void show_help()
{
    printf("menu\n Mostra il menù, ossia codice, nome del piatto e prezzo.\n");
    printf("comanda {<piatto_1-quantità_1>...<piatto_n-quantità_n>}\n"
           " Consente l'invio di una comanda. Un esemio di utilizzo è il seguente:\n"
           " comanda A1_1 A2_3\n");
    printf("conto\n Mostra l'elenco dei piatti consumati ed il calcolo del conto.\n");
    fflush(stdout);
}

//Gestisce il comando menu
int get_menu(int sd, char* buff)
{
    int ret;

    ret = send_msg(sd, buff);
    if(ret < 0)
        return ret;

    //ricevo i piatti del menù
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

//Gestisce il comando "comanda"
int com(int sd, char* buff, int table)
{
    int ret;
    char comanda[STR_SIZE];

    ret = sscanf(buff, "comanda %[^\n]", comanda);
    if(ret != 1){
        printf("Errore nell'utilizzo del comando!\n");
        fflush(stdout);
        return 1;
    }

    sprintf(buff, "comanda %d %s", table, comanda);
    ret = send_msg(sd, buff);
    if(ret < 0)
        return ret;

    ret = recv_msg(sd, buff);
    if(ret <= 0)
        return ret;

    printf("%s\n", buff);
    return ret;
}

//Gestisce il comando conto
int conto(int sd, char* buff, int table)
{
    int ret;

    sprintf(buff, "conto %d", table);
    ret = send_msg(sd, buff);
    if(ret < 0)
        return ret;

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