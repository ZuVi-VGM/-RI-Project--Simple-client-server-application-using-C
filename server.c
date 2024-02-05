//
// Created by vitog on 27/06/2023.
//

#include "includes/header.h"
#include "includes/utility.h"
#include "includes/server_utility.h"

int main(int argc, char* argv[])
{
    int ret, port, listener, new_sd, addrlen, i;
    fd_set master;
    fd_set r_fd;
    int fd_max;

    struct sockaddr_in my_addr, cl_addr;
    char buff[BUF_SIZE];
    char cmd[5];

    //Controllo se l'avvio è corretto e recupero la porta su cui effettuare il bind (includes/utility.c)
    port = get_port(argc, argv);
    if(port < 0)
        exit(1);

    //Recupero dati ed inizializzazione delle strutture dati
    printf("Recupero la lista dei tavoli\n");
    get_tables();
    printf("Recupero la lista delle prenotazioni\n");
    get_reservations();

    printf("Recupero il menù\n");
    get_dishes();

    printf("Il server si avvierà in ascolto sulla porta %d\n", port);
    fflush(stdout);

    //Inizializzo il socket di ascolto
    listener = socket(AF_INET, SOCK_STREAM, 0);

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port =  htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    //Effettuo il bind
    ret = bind(listener, (struct sockaddr*)&my_addr, sizeof(my_addr));

    if(ret < 0){
        perror("Errore in fase di bind: ");
        exit(1);
    }

    ret = listen(listener, MAX_QUEUE); //MAX_QUEUE default = 10 (includes/header.h)
    if(ret < 0){
        perror("Errore durante la listen: ");
        exit(1);
    }

    //Inizializzo i set per la select
    FD_ZERO(&master);
    FD_ZERO(&r_fd);

    FD_SET(0, &master);
    FD_SET(listener, &master);
    fd_max = listener;

    printf("Server in ascolto sulla porta %d\n", port);
    //Stampo la guida
    printf("------------------------------------\n");
    fflush(stdout);
    printf("stat table|status\n Senza parametri restituisce lo stato di tutte le comande giornaliere.\n"
           " Passando come argomento un tavolo (es: stat T1) restituirà lo stato delle comande del tavolo.\n"
           " Passando come argomento uno status (es: stat a,p,s) restituirà l'elenco delle comande in quello stato.\n");
    printf("stop\n Arresta il server se non sono presenti comande in attesa o in preparazione.\n");
    fflush(stdout);

    while(1)
    {
        //MAGIA
        r_fd = master;

        ret = select(fd_max+1, &r_fd, NULL, NULL, NULL);
        if(ret < 0){
            perror("Errore nella select: ");
            exit(1);
        }

        for(i=0; i <= fd_max; i++)
        {
            if(FD_ISSET(i, &r_fd))
            {
                if(i == 0){
                    //STDIN
                    fgets(buff, STR_SIZE, stdin);

                    sscanf(buff, "%s", cmd);
                    if(strcmp(cmd, "stat") == 0)
                    {
                        stat_handler(buff);
                    } else if (strcmp(cmd, "stop") == 0) {
                        ret = stop_handler();
                        if(ret == 0) {
                            close(listener);
                            exit(0);
                        }
                    } else {
                        printf("Comando non valido!\n");
                        fflush(stdout);
                    }
                } else if(i == listener){
                    printf("Nuova connessione in entrata!\n");
                    fflush(stdout);
                    addrlen = sizeof(cl_addr);
                    new_sd = accept(listener, (struct sockaddr*)&cl_addr, (socklen_t*)&addrlen);
                    if (new_sd < 0){
                        perror("Errore in fase di accept(): \n");
                        continue;
                    }

                    //La fase di autenticazione posso gestirla in server_utility
                    ret = authentication_handler(new_sd, buff);
                    if(ret <= 0)
                    {
                        perror("Errore in fase di autenticazione: \n");
                        close(new_sd);
                        continue;
                    } else if (ret == 500){
                        //Dispositivo non valido
                        printf("Dispositivo non valido!\n");
                        fflush(stdout);
                        close(new_sd);
                        continue;
                    }

                    FD_SET(new_sd, &master);
                    if(new_sd > fd_max)
                        fd_max = new_sd;
                } else {
                    //Ricezione!
                    ret = recv_msg(i, buff);
                    if (ret < 0) {
                        perror("Errore in fase di ricezione: \n");
                        continue;
                    } else if (ret == 0) {
                        printf("Connessione chiusa!\n");
                        //Se era un td o un kd aggiorna le corrispondenti liste
                        handle_disconnection(i);
                        close(i);
                        FD_CLR(i, &master);
                        continue;
                    }

                    printf("Comando %s ricevuto!\n", buff);
                    fflush(stdout);
                    sscanf(buff, "%s", cmd);
                    if (strcmp(cmd, "find") == 0) {
                        ret = find_handler(i, buff);
                    } else if (strcmp(cmd, "book") == 0) {
                        ret = book_handler(i, buff);
                    } else if (strcmp(cmd, "menu") == 0) {
                        ret = menu_handler(i, buff);
                    } else if (strcmp(cmd, "comanda") == 0) {
                        ret = com_handler(i, buff);
                    } else if(strcmp(cmd, "conto") == 0) {
                        ret = conto_handler(i, buff);
                    } else if(strcmp(cmd, "take") == 0){
                        ret = take_handler(i, buff);
                    } else if(strcmp(cmd, "show") == 0){
                        ret = show_handler(i, buff);
                    } else if(strcmp(cmd, "ready") == 0){
                        ret = ready_handler(i, buff);
                    }

                    //gestisco l'esito dei comandi
                    if(ret <= 0) {
                        if (ret == 0) {
                            printf("Connesione chiusa!\n");
                            close(i);
                            FD_CLR(i, &master);
                            continue;
                        } else {
                            printf("Errore durante la gestione della prenotazione.\n");
                            continue;
                        }
                    }
                }
            }
        }
    }

    printf("Chiusura del socket di ascolto!");
    fflush(stdout);
    close(listener);
    exit(0);
}