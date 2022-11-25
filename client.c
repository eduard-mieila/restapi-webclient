#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

// Date server
#define HOST "34.118.48.238"
#define PORT 8080

// Rute catre care se trimit mesajele HTTP
#define REG_ROUTE "/api/v1/tema/auth/register"
#define LOGIN_ROUTE "/api/v1/tema/auth/login"
#define LOGOUT_ROUTE "/api/v1/tema/auth/logout"
#define ENTER_ROUTE "/api/v1/tema/library/access"
#define GET_BOOKS_ROUTE "/api/v1/tema/library/books"
#define ADD_BOOK_ROUTE "/api/v1/tema/library/books"

int main(int argc, char *argv[]) {
    char *message;
    char *response;
    int sockfd, loggedIn = 0;
    char username[USERLEN], password[USERLEN];
    char stdin_buffer[BUFLEN];
    char session_cookie[BUFLEN];
    char jwt_token[BUFLEN];
    char trash[BUFLEN];
 
    memset(stdin_buffer, 0, BUFLEN);
    memset(session_cookie, 0, BUFLEN);
    memset(jwt_token, 0, BUFLEN);

    while (1) {
        // Citire comanda de la tastatura
        memset(stdin_buffer, 0, BUFLEN);
        fgets(stdin_buffer, BUFLEN, stdin);
        // scanf("%s", stdin_buffer);

        // Deschidere socket
        // Din cauza ca legatura cu serverul ramane activa doar 5 secunde,
        // vom deschide o conexiune de fiecare data cand se primeste o comanda
        // si o vom inchide dupa ce s-a primit un raspuns
        sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

        if (strncmp(stdin_buffer, "register", 8) == 0) {
            // Inregistrarea unui cont - register
            // Citire date utilizator
            memset(username, 0 , USERLEN);
            memset(password, 0 , USERLEN);
            printf("username=");
            scanf("%s", username);
            printf("password=");
            scanf("%s", password);
            
            // Compunere mesaj JSON
            char jsonString[BUFLEN];
            sprintf(jsonString, "{\"username\":\"%s\", \"password\":\"%s\"}",
                                username, password);
            

            // Compunere cerere POST HTTP
            char **data = calloc(1, sizeof(char*));
            data[0] = jsonString;
            message = compute_post_request(HOST, REG_ROUTE, "application/json",
                                data, 1, NULL, 0);


            // Trimite cerere catre server; asteapta raspuns
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            
            // Citeste date despre raspuns
            int errCode = 0;
            char prot[50], errDesc[100];
            sscanf(response, "%s %d %[^\n]", prot, &errCode, errDesc);

            // Verifica raspunsul
            if (errCode == 201) {    
                // Daca s-a primit CREATED, afiseaza mesaj de confirmare
                printf("> %s registered! \n", username);
            } else {
                // Daca s-a primit alt cod de eroare, parseaza mesajul,
                // afiseaza detalii.
                char *pnt = strstr(response, "{");
                JSON_Value *jsonResponse = json_parse_string(pnt);
                DIE(jsonResponse == NULL, "register JSON Val parse error");
                
                JSON_Object *jsonObjResponse = json_object(jsonResponse);
                DIE(jsonObjResponse == NULL, "register JSON Obj parse error");

                printf("> Registration failed! %s\n",
                        json_object_get_string(jsonObjResponse, "error"));

                // Elibereaza memoria ocupata de obiectele JSON
                json_object_clear(jsonObjResponse);
                json_value_free(jsonResponse);
            }
            
            // Elibereaza memorie
            free(data);
            free(message);
            free(response);

        } else if (strncmp(stdin_buffer, "login", 5) == 0) {
            // Autentificare - login
            // Citeste username si password utilizator
            memset(username, 0 , USERLEN);
            memset(password, 0 , USERLEN);
            printf("username=");
            scanf("%s", username);
            printf("password=");
            scanf("%s", password);
            
            // Genereaza mesajul JSON ce urmeaza sa fie trimis catre server
            char jsonString[BUFLEN];
            sprintf(jsonString, "{\"username\":\"%s\", \"password\":\"%s\"}",
                                username, password);


            // Compune cerere POST HTTP
            char **data = calloc(1, sizeof(char*));
            data[0] = jsonString;
            message = compute_post_request(HOST, LOGIN_ROUTE, "application/json",
                                data, 1, NULL, 0);


            // Trimite cererea catre server; asteapta raspuns
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            
            // Citeste date despre raspuns
            int errCode = 0;
            char prot[50], errDesc[100];
            sscanf(response, "%s %d %[^\n]", prot, &errCode, errDesc);


            if (errCode == 200) {
                // s-a primit OK
                // Seteaza loggedIn pe 1
                loggedIn = 1;
                
                // Gaseste Set-Cookie, salveaza-l
                char *pnt = strstr(response, "Set-Cookie: ");
                memset(session_cookie, 0, BUFLEN);
                sscanf(pnt, "%s %s", trash, session_cookie);
                session_cookie[strlen(session_cookie) - 1] = 0;

                // Afiseaza mesaj de confirmare
                printf("> Hi, %s!\n", username);
            } else {
                // Daca s-a primit alt cod de eroare, parseaza mesajul,
                // afiseaza detalii.
                char *pnt = strstr(response, "{");
                JSON_Value *jsonResponse = json_parse_string(pnt);
                DIE(jsonResponse == NULL, "login JSON Val parse error");

                JSON_Object *jsonObjResponse = json_object(jsonResponse);
                DIE(jsonObjResponse == NULL, "login JSON Obj parse error");

                printf("> Log-in failed! %s\n",
                        json_object_get_string(jsonObjResponse, "error"));

                // Elibereaza memoria ocupata de obiectele JSON
                json_object_clear(jsonObjResponse);
                json_value_free(jsonResponse);
            }

            // Elibereaza memoria
            free(data);
            free(message);
            free(response);

        } else if (strncmp(stdin_buffer, "enter_library", 13) == 0) {
            // Cerere de acces in biblioteca - enter_library
            // Daca utilizatorul nu a efectuat inainte login, ignora comanda
            if (!loggedIn) {
                printf("> In order to enter the library you need to log in!\n");
                printf("> Type login and enter your credentials.\n");
                continue;
            }
            
            // Pune session_cookie in cookies-urile pentru cererea de GET
            char **cookies = calloc(1, sizeof(char*));
            cookies[0] = calloc(2 * BUFLEN, sizeof(char));
            sprintf(cookies[0], "Cookie: %s", session_cookie);

            // Compune cerere GET HTTP
            message = compute_get_request(HOST, ENTER_ROUTE, NULL, cookies, 1);

            // Trimite cererea si asteapta raspuns
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);


            // Citeste date despre raspuns
            int errCode = 0;
            char prot[50], errDesc[100];
            sscanf(response, "%s %d %[^\n]", prot, &errCode, errDesc);
            
            // Parseaza mesajul primit
            // In acest caz, deoarece mesajul va contine cu siguranta un obiect
            // JSON, parsarea acestuia se va face mai devreme, comparativ cu
            // cazurile anterioare
            char *pnt = strstr(response, "{");
            JSON_Value *jsonResponse = json_parse_string(pnt);
            DIE(jsonResponse == NULL, "enter_library JSON Val parse error");

            JSON_Object *jsonObjResponse = json_object(jsonResponse);
            DIE(jsonObjResponse == NULL, "enter_library JSON Obj parse error");
            
            if (errCode == 200) {
                // Daca s-a primit OK, salvam tokenul
                memset(jwt_token, 0, BUFLEN);
                strcpy(jwt_token, json_object_get_string(jsonObjResponse, "token"));
                // Afisam mesaj de confirmare
                printf("> Access granted!\n");
            } else {
                // Daca s-a primit un alt mesaj de eroare, afisam detalii despre
                // acasta eroare
                printf("> Access denied! %s\n",
                            json_object_get_string(jsonObjResponse, "error"));
            }

            // Elibereaza memoria ocupata de obiectele JSON
            json_object_clear(jsonObjResponse);
            json_value_free(jsonResponse);

            // Eliberam memoria
            free(cookies[0]);
            free(cookies);
            free(message);
            free(response);

        } else if (strncmp(stdin_buffer, "get_books", 9) == 0) {
            // Vizualizarea informatiilor sumare despre toate cartile - get_books
            // Verifica daca exista un s-a cerut acces in biblioteca
            if (jwt_token[0] == 0) {
                printf("> Error! You need access to the library in order to see the books!\n");
                continue;
            }

            // Pune JWT Token in parametrii pentru cererea de POST
            // pentru a demonstra ca user-ul are acces la biblioteca
            char **header = calloc(1, sizeof(char*));
            header[0] = calloc(2*BUFLEN, sizeof(char));
            sprintf(header[0], "Authorization: Bearer %s", jwt_token);

            // Compune cerere GET HTTP
            message = compute_get_request(HOST, GET_BOOKS_ROUTE, NULL,
                                                header, 1);


            // Trimite cererea si asteapta raspuns
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);


            // Citeste date despre raspuns
            int errCode = 0;
            char prot[50], errDesc[100];
            sscanf(response, "%s %d %[^\n]", prot, &errCode, errDesc);
            
            if (errCode == 200) {
                // Daca s-a primit ok, parseaza vectorul JSON primit, apoi
                // printeaza datele despre fiecare carte.
                char *pnt = strstr(response, "[");
                JSON_Value *jsonResponse = json_parse_string(pnt);
                DIE(jsonResponse == NULL, "get_books JSON Val parse error");

                JSON_Array *jsonArray = json_value_get_array(jsonResponse);
                DIE(jsonArray == NULL, "get_books JSON Array parse error");

                printf("*****************************************\n");
                if (json_array_get_count(jsonArray) != 0) {
                    printf("%-5.5s     %s\n", "ID", "Title");
                    for (int i = 0; i < json_array_get_count(jsonArray); i++) {
                        JSON_Object *currentBook = json_array_get_object(jsonArray, i);
                        DIE(currentBook == NULL, "get_books JSON Book parse error");
                        printf("%5d     %s\n",
                                (int)json_object_get_number(currentBook, "id"),
                                json_object_get_string(currentBook, "title"));
                    }
                } else {
                    printf("No books in the library!\n");
                }
                printf("*****************************************\n");

                // Elibereaza memoria ocupata de obiectele JSON
                json_array_clear(jsonArray);
                json_value_free(jsonResponse);
            } else {
                // Daca s-a primit un alt tip de eroare, afiseaza detalii despre
                // aceasta
                char *pnt = strstr(response, "{");
                JSON_Value *jsonResponse = json_parse_string(pnt);
                DIE(jsonResponse == NULL, "get_books JSON Val parse error");

                JSON_Object *jsonObjResponse = json_object(jsonResponse);
                DIE(jsonObjResponse == NULL, "get_books JSON Obj parse error");

                printf("Error! %s\n", json_object_get_string(jsonObjResponse, "error"));

                // Elibereaza memoria ocupata de obiectele JSON
                json_object_clear(jsonObjResponse);
                json_value_free(jsonResponse);
            }

            // Elibereaza memoria
            free(header[0]);
            free(header);
            free(message);
            free(response);

        } else if (strncmp(stdin_buffer, "get_book", 8) == 0) {
            // Vizualizarea detaliilor despre o carte - get_book
            // Verifica daca exista un s-a cerut acces in biblioteca
            if (jwt_token[0] == 0) {
                printf("> Error! You need access to the library in order to see a book!\n");
                continue;
            }

            // Citeste id-ul cartii despre care dorim sa aflam detalii
            unsigned int id;
            printf("id=");
            scanf("%u", &id);

            // Compune ruta pe care se va trimite cererea HTTP
            char route[BUFLEN];
            sprintf(route, "%s/%d", GET_BOOKS_ROUTE, id);

            // Pune JWT Token in parametrii pentru cererea de POST
            // pentru a demonstra ca user-ul are acces la biblioteca
            char **header = calloc(1, sizeof(char*));
            header[0] = calloc(2 * BUFLEN, sizeof(char));
            sprintf(header[0], "Authorization: Bearer %s", jwt_token);

            // Compune cerere GET HTTP
            message = compute_get_request(HOST, route, NULL, header, 1);

            // Trimite cererea si asteapta raspuns
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);


            // Citeste date despre raspuns
            int errCode = 0;
            char prot[50], errDesc[100];
            sscanf(response, "%s %d %[^\n]", prot, &errCode, errDesc);
            
            // Parseaza mesajul primit
            // In acest caz, deoarece mesajul va contine cu siguranta un obiect
            // JSON, parsarea acestuia se va face mai devreme, comparativ cu
            // cazurile anterioare
            char *pnt = strstr(response, "{");
            JSON_Value *jsonResponse = json_parse_string(pnt);
            DIE(jsonResponse == NULL, "get_book JSON Val parse error");

            JSON_Object *jsonObjResponse = json_object(jsonResponse);
            DIE(jsonObjResponse == NULL, "get_books JSON Obj parse error");

            if (errCode == 200) {
                // Daca s-a primit OK, afiseaza detaliile despre cartea ceruta
                printf("*****************************************\n");
                printf("    Title: %s\n", json_object_get_string(jsonObjResponse, "title"));
                printf("   Author: %s\n", json_object_get_string(jsonObjResponse, "author"));
                printf("Publisher: %s\n", json_object_get_string(jsonObjResponse, "publisher"));
                printf("    Genre: %s\n", json_object_get_string(jsonObjResponse, "genre"));
                printf("    Pages: %d\n", (int)json_object_get_number(jsonObjResponse, "page_count"));
                printf("*****************************************\n");

            } else {
                // Daca s-a primit un mesaj de eroare, afiseaza detalii despre acesta
                printf("> Error! %s\n", json_object_get_string(jsonObjResponse, "error"));
            }

            // Elibereaza memoria ocupata de obiectele JSON
            json_object_clear(jsonObjResponse);
            json_value_free(jsonResponse);

            // Elibereaza memoria
            free(header[0]);
            free(header);
            free(message);
            free(response);

        } else if (strncmp(stdin_buffer, "add_book", 8) == 0) {
            // Adaugarea unei carti - add_book
            // Verifica daca exista un s-a cerut acces in biblioteca
            if (jwt_token[0] == 0) {
                printf("> Error! You need access to the library in order to add a book!\n");
                continue;
            }
            
            // Citeste datele despre carte care urmeaza sa fie introdusa
            char title[STR_LEN], author[STR_LEN], genre[STR_LEN], publisher[STR_LEN];
            unsigned int page_count;
            char jsonBook[BUFLEN];
            printf("title=");           fgets(title, STR_LEN - 1, stdin);
            title[strlen(title) - 1] = 0;
            printf("author=");          fgets(author, STR_LEN - 1, stdin);
            author[strlen(author) - 1] = 0;
            printf("genre=");           fgets(genre, STR_LEN - 1, stdin);
            genre[strlen(genre) - 1] = 0;
            printf("publisher=");       fgets(publisher, STR_LEN - 1, stdin);
            publisher[strlen(publisher) - 1] = 0;
            printf("page_count=");      scanf("%u", &page_count);
            
            // Compune obiectul JSON
            sprintf(jsonBook,
                    "{\"title\":\"%s\", \"author\":\"%s\", \"genre\":\"%s\", \"page_count\":%u, \"publisher\":\"%s\"}",
                    title, author, genre, page_count, publisher);
            
            // Pune JWT Token in parametrii pentru cererea de POST
            // pentru a demonstra ca user-ul are acces la biblioteca
            char **cookies = calloc(1, sizeof(char*));
            cookies[0] = calloc(2 * BUFLEN, sizeof(char));
            sprintf(cookies[0], "Authorization: Bearer %s", jwt_token);
            
            // Pune obiectul JSON in cererea HTTP
            char **header = calloc(1, sizeof(char*));
            header[0] = jsonBook;

            // Compune cerere POST HTTP
            message = compute_post_request(HOST, ADD_BOOK_ROUTE, "application/json",
                                            header, 1, cookies, 1);


            // Trimite cererea si asteapta raspuns
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);


            // Citeste date despre raspuns
            int errCode = 0;
            char prot[50], errDesc[100];
            sscanf(response, "%s %d %[^\n]", prot, &errCode, errDesc);
            
            if (errCode == 200) {
                // Daca s-a primit OK, afiseaza mesaj de confirmare
                printf("> Book %s by %s added to library!\n", title, author);
            } else {
                // Daca s-a primit un alt mesaj de eroare, afiseaza detalii
                // despre aceasta
                char *pnt = strstr(response, "{");
                JSON_Value *jsonResponse = json_parse_string(pnt);
                DIE(jsonResponse == NULL, "add_book JSON Val parse error");

                JSON_Object *jsonObjResponse = json_object(jsonResponse);
                DIE(jsonObjResponse == NULL, "add_books JSON Obj parse error");

                printf("> Error while adding book %s by %s!", title, author);
                printf(" %s\n", json_object_get_string(jsonObjResponse, "error"));

                // Elibereaza memoria ocupata de obiectele JSON
                json_object_clear(jsonObjResponse);
                json_value_free(jsonResponse);
            }

            // Elibereaza memoria
            free(cookies[0]);
            free(cookies);
            free(header);
            free(message);
            free(response);
        } else if (strncmp(stdin_buffer, "delete_book", 11) == 0) {
            // Stergerea unei carti - delete_book
            // Verifica daca exista un s-a cerut acces in biblioteca
            if (jwt_token[0] == 0) {
                printf("> Error! You need access to the library in order to delete a book!\n");
                continue;
            }
            
            // Citeste id-ul cartii care urmeaza sa fie stearsa
            unsigned int id;
            printf("id=");
            scanf("%u", &id);

            // Compune ruta catre care urmeaza sa fie trimisa cererea HTTP
            char route[BUFLEN];
            sprintf(route, "%s/%u", GET_BOOKS_ROUTE, id);

            // Pune JWT Token in parametrii pentru cererea de POST
            // pentru a demonstra ca user-ul are acces la biblioteca
            char **header = calloc(1, sizeof(char*));
            header[0] = calloc(2 * BUFLEN, sizeof(char));
            sprintf(header[0], "Authorization: Bearer %s", jwt_token);

            // Compune cerere GET HTTP
            message = compute_delete_request(HOST, route, NULL, header, 1);


            // Trimite cererea si asteapta raspuns
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);


            // Citeste date despre raspuns
            int errCode = 0;
            char prot[50], errDesc[100];
            sscanf(response, "%s %d %[^\n]", prot, &errCode, errDesc);

            if (errCode == 200) {
                // Daca s-a primit ok, afiseaza mesaj de confirmare
                printf("> Book no. %d deleted from library!\n", id);
            } else {
                // Daca s-a primit un alt tip de eroare, afiseaza detalii despre
                // aceasta
                char *pnt = strstr(response, "{");
                JSON_Value *jsonResponse = json_parse_string(pnt);
                DIE(jsonResponse == NULL, "delete_book JSON Val parse error");

                JSON_Object *jsonObjResponse = json_object(jsonResponse);
                DIE(jsonObjResponse == NULL, "delete_book JSON Obj parse error");

                printf("> Error! %s\n", json_object_get_string(jsonObjResponse, "error"));

                // Elibereaza memoria ocupata de obiectele JSON
                json_object_clear(jsonObjResponse);
                json_value_free(jsonResponse);
            }

            // Elibereaza memoria
            free(header[0]);
            free(header);
            free(message);
            free(response);

        } else if (strncmp(stdin_buffer, "logout", 6) == 0) {
            // Deconectare - logout
            // Verifica daca cineva s-a logat, altfel actiunea de logout nu are sens
            if (!loggedIn) {
                printf("> In order to log out you need to log in!\n");
                continue;
            }

            // Pune session_cookie in cookies-urile pentru cererea de GET
            // pentru a demonstra ca user-ul este autentificat
            char **cookies = calloc(1, sizeof(char*));
            cookies[0] = calloc(2 * BUFLEN, sizeof(char));
            sprintf(cookies[0], "Cookie: %s", session_cookie);

            // Compune cerere GET HTTP
            message = compute_get_request(HOST, LOGOUT_ROUTE, NULL, cookies, 1);


            // Trimite cererea si asteapta raspuns
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Citeste date despre raspuns
            int errCode = 0;
            char prot[50], errDesc[100];
            sscanf(response, "%s %d %[^\n]", prot, &errCode, errDesc);
            
            if (errCode == 200) {
                // Daca s-a primit OK, seteaza loggedIn pe 0
                loggedIn = 0;

                // Sterge session-cookie si JWT-Token
                memset(session_cookie, 0 , BUFLEN);
                memset(jwt_token, 0 , BUFLEN);

                // Afiseaza mesaj de confirmare
                printf("> Goodbye, %s!\n", username);
            } else {
                // Daca s-a primit o eroare,a fiseaza detalii despre aceasta
                char *pnt = strstr(response, "{");
                JSON_Value *jsonResponse = json_parse_string(pnt);
                DIE(jsonResponse == NULL, "logout JSON Val parse error");

                JSON_Object *jsonObjResponse = json_object(jsonResponse);
                DIE(jsonObjResponse == NULL, "logout JSON Obj parse error");

                printf("> Log-out failed! %s\n",
                        json_object_get_string(jsonObjResponse, "error"));

                // Elibereaza memoria ocupata de obiectele JSON
                json_object_clear(jsonObjResponse);
                json_value_free(jsonResponse);
            }

            // Elibereaza memoria
            free(cookies[0]);
            free(cookies);
            free(message);
            free(response);

        } else if (strncmp(stdin_buffer, "exit", 4) == 0) {
            // Inchiderea programului - exit
            // Inchide conexiunea, paraseste executia
            close_connection(sockfd);
            break;
        }

        // Verifica daca s-a introdus o comanda gresita
        check_wrong_command(stdin_buffer);

        // Inchide conexiunea
        close_connection(sockfd);
    }
    
    return 0;
}
