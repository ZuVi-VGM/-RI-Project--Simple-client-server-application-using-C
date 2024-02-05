//
// Created by vitog on 02/07/2023.
//

#include "includes/header.h"
#include "includes/utility.h"
#include "includes/client_utility.h"

int main(int argc, char* argv[])
{
    int ret, port, sd, i;
    fd_set master;
    fd_set r_fd;
    int fd_max;
    struct sockaddr_in srv_addr, my_addr;
    char buff[BUF_SIZE];
    char comando[5];

    //Controllo se l'avvio è corretto e recupero la porta su cui effettuare il bind (includes/utility.c)
    port = get_port(argc, argv);
    if(port < 0)
        exit(1);

    sd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr));
    if (ret < 0){
        perror("Errore in fase di bind: \n");
        exit(1);
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(4242);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);

    ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if(ret < 0)
    {
        perror("Errore di connessione: ");
        exit(1);
    }

    //Effettuo l'autenticazione
    ret = authentication_handler(sd, buff);

    if(ret <= 0){
        if(ret == 0)
            printf("Autenticazione rifiutata!\n");
        else
            perror("Errore in fase di autenticazione: \n");

        exit(1);
    }

    printf("Connesso ed autenticato!\n");
    fflush(stdout);

    //preparo i parametri per la select
    FD_ZERO(&master);
    FD_ZERO(&r_fd);

    FD_SET(0, &master);
    FD_SET(sd, &master);
    fd_max = sd;

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
                if(i==0){
                    //STDIN
                    fgets(buff, 50, stdin);

                    sscanf(buff, "%s", comando);
                    if(strcmp(comando, "esc") == 0)
                    {
                        printf("CHIUSURA\n");
                        fflush(stdout);
                        close(sd);
                        exit(0);
                    } else if(strcmp(comando, "find") == 0) {
                        ret = find_handler(sd, buff);

                        if(ret <= 0)
                        {
                            if(ret == 0)
                            {
                                printf("Connessione con il server interrotta!\n");
                                close(sd);
                                exit(1);
                            } else {
                                perror("Errore durante l'esecuzione: \n");
                                continue;
                            }
                        }
                    } else if(strcmp(comando, "book") == 0) {
                        printf("Il comando book può essere eseguito solo dopo la find!\n");
                        fflush(stdout);
                        continue;
                    } else {
                        printf("Comando non valido!\n");
                        fflush(stdout);
                        continue;
                    }
                } else if(i==sd){
                    //Qui gestisco semplicemente la chiusura del server
                    ret = recv_msg(sd, buff);
                    if(ret < 0)
                    {
                        //errore
                        perror("Errore in fase di ricezione: \n");
                        continue;
                    } else if(ret == 0) {
                        printf("Server chiuso!\n");
                        close(i);
                        exit(0);
                    }

                    //Debug
                    printf("%s\n", buff);
                    fflush(stdout);
                }
            }
        }
    }
}