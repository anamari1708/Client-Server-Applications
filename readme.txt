Paziti da se uvijek prvo pokrene server da bi se klijenti mogli spojit na njega. Moze se spojiti maksimalno 100 klijenata i igrati neovisno jedan o drugome.
Kako koji korisnik zavrsi s igrom, poruka o njegovom odjavljivanju i rezultatu salje se svim prikljucenim korisnicima.

Klijenta se odmah trazi unos imena i nakon unosa imena (ako se ne unese prekratko ili predugo ime) server mu salje izgled labirinta. 
Na server se ispisuje poruka klijenta koji se spojio.
Takodjer je receno da ukoliko korisnik zeli znati vise o igri da napise: pravila.
Kada se to napise server mu salje s cime se pomice lijevo, desno, gore i dolje i koji je cilj igre.
Korisnik se moze pomicati po labirintu s:
gore 
dole
livo
desno

Svaki put mu server salje novu poziciju unutar matrice gdje se nalazi. Ukoliko takne zid dolazi mu poruka, a ukoliko izadje van 
labirnita takodjer ga server o tome obavijesti.
Kada dodje do cilja klijent se odjavljuje i ispisuje mu se broj koraka koji mu je trebao da dodje do cilja,
a serveru dolazi poruka koji se klijent odjavio.

Ako neki korisnik dok je ovaj gore bio aktivan pokusa igrati igru s njegovim imenom, prijavit ce se error i na serveru i na klijentu tom koji se pokusava spojit i samo ga se nece spojit (ispisat ce se poruka da ide koristiti isto ime kao netko tko vec igra).

Korisnik takodjer moze slati i druge poruke serveru osim gore,dole,livo,desno i vidi ih samo server, ali te poruke se samo ispisuju na serveru uz naznaku koji ih klijent salje i ne pokrecu nista.