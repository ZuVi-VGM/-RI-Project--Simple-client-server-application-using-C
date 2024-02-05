//
// Created by vitog on 07/07/2023.
//

#include "includes/header.h"
#include "includes/utility.h"
#include "includes/kd_utility.h"

int main(int argc, char* argv[])
{
    int ret, port, sd, i;
    fd_set master;
    fd_set r_fd;
    int fd_max;
    struct sockaddr_in srv_addr, my_addr;
    char buff[BUF_SIZE];
    char comando[5];

    //Controllo se l'avvio è corretto e recupero la porta su cui effettuare il bind (includes/client_utility.c)
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

    printf("***************************** BENVENUTO *****************************\n"
           "Digita un comando:\n\n"
           "1) take\t\t--> accetta una comanda\n"
           "2) show\t\t--> mostra le comande in preparazione\n"
           "3) ready\t\t--> imposta lo stato della comanda su pronto\n");
    fflush(stdout);

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
                if(i==0) {
                    //STDIN
                    fgets(buff, 50, stdin);
                    //printf("Comando %s ricevuto!\n", buff);
                    //fflush(stdout);

                    sscanf(buff, "%s", comando);
                    if (strcmp(comando, "esc") == 0) {
                        printf("CHIUSURA\n");
                        fflush(stdout);
                        close(sd);
                        exit(0);
                    } else if(strcmp(comando, "help") == 0) {
                        show_help();
                    } else if(strcmp(comando, "take") == 0) {
                        strcpy(buff, comando); //in caso l'utente avesse inserito qualcosa dopo "menu"
                        ret = take(sd, buff);
                    } else if(strcmp(comando, "show") == 0) {
                        strcpy(buff, comando);
                        ret = show(sd, buff);
                        continue;
                    } else if(strcmp(comando, "ready") == 0){
                        ret = ready(sd, buff);
                        continue;
                    }else {
                        printf("Comando non valido!\n");
                        fflush(stdout);
                        continue;
                    }

                    //Gestisco eventuali errori di comunicazione
                    if(ret <= 0)
                    {
                        if(ret == 0){
                            printf("Connessione chiusa!\n");
                            fflush(stdout);
                            close(sd);
                            exit(0);
                        } else {
                            printf("Errore nell'esecuzione del comando!\n");
                            fflush(stdout);
                            continue;
                        }
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

                    sscanf(buff, "%s", comando);
                    if(strcmp(comando, "CLOSE")){
                        close(sd);
                        printf("Server in chiusura!\n");
                        fflush(stdout);
                        exit(0);
                    }
                }
            }
        }
    }
}