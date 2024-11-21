
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include "include/structure.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4242
#define DIM_BUFFER 1024

int main(int argc, char *argv[]){
    int fdmax, ret, i;//descrittore massimo, valore di ritorno, contatore ciclo
    fd_set  master, read_fds;//2 set di descrittori
    char buffer[DIM_BUFFER];//buffer per memorizzare i dati
    struct sockaddr_in sv_addr;//struttura per l'indirizzo del server
    in_port_t porta = htons(SERVER_PORT);//porta del server convertita in network byte order

    char choosenSet[2], *tmp;//scelta dello scenario, variabile temporanea per strtok
    /* per bloccare l'invio dei mex in caso di giocatore secondario */
    bool canSend = true;

    printf("! Avvio e inizializzazione del client in corso...\n\n");
    fdmax = socket(AF_INET, SOCK_STREAM, 0);
    if(fdmax < 0) {
        perror("ERR: Errore creazione socket");
        exit(1);
    }

    memset(&sv_addr, 0, sizeof(sv_addr));//svuoto struct per indirizzo server
    sv_addr.sin_family = AF_INET;
    sv_addr.sin_port = porta;
    inet_pton(AF_INET, SERVER_IP, &sv_addr.sin_addr);//converte l'indirizzo IP in formato binario (da presentazione a numerico)

    ret = connect(fdmax, (struct sockaddr*)&sv_addr, sizeof(sv_addr));//invio richiesta di connessione al server
    if(ret < 0) {
        perror("ERR: Errore nella connessione con il server");
        exit(1);
    }
    printf("OK: Client inizializzato\n\n");

    printf("******************* CLIENT AVVIATO *******************\n");
    memset(buffer, 0, DIM_BUFFER);//svuoto il buffer prima di ricevere il messaggio iniziale
    
    /* Ricezione del messaggio iniziale con le istruzioni*/
    ret = recv(fdmax, buffer, DIM_BUFFER, 0);
    if(ret < 0){
        perror("ERR: Errore nella ricezione delle istruzioni");
        exit(1);
    }
    printf("%s\n"
            "******************************************************\n\n", buffer);
    //svuoto i set e poi aggiungo il descrittore massimo e lo stdin
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(fdmax, &master);
    FD_SET(STDIN_FILENO, &master);

    for(;;) {
        memset(buffer, 0, DIM_BUFFER);//svuoto il buffer
        read_fds = master;//copio il set master in read_fds
        select(fdmax + 1, &read_fds, NULL, NULL, NULL);//monitora i descrittori per vedere se sono pronti per la lettura

        for(i = 0; i <= fdmax; i++) {
            /* Controllo dello STDIN per ricezione comandi da terminale del client */
            if(FD_ISSET(i, &read_fds)) {//se l'i-esimo socket è pronto...

                if(i == STDIN_FILENO) {//verifica se il descrittore di file pronto per la lettura è lo standard input (STDIN_FILENO), il che significa che il client ha ricevuto un comando dalla console.
                    memset(buffer, 0, DIM_BUFFER);//svuoto il buffer
                    fgets(buffer, DIM_BUFFER, stdin);//legge una riga di input dallo standard input (console) e la memorizza nel buffer.

                    /* se siamo giocatori secondari e non usiamo il comando end*/
                    if(canSend == false  && strstr(buffer, "end") == NULL) {//verifica se il client non è autorizzato a inviare messaggi (canSend == false) e se il comando non contiene la stringa "end"
                        printf("Ancora non sei stato interpellato dal giocatore principale\n");//indica che il client non è autorizzato a inviare messaggi
                        continue;
                    }

                    /* eliminazione del carattere di newline e aggiunta dello spazio finale per strtok del messaggio lato server*/
                    strtok(buffer, "\n"); //utilizza strtok per trovare e rimuovere il carattere di newline (\n) dal buffer. strtok suddivide la stringa buffer in token utilizzando il carattere di newline come delimitatore e restituisce il primo token.
                    strcat(buffer, " ");//aggiunge uno spazio alla fine del buffer. Questo è utile per garantire che il messaggio sia formattato correttamente per l'elaborazione lato server.

                    ret = send(fdmax, buffer, DIM_BUFFER, 0);//invio il contenuto del buffer al server
                    if(ret < 0){
                        perror("ERR: Errore durante l'invio del messaggio");
                        exit(1);
                    }


                    /* se al momento dell'inserimento del comando di start si sceglie il personaggio secondario blocchiamo l'invio al server, 
                    il client secondario deve interagire solo quando viene chiamato, non puoi partire come pg secondario*/
                    if(strcmp(buffer, "start ") != 0 && strstr(buffer, "start") != NULL) {//verifica se il comando non è esattamente "start " e se contiene la stringa "start"
                        strtok(buffer, " ");//suddividiamo il buffer in token utilizzando lo spazio come delimitatore e restituiamo il primo token
                        tmp = strtok(NULL, " ");//ottiene il prossimo token, che dovrebbe essere il tipo di giocatore
                        if(tmp != NULL) {
                            strcpy(choosenSet, tmp);
                            tmp = strtok(NULL, " ");
                            if(tmp != NULL) {
                                strcpy(buffer, tmp);
                                if(strcmp(buffer, "2") == 0)
                                    canSend = false;
                                else 
                                    canSend = true; 
                            }
                        }
                    }
                    
                }
                /* Altrimenti è il server che invia messaggi direttamente, come infos e join del giocatore secondario*/
                else if(i == fdmax) {//allora è pronto il nostro socket
                    memset(buffer, 0, DIM_BUFFER);//svuoto il buffer
                    ret = recv(fdmax, buffer, DIM_BUFFER, 0);//leggo il messaggio dal server e lo memorizzo nel buffer

                    if(ret == 0) {//ha chiuso la connessione
                        printf("Disconnessione avvenuta\n");
                        exit(0);
                    }

                    if(ret < 0) {
                        perror("ERR: Errore in ricezione del messaggio");
                        exit(1);
                    }

                    /* set a true del canSend in caso di chiamata in corso da parte del client principale*/
                    if(strcmp(buffer, "chiamata in corso") == 0) {
                        canSend = true;//imposta canSend a true, indicando che adesso il client secondario è autorizzato a inviare messaggi
                        if(strcmp(choosenSet, "1") == 0) { /*se lo scenario scelto è Museum*/
                            printf("\nUn prigioniero ti sta chiamando, quanti diamanti gli vuoi chiedere?\n");
                        } 
                        /*else if(...) per gli altri scenari */
                    }  
/*verifica se il messaggio ricevuto dal server contiene uno dei seguenti errori:
"scenario non esistente"
"giocatore inserito non esistente"
"devi prima fare login"
"devi prima avviare una partita"
"ALERT"
"Richiesta errata"
In questo casoriportiamo canSend a true: indicando che il client è autorizzato a inviare messaggi nonostante l'errore.*/
                    if(strstr(buffer, "scenario non esistente") != NULL
                        || strstr(buffer, "giocatore inserito non esistente") != NULL
                        || strstr(buffer, "devi prima fare login") != NULL 
                        || strstr(buffer, "devi prima avviare una partita") != NULL
                        || strstr(buffer, "ALERT") != NULL
                        || strstr(buffer, "Richiesta errata") != NULL) {

                        canSend = true;
                    }

                    if(strstr(buffer, "Diamanti insufficienti") != NULL)// se il server ci manda questa stringa il client non può inviare altri msg poichè non abbiamo soddisfatto la richiesta
                        canSend = false;

                    if(strcmp(buffer, "chiamata in corso") != 0)//se il messaggio NON è "chiamata in corso" stampa il messaggio ricevuto dal server
                        printf("%s\n", buffer);
                }
            }
        } 
    } 
}