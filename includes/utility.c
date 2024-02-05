//
// Created by vitog on 05/07/2023.
//
#include "header.h"
#include "utility.h"

//Recupero la porta passata come parametro di avvio al server o ai vari client
int get_port(int argc, char* argv[])
{
    int port;
    if(argc != 2)
    {
        printf("Errore in fase di avvio: numero di parametri errato.\nAvviare con ./server [port]\n");
        fflush(stdout);
        return -1;
    } else if(argc == 2) {
        port = atoi(argv[1]);
        if(port == 0){
            printf("Errore: la porta specificata non è valida.\n");
            fflush(stdout);
            return -1;
        }
        return port;
    }

    return -1;
}

//Invio un messaggio sfruttando il seguente protocollo:
//1) Invio la lunghezza del messaggio
//2) Invio il messaggio
int send_msg(int sd, char* msg)
{
    int ret;
    uint16_t msg_len;

    msg_len = htons(strlen(msg)+1);
    ret = send(sd, (void*)&msg_len, sizeof(uint16_t), 0);
    if(ret < 0)
        return ret;

    ret = send(sd, (void*)msg, strlen(msg)+1, 0);
    return ret;
}

//Ricevo un messaggio sfruttando il seguente protocollo:
//1) Ricevo la lunghezza del messaggio
//2) Ricevo il messaggio
int recv_msg(int sd, char* msg)
{
    int ret;
    uint16_t msg_len;

    ret = recv(sd, (void*)&msg_len, sizeof(uint16_t), 0);
    if(ret <= 0)
        return ret;

    msg_len = ntohs(msg_len);
    ret = recv(sd, (void*)msg, msg_len, 0);
    return ret;
}