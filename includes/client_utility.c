//
// Created by vitog on 02/07/2023.
//
#include "header.h"
#include "utility.h"
#include "client_utility.h"

//Gestisce l'autenticazione con il server
int authentication_handler(int sd, char* buff)
{
    int ret;
    char* msg = "cl\0";

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

//Gestisce l'esecuzione del comando find e book
int find_handler(int sd, char* buff)
{
    int ret;
    char cmd[5];
    int i = 0, j;
    int* tlist = NULL;

    char name[STR_SIZE];
    int people;
    time_t rawtime;
    struct tm* t_struct = malloc(sizeof(struct tm));
    struct tm* n_struct = malloc(sizeof(struct tm));
    t_struct->tm_isdst = 1;
    n_struct->tm_isdst = 1;

    //Recupero i parametri
    ret = sscanf(buff, "find %s %d %d-%d-%d %d", name, &people, &t_struct->tm_mday, &t_struct->tm_mon, &t_struct->tm_year, &t_struct->tm_hour);
    if(ret < 6) {
        printf("Errore nell'utilizzo della find!\n");
        printf("find nome persone dd-mm-YYYY HH\n");
        fflush(stdout);
        return 1;
    }

    //Serve per ottenere il timestamp corretto con mktime
    t_struct->tm_mon--;
    t_struct->tm_year -= 1900;

    //Faccio una copia di t_struct per controllare il caso in cui vengano inseriti parametri non validi all'interno dei campi
    n_struct->tm_mday = t_struct->tm_mday;
    n_struct->tm_mon = t_struct->tm_mon;
    n_struct->tm_year = t_struct->tm_year;

    rawtime = mktime(t_struct);

    if(n_struct->tm_mday != t_struct->tm_mday || n_struct->tm_mon != t_struct->tm_mon || n_struct->tm_year != t_struct->tm_year || difftime(rawtime, time(NULL)) <= 0){
        printf("Inserire una data valida!\n");
        fflush(stdout);
        free(t_struct);
        return 1;
    }

    free(t_struct);

    //Invio il comando
    sprintf(buff, "find %s %d %u", name, people, (unsigned)rawtime);
    ret = send_msg(sd, buff);
    if(ret < 0)
        return ret;

    //Ricevo i dati /finché non ricevo END
    for(;;) {
        ret = recv_msg(sd, buff);
        if(ret <= 0)
            return ret;

        if(strcmp(buff, "END") != 0) {
            if(tlist == NULL)
                tlist = malloc(sizeof(int));
            else
                tlist = realloc(tlist, (i+1)*sizeof(int));
            
            sscanf(buff, "T%d", &j);
            tlist[i] = j;
            printf("%d) %s\n", ++i, buff); //qua devo creare l'array di tavoli
        } else {
            break;
        }
    }

    if(i == 0){
        printf("Non ci sono tavoli disponibili per la data e l'orario richiesti!\n");
        fflush(stdout);
        return ret;
    }

    j=0;
    //Se la lista non è vuota mi metto in attesa di ricevere il comando book
    for(;;) {
        fgets(buff, 50, stdin);

        sscanf(buff, "%s %d", cmd, &j);
        if(strcmp(cmd, "book") == 0 && j > 0 && j <= i)
            break;
        else {
            printf("Comando non valido! Per proseguire inserire book i\n");
            fflush(stdout);
            continue;
        }
    }

    sprintf(buff, "%s %d %s %d %u", cmd, tlist[j-1], name, people, (unsigned)rawtime);
    ret = send_msg(sd, buff);
    if(ret < 0)
        return ret;

    ret = recv_msg(sd, buff);
    if(ret <= 0)
        return ret;

    printf("%s\n", buff);
    fflush(stdout);
    free(tlist);
    return ret;
}