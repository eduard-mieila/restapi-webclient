# Client Web REST API

## Descriere generală
Clientul rulează în linia de comandă și are ca scop autentificarea la un REST Server ce simulează o bibliotecă. Utilizatorul se poate înregistra, conecta(solicita un Session-Cookie) și deconecta de la bilioteca, poate solicita acces la date(solictă un token JWT), adăuga și sterge cărți și poate afișa informații despre o anume carte sau despre toate cărțile pe care le-a adăugat la bibliotecă. Implementarea s-a efectuat în fișierul client.c și s-a folosit biblioteca standard C, socketi Linux și biblioteca Parson pentru parsarea mesajelor JSON primite de la server.

## How to use
### Build
    make client

### Run
    ./client
    # sau
    make run

### Comenzi posibile
Register:
    
    register
    > username=something
    > password=something

---
Login:
    
    login
    > username=something
    > password=something

---
Enter library:

    enter_library

---
Get books:

    get_books

---
Get book:

    get_book
    > id=10

---
Add book:

    add_book
    > title=Testbook
    > author=student
    > genre=comedy
    > publisher=John
    > page_count=10

---
Delete book:

    delete_book
    > id=10

---
Logout:

    logout

---
Exit:

    exit

## Informații suplimentare despre implementare
Clientul Web functioneaza astfel:
1. Se citeste o comanda de la tastatura
2. Se deschide o conexiune catre server
3. Se verifica daca s-a primit o comanda ce trebuie executata si se compun
   mesajele necesare
4. Se trimite cererea catre server si se asteapta raspuns
5. Se parseaza raspunsul
6. Se elibereaza memoria
7. Se inchide conexiunea cu serverul

Daca nu s-a citit o comanda cunoscuta de client, afisam un mesaj corespunzator.

### Comanda register:
- preia username-ul si parola
- compune pe rand mesajul JSON, cererea HTTP
- se trimite cererea, se asteapta raspuns
- se parseaza raspunsul; daca se primeste CREATED(201), se afiseaza un mesaj
  de confirmare, altfel se parseaza eroarea si se afiseaza descrierea acesteia
- se elibereaza memoria

### Comanda login:
- preia username-ul si parola
- compune pe rand mesajul JSON, cererea HTTP
- se trimite cererea, se asteapta raspuns
- se parseaza raspunsul; daca se primeste OK(200), se actualizeaza varaibila
  loggedIn(pentru a sti pe viitor daca acest client a fost anterior conectat la
  server), preia Session-Cookie si il pastreaza, afiseaza un mesaj de confirmare, 
  altfel se parseaza eroarea si se afiseaza descrierea acesteia
- se elibereaza memoria

### Comanda enter_library:
- verifica daca utilizatorul s-a logat anterior; daca nu a facut-o, ignora
  comanda, afiseaza un mesaj de eroare
- se adauga Session-Cookie la mesaj, se compune mesajul HTTP
- se trimite cererea, se asteapta raspuns
- se parseaza raspunsul; daca se primeste OK(200), se afiseaza un mesaj
  de confirmare, se parseaza mesajul, se extrage Tokenul JWT, altfel se parseaza
  eroarea si se afiseaza descrierea acesteia
- se elibereaza memoria

### Comanda get_books:
- se verifica daca exista un JWT Token in memorie, daca nu, inseamna ca nu s-a
  cerut/obtinut acces la biblioteca, se afiseaza un mesaj de eroare, se ignora
  comanda
- se pune Token-ul JWT la mesaj, se compune mesajul HTTP
- se trimite cererea, se asteapta raspuns
- se parseaza raspunsul; daca s-a primit OK(200), se parseaza vectorul JSON, se
  afiseaza datele fiecarei carti din vector, la final se afiseaza mesajul de
  sfarsit de lista, altfel se parseaza eroarea si se afiseaza descrierea acesteia
- se elibereaza memoria

### Comanda get_book:
- se verifica daca exista un JWT Token in memorie, daca nu, inseamna ca nu s-a
  cerut/obtinut acces la biblioteca, se afiseaza un mesaj de eroare, se ignora
  comanda
- se preia ID-ul cartii cautate
- se compune ruta pe care se va trimite cererea HTTP
- se pune Token-ul JWT la mesaj, se compune mesajul HTTP
- se trimite cererea, se asteapta raspuns
- se parseaza raspunsul; daca s-a primit OK(200), se parseaza obiectul JSON, se
  afiseaza datele despre carte, altfel se parseaza eroarea si se afiseaza
  descrierea acesteia
- se elibereaza memoria

### Comanda add_book:
- se verifica daca exista un JWT Token in memorie, daca nu, inseamna ca nu s-a
  cerut/obtinut acces la biblioteca, se afiseaza un mesaj de eroare, se ignora
  comanda
- se citesc datele despre cartea ce urmeaza sa fie introdusa
- se compune obiectul JSON(sub forma de string) ce urmeaza sa fie trimis catre
  server
- se pune Token-ul JWT la mesaj, se compune mesajul HTTP
- se trimite cererea, se asteapta raspuns
- se parseaza raspunsul; daca s-a primit OK(200), se afiseaza un mesaj de
  confirmare, altfel se parseaza eroarea si se afiseaza descrierea acesteia
- se elibereaza memoria

### Comanda delete_book:
- se verifica daca exista un JWT Token in memorie, daca nu, inseamna ca nu s-a
  cerut/obtinut acces la biblioteca, se afiseaza un mesaj de eroare, se ignora
  comanda
- se citeste ID-ul cartii ce urmeaza sa fie stearsa
- se compune ruta pe care se va trimite cererea HTTP
- se pune Token-ul JWT la mesaj, se compune mesajul HTTP
- se trimite cererea, se asteapta raspuns
- se parseaza raspunsul; daca s-a primit OK(200), se afiseaza un mesaj de
  confirmare, altfel se parseaza eroarea si se afiseaza descrierea acesteia
- se elibereaza memoria

### Comanda logout:
- se verifica daca s-a realizat un login inainte; daca nu, se ignora comanda
- se adauga Session-Cookie la mesaj, se compune mesajul HTTP
- se trimite cererea, se asteapta raspuns
- se parseaza raspunsul; daca s-a primit OK(200), se afiseaza un mesaj de
  confirmare, se actualizeaza variabila loggedIn, se sterg Tokenul JWT si 
  Session-Cookie, altfel se parseaza eroarea si se afiseaza descrierea acesteia
- se elibereaza memoria

### Comanda exit:
- inchide conexiunea cu serverul
- inchide executia programului


Pentru parsarea mesajelor primite de la server care sunt de forma JSON, avand
in vedere faptul ca tema a fost rezolvata in C, am ales sa folosesc biblioteca
Parson, disponibila aici: https://github.com/kgabis/parson.