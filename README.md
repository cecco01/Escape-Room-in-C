# Progetto di Reti Informatiche

> Progetto per il corso Reti Informatiche A.A. 23/24 dell'Università di Pisa

## Descrizione

Il progetto consiste nella realizzazione di una applicazione client-server.
Il file di documentazione è volutamente sintetico perchè il professore ha richiesto che fosse di massimo due pagine, per vedere le richieste fatte del professore consultare il documento "analisi-progetto".

## Avvio

Per avviare è sufficiente scrivere da terminale:

```bash
~$ make
~$ ./exec2024.sh 
```

Il comando make lo useremo per compilare, mentre l'esecuzione dello script permette, con un solo comando, di avviare il server e i due client.

## Comandi escape room:

* start *room* *type*
* look [*object* | *location*]
* take *object*
* use *object* [*object*] 
* objs 
* end
* stop (Solo per il server)

## Tema

Era necessario implementare almeno uno scenario per l'escape room, per il tema dello scenario ho deciso di ispirarmi al film "Una notte al museo".

### Museum

Dopo aver fatto il login e aver scelto questo scenario si è davanti ad una scelta, si può partecipare come prigioniero del guardiano del museo e provare a scappare entro il tempo limite oppure si può partecipare come guardiano del museo corrotto ([vedere client aggiuntivo](#client-aggiuntivo)).

#### Prigioniero

Sei in prigione. Ti trovi sul \+\+letto\+\+, davanti a te c'è uno \+\+scaffale\+\+ e una \+\+finestra\+\+. Mentre dall'altro lato della stanza c'è il tuo compagno di stanza, \*\*Roosvelt\*\*. 

* **look letto**: sotto il letto hai trovato un \*\*walkie-talkie\*\* e delle \*\*diamante1\*\*

    * walkie-talkie: Puoi usare il walkie-talkie per chiamare il guardiano corrotto, ovvero il client aggiuntivo, ma se il client non si è connesso non risponde nessuno al walkie-talkie. Altrimenti il client aggiuntivo risponde chiedendo un numero di diamanti a piacere
        
        * look walkie-talkie: É il walkie-talkie usato dai custodi del museo.

        * use walkie-talkie: Qualcuno mi sente?.... Biiiip. Biiiip. (Qui il gioco cambia a seconda della presenza del guardiano corrotto)

        * take walkie-talkie                        

    * diamante1

        * look diamante1: È un diamante di notevoli dimensioni.

        * use diamante1: Per quanto possa essere duro, non mi basterà per tagliare le sbarre.

        * take diamante1   

* **look Roosvelt**: Roosvelt Roosvelt forse può darti un consiglio su come uscire di qui. Però ti aiuterà solo a patto di rispondere ad un indovinello.

    * Vediamo se sei stato attento. Sono di andata e di ritorno, chi sono? 

        Costruire una bomba è molto facile, hai provato a combinare del sapone e delle batterie?

* **look scaffale**: nello scaffale c'è una \*\*scatola\*\* e delle \*\*diamante2\*\*.

    * look scatola: La scatola è bloccata da un lucchetto a codice. (use scatola) Il codice del lucchetto è il continuo di questa sequenza
        11
        21
        1211
        111221

        dentro la scatola c'è del \*\*sapone\*\* e altre \*\*diamante3\*\*.

        * sapone

            * use sapone: Ancora non è il momento di lavarsi.

            * take sapone (implicito)

            * use sapone walkie-talkie (o use walkie-talkie sapone): Hai smontato il walkie-talkie è hai preso la batteria. Tramite la batteria e il sapone hai costruito una \*\*bomba\*\*

                * take bomba (implicito)

                * use bomba sbarre (o use sbarre bomba): Complimenti sei riuscito ad evadere!

        * diamante3

            * look diamante3: È un diamante di notevoli dimensioni.

            * use diamante3: Fun fact: la parola diamante viene dal greco “adamas” che significa “invincibile”, suggerisce l'eternità dell'amore.

            * take diamante3 (implicito)

    * diamante2

        * look diamante2: È un diamante di notevoli dimensioni.

        * use diamante2: Quanti carati sarà?

        * take diamante2

* **look finestra**: la finestra ha le \*\*sbarre\*\*.

    * take sbarre: non si possono prendere

    * use bomba sbarre (o use sbarre bomba): Complimenti sei riuscito ad evadere!

#### Client aggiuntivo

Il client aggiuntivo è un secondo partecipante alla escape room che ha il ruolo di guardiano corrotto. Ti fa uscire se hai abbastanza diamanti, egli può decidere quanto costa l'uscita da un minimo di 1 e max di 3.

Il client aggiuntivo viene inserito nella prima partita disponibile.

#### Soluzioni Enigmi

1. Quiz di Roosvelt: Anna.

2. Lucchetto: 312211

#### Soluzioni Escape Room

Ci sono due possibli soluzioni, per la seconda soluzione è necessario che si connetta il client aggiuntivo

1. Per uscire dalla prigione è necessario far "esplodere" le sbarre alla finestra. Per poterlo fare è necessario creare \*\*bomba\*\* e usarla con \*\*sbarre\*\*.
Per creare \*\*bomba\*\* è necessario usare \*\*walkie-talkie\*\* e il \*\*sapone\*\*.

2. Il client aggiuntivo ha lo scopo di guardiano Corrotto che ti può far uscire in cambio di un certo numero di diamanti. Usando il \*\*walkie-talkie\*\*, se il client è connesso può chiedere un numero di \*\*diamanti\*\* per far uscire il prigioniero. Se il client principale (prigioniero) ha il numero di diamanti necessari allora la partita finisce.
