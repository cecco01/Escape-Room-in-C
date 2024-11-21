#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "include/auth.h"
#include "include/structure.h"
#include "include/command.h"
#include "include/utils.h"
#include "scenari/Museum/Museum.h"

int main(int argc, char *argv[]){
    int fdmax, ret, i, listening_socket, communication_socket;// max descrittore di file, valore per memorizzare il return delle primitive, contatore cicli, socket ascolto, socket comunicazione
    fd_set  master, readfds;//i 2 set di descrittori
    socklen_t addrlen;//lunghezza indirizzo socket
    struct sockaddr_in  sv_addr, cl_addr;//strutture per l'inidirizzo del socket server e client
    in_port_t porta = htons(atoi(argv[1]));//porta del server convertita in network byte order
    char buffer[DIM_BUFFER], type[DIM_TYPE];//array di char: il 1 per memorizzare dati, il 2 per memorizzare il tipo di comando
    struct session* current_session;//puntatore alla sessione corrente
    struct user* current_user;//puntatore all'utente corrente


    /* ------------------------------------
        Avvio e inizializzazione del server 
       ------------------------------------ */ 
    printf("! Avvio e inizializzazione del server in corso...\n\n");

    listening_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(listening_socket < 0) {//controllo esito socket
        perror("ERR: Errore nella creazione del socket - ");
        exit(1);
    }

    /* permette il riavvio del server instantanemente senza problemi di porta*/
    ret = setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    if(ret < 0) {
        perror("ERR: Errore in setsockopt(...) - ");
        exit(1);
    }

    memset(&sv_addr, 0, sizeof(sv_addr));//mi assicuro che i byte della struct siano tutti nulli
    sv_addr.sin_family = AF_INET;//famiglia di indirizzi
    sv_addr.sin_port = porta;//porta server
    sv_addr.sin_addr.s_addr = INADDR_ANY;//imposta l'IP del server ( significa che il server accetterà connessioni su tutte le interfacce di rete disponibili.)

    /*associo il socket in ascolto ad indirizzo e porta presenti nella struttura chiamata "sv_addr"*/
    ret = bind(listening_socket, (struct sockaddr*)&sv_addr, sizeof(sv_addr));
    if(ret < 0) {
        perror("ERR: Errore bind - ");
        exit(1);
    }
    /*ora pongo il socket in ascolto*/
    ret = listen(listening_socket, MAX_CLIENT);
    if(ret < 0) {
        perror("ERR: Errore Listen - ");
        exit(1);
    }
    printf("OK: Socket di ascolto creato. fd: %d\n\n", listening_socket);

    /* Azzeramento e inizializzaizione dei set (princpale e di appoggio) */
    FD_ZERO(&master);
    FD_ZERO(&readfds);//svuoto

    FD_SET(listening_socket, &master);//aggiungo socket di ascolto ed il descrittore di file standard input (STDIN_FILENO) al set di descrittori master. Questo permette al server di leggere anche i comandi dalla console.
    FD_SET(STDIN_FILENO, &master);
    fdmax = listening_socket;//questo è il max descrittore di file del set

    printf("******************* SERVER AVVIATO *******************\n"
           "Comandi disponibili:\n"
           "\tstop\ttermina il server\n"
           "******************************************************\n\n");
    
    for(;;) 
    {
        /* printf("for infinito\n"); */
        memset(buffer, 0, DIM_BUFFER);//azzero il buffer
        readfds = master;//copio il set master in readfds
        select(fdmax + 1, &readfds, NULL, NULL, NULL);//monitoro i descrittori di file per vedere se sono pronti per la lettura

        for(i = 0; i <= fdmax; i++) {
            /* printf("for fdmax\n"); */
            if(FD_ISSET(i, &readfds)) {//controllo se l'i-esimo descrittore di file è pronto per la lettura (cioè se è rimasto nel set dopo la select)
                
/* verifica se il descrittore pronto per la lettura è lo standard input (STDIN_FILENO) per ricezione comandi diretti al server */
                if(i == STDIN_FILENO) {
                    memset(buffer, 0, DIM_BUFFER);//svuoto il buffer per poi leggere il comando
                    fgets(buffer, DIM_BUFFER, stdin);//legge una riga di input dallo standard input (console) e la memorizza nel buffer.

                    printf("\n******************************************************\n");
                    if(strcmp(buffer, "stop\n") != 0) {//Se il comando non è "stop\n", stampa un messaggio di avviso e continua con il prossimo ciclo del loop.
                        printf("WARN: Comando non riconosciuto.\n"
                            "******************************************************\n\n");
                        continue;
                    }
                    printf("! Chiusura del server...\n\n");
                    if(gameOn() == true) { /* se è presente una partita non termino */
                        printf("WARN: Impossibile terminare il server, il gioco è in corso.\n"
                                "******************************************************\n\n");
                        continue;
                    }
                    deleteUsers(); /* basta eliminare gli utenti perchè se siamo arrivati qui le sessioni sono già state deallocate */
                    close(listening_socket);//chiudo socket, svuoto set
                    FD_ZERO(&master);
                    FD_ZERO(&readfds);
                    printf("OK: Server chiuso correttamente\n"
                            "******************************************************\n");
                    exit(0);
                }
                /* Controllo il socket di ascolto, addetto alle nuove connessioni*/
                else if(i == listening_socket) {//se ad esser pronto è il socket di ascolto
                    addrlen = sizeof(cl_addr);//dimensione idnirizzo client
                    memset(&cl_addr, 0, sizeof(cl_addr));//svuoto
                    communication_socket = accept(listening_socket, (struct sockaddr*)&cl_addr, &addrlen);//accetta la nuova connessione in entrata e crea un nuovo socket di comunicazione 
                    
                    if(communication_socket < 0) {
                        perror("ERR: Errore accept");
                        exit(1);
                    }
                    printf("******************************************************\n"
                           "Nuovo client connesso, socket: %d\n"
                           , communication_socket);

                    /* invio al client gli scenari disponibili */
                    memset(buffer, 0, DIM_BUFFER);//svuoto il buffer per inviare gli scenari tramite la funzione instruction
                    instruction(buffer);
                    send(communication_socket, buffer, DIM_BUFFER, 0);//invio il buffer al client
                    printf("Risposta server: scenari disponibili inviati al client.\n"
                           "******************************************************\n\n");
                    
                    FD_SET(communication_socket, &master);//aggiungo il socket di com. appena creato al set master
                    if(communication_socket > fdmax) {
                        fdmax = communication_socket;//nel caso aggiorno il descrittore max
                    }
                }
                /* Altrimenti, se non è nessuno dei socket precedenti, è quello di comunicazione */
                else {
                    memset(buffer, 0, DIM_BUFFER);//svuoto il buffer
                    ret = recv(i, (void*)buffer, DIM_BUFFER, 0);//leggo il messaggio dal client e lo memorizzo nel buffer
                    current_session = getSession(i, type);//cerco la sessione a cui appartiene il socket, e ne memorizzo il tipo di sessione in type (il tipo indica se giocatore principale o secondario)
                    
                    /* potrebbe returnare NULL se non trova niente (qualsiasi cosa prima di aver fatto comando start)*/

                    if(ret == 0) {//client ha chiuso la connessione... 2 casi: non è in sessione o è in sessione


                        /* se il client ancora non è entrato in nessuna sessione */
                        if(current_session == NULL) {
                            printf("******************************************************\n"
                              "! Sconnessione socket %d in corso...\n", i);
                            logout(findUserFromSocket(i)); /* non essendo loggato */
                            close(i);  
                            FD_CLR(i, &master);
                            printf("Socket %d chiuso.\n"
                                    "Il socket non faceva parte di nessuna partita.\n", i);
                            printf("SUCCES: Disconessione eseguita con successo\n"
                                "******************************************************\n\n");
                            continue;
                        }


                        /* 
                            se invece il client faceva parte di una sessione si disconnette il socket e 
                            si controlla il tipo di utente in modo da gestire la chiusura aggiuntiva del socket secondario.
                        */
                        current_user = strcmp(type, "MAIN") == 0 ? current_session->main : current_session->secondary;
                        printf("******************************************************\n"
                              "! Sconnessione socket %d in corso...\n"
                              "! Eliminazione utente: %s\n", i, current_user->username);
                        logout(current_user);
                        close(i);
                        printf("Socket %d chiuso.\n", i);
                        FD_CLR(i, &master);
                        printf("Socket %d rimosso dal set dei descrittori.\n"
                                "OK: Disconnessione client eseguita con successo.\n\n", i);
                               
                        /* se il client è principale va disconnesso anche il client secondario*/
                        if(strcmp(type, "MAIN") == 0) {
                            if(current_session->secondary) {//se è attivo il giocatore secondario...
                                printf("Disconessione altro giocatore...\n"
                                        "! Sconnessione socket %d in corso...\n"
                                        "! Eliminazione utente: %s\n", current_session->secondary->socket, current_session->secondary->username);
                                close(current_session->secondary->socket);
                                printf("OK: Socket %d chiuso.\n", current_session->secondary->socket);
                                FD_CLR(current_session->secondary->socket, &master);
                                printf("OK: Socket %d rimosso dal set dei descrittori.\n"
                                        "OK: Disconnessione client secondario eseguita con successo.\n\n", current_session->secondary->socket);
                                printf("! Eliminazione della sessione\n");
                                logout(current_session->secondary); 
                            }
                            deleteSession(current_session->id);
                            printf("OK: Sessione eliminata con successo\n");
                        }
                        printf("SUCCES: Disconessione eseguita con successo\n"
                                "******************************************************\n\n");
                        continue;
                    }
//stampa info relative al comando ricevuto:                  
                    printf("******************************************************\n"
                          "Comando: %s\n"
                          "Sessione: %d\n"
                          "Scenario: %s\n"
                          "Utente: %s\n"
                          "Socket: %d\n"
                          "Risposta server: "
                          ,buffer, 
                          current_session ? current_session->id : -1,
                          current_session ? current_session->set.name : "non in sessione", 
                          current_session ? (strcmp(type, "MAIN") == 0 ? current_session->main->username : current_session->secondary->username) : "non in sessione", 
                          i); 
                    commandSwitcher(i, buffer, type, current_session, &master);//serve a gestire i comandi ricevuti dai client e indirizzarli ai rispettivi handler
                    printf("\n******************************************************\n\n");
                }
            }
        } 
    }
}