# ipk-projekt1


## Zadanie
Vytvorit **server** v jazyku C/C++ komunikujuci prostrednictvom protokolu **HTTP**, ktory bude poskytovat rozne informacie o systeme. Server bude nasluchat (`listen`) na zadanom porte a podla url bude vracat pozadovane informacie. Server musi *spravne spracovavat hlavicky HTTP* a *vytvarat spravne HTTP odpovede*. Typ odpovede bude `text/plain`. Komunikacia so serverom by mala byt mozna pomocou **web browseru** a aj nastrojmi **wget** a **curl**. Nutnostou je spustenie na *Linux Ubuntu 20.04*

Server bude prelozitelny pomocou `Makefile`, ktory vytvori spustitelny subor `hinfosvc`.
Server bude spustitelny s argumentom oznacujucim lokalny `port`, na ktorom bude nasluchat poziadavkam.

`./hinfosvc 12345`

Server bude mozne ukoncit pomocou `CTRL+C`.
Server bude vediet spracovat nasledujuce 3 typy dotazov, ktore su na server zaslane pomocou prikazu `GET`:
  1. Ziskanie domenoveho mena:
     - napr. `GET http://servername:12345/hostname` vrati `merlin.fit.vutbr.cz`
  2. Ziskanie informacii o CPU:
     - napr. `GET http://servername:12345/cpu-name` vrati `Intel(R) Xeon(R) CPU E5-2640 0 @ 2.50GHz`
  3. Aktualnu zataz:
     - napr. `GET http://servername:12345/load` vrati `65%`
  
  ### Kde v systeme ziskat potrebne informacie?
    prikaz `uname`
    prikaz `lscpu`
    zo suboru v adresari `/proc`
    

## Implementacia
  je **nutne** vyuzit kniznicu socketov: `sys/socket.h`
  je **nepripustne** vyuzivat kniznice na spracovanie HTTP a pod. (ciel je minimum zavislosti)

Sucastou projektu je aj **dokumentacia**, ktoru predstavuje subor `Readme.md` a obsahuje:
  - strucny **popis** projektu
  - **sposob spustenia** projektu
  - **priklady pouzitia** projektu

  - nezabudnut uviest autora, sposob vytvorenia aplikacie, ukazkove priklady, dalsie inf...
  - vid. napr https://github.com/me-and-company/readme-template
