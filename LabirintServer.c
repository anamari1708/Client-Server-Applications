//SERVER

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define LENGTH 2048

static _Atomic unsigned int cli_count = 0;
static int uid = 10;

/* Struktura klijenta */
typedef struct
{
    struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
  int broj_koraka;
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void str_overwrite_stdout() //funkcija koja sluzi za lipsi ispis
{
    printf("\r%s", "> ");
    fflush(stdout);
}

/* Funkcija za dodavanje klijenata u niz klijenata koji su prikljuceni */
void queue_add(client_t* cl)
{
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i)
  {
		if(clients[i]==NULL) //ako je neko mjesto prazno tu cemo ga ubacit
    {
			clients[i] = cl;
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Funkcija za micanje klijenta iz niza prikljucenih klijenata*/
void queue_remove(int uid)
{
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i)
  {
		if(clients[i])
    {
			if(clients[i]->uid == uid)
      {
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Funkcija koja salje poruku svim prikljucenim klijentima osim tom koji salje */
void send_message(char* s, int uid)
{
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<MAX_CLIENTS; ++i)
  {
		if(clients[i])
    {
			if(clients[i]->uid != uid)
      {
				if(write(clients[i]->sockfd, s, strlen(s)) < 0)
        {
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Funkcija koja salje poruku samo jednom odredjenom klijentu */
void send_message_(char* s, int uid)
{
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<MAX_CLIENTS; ++i)
  {
		if(clients[i])
    {
			if(clients[i]->uid == uid)
      {
				if(write(clients[i]->sockfd, s, strlen(s)) < 0)
        {
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Posalji izgled labirinta tom odredenom klijentu  */
void send_lab(int uid)
{
  char lab[16]={'#','#','#','$','#','#','#',' ',' ','#',' ',' ','X',' ',' ','#'};
  char* s;
  
  s=(char*) malloc(sizeof(char)*16+1);
  strcpy(s,lab); 
  
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<MAX_CLIENTS; ++i)
  {
		if(clients[i])
    {
			if(clients[i]->uid == uid)//jer saljemo samo njemu, a ne svima kao u prosloj funkciji
      {
				write(clients[i]->sockfd, s, strlen(s));
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Funkcija za provjeru postoji li to ime vec u igri */
int ProvjeriPostojiLi(char* name) 
{ 
  for(int i=0; i<MAX_CLIENTS; ++i)
  {
		if(clients[i])
    {
			if((strcmp(clients[i]->name,name)==0))//to ime vec postoji
      {
        return 1;
			}
		}
	}
         
   return 0;
         
}

/* Funkcija (najvaznija) za upravljanje komunikacije s klijentom*/
void* handle_client(void* arg)
{
	char buff_out[BUFFER_SZ];
	char name[32];
	int leave_flag = 0;

	cli_count++;
	client_t* cli = (client_t *)arg;

	// provjera za ime
	if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1) //ako je ime premalo ili preveliko
  {
		leave_flag = 1;
	} 
   
  else
  {
    if(ProvjeriPostojiLi(name)==0) //ako to ime ne postoji
    {
      strcpy(cli->name, name);
		  sprintf(buff_out, "%s nam se pridružio u igri\n", cli->name);
		  printf("%s", buff_out);
		  send_message(buff_out, cli->uid); //da se posalje svim drugim klijentima da se pridruzio novi klijent
    }
    else
    {
      strcpy(buff_out,"ERROR: Ispricavamo se, vase ime vec koristi neki od korisnika!\n");
      send_message_(buff_out, cli->uid);
      recv(cli->sockfd, name, 32, 0);
      //leave_flag = 1;
    }
	}
  
  send_lab(cli->uid);

	bzero(buff_out, BUFFER_SZ);
  char lab[4][4]={{'#','#','#','$'},{'#','#','#',' '},{' ','#',' ',' '},{'X',' ',' ','#'}};
  int start_position[2];
  start_position[0]=3;
  start_position[1]=0;
  char mes[30];
  //char message[LENGTH] = {};

	while(1)
  {
        if (leave_flag) //leave_flag se postavlja u 1 ako nije uneseno imem ako klijent odlazi ili ako je doslo do greske te se izazi iz while petlje
        {
          break;
        }
    
        int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
        //unutar buff_out-a pisati ce npr ana: desno 
        //zbog toga za gledanje je li klijent poslao bas desno moramo preskociti velicinu imena +2 znaka (za : i razmak) dolje pri usporeðivanju

        if (receive > 0)
        {
          if(strlen(buff_out) > 0)
          {
            printf(" %s ", buff_out);
   
            
            if( buff_out[strlen(cli->name)+2]=='g' && buff_out[strlen(cli->name)+3]=='o' && buff_out[strlen(cli->name)+4]=='r' && buff_out[strlen(cli->name)+5]=='e') 
            {
                cli->broj_koraka++;

                if(lab[start_position[0]-1][start_position[1]]!='#') //ako tu di se pomakao je prazno ili cilj
                {
                     start_position[0]=start_position[0]-1; //ide gore pa mu se pozicija retka umanjuje
                     sprintf(mes,"Pomakli ste se prema gore. Vasa nova pozicija X-a je [%d] [%d]\n",start_position[0],start_position[1]);
                     send_message_(mes, cli->uid);

                     if((start_position[0])<0) 
                    {
                      strcpy(mes,"Izasli ste van labirinta. Ovaj potez se ne racuna.");
                      send_message_(mes, cli->uid);
                      start_position[0]=start_position[0]+1;
                    }
                  
                    if(lab[start_position[0]][start_position[1]]=='$') //da vidimo je li nakon pomaka dosao na cilj
                  {
                     memset(mes,30,0);
                     strcpy(mes,"\nSTIGLI STE DO CILJA! CESTITAMO!");
                     send_message_(mes, cli->uid);
                     sprintf(buff_out, "%s se odjavljuje\n", cli->name);
                     printf("%s", buff_out);
                     send_message(buff_out, cli->uid); //ako klijent odlazi saljemo svim klijentima tu informaciju
                     break;
                  }
                    continue;
                 }
              
              else
              {
                 strcpy(mes,"Udarili ste zid.");
                 send_message_(mes, cli->uid);
                 continue;
              }

             }
            
            else if( buff_out[strlen(cli->name)+2]=='d' && buff_out[strlen(cli->name)+3]=='o' && buff_out[strlen(cli->name)+4]=='l' && buff_out[strlen(cli->name)+5]=='e') 
            {
                cli->broj_koraka++;

                if(lab[start_position[0]+1][start_position[1]]!='#') //ako tu di se pomakao je prazno ili cilj
                {
                   start_position[0]=start_position[0]+1; //ide dole pa mu se pozicija retka uvecava
                   sprintf(mes,"Pomakli ste se prema dole. Vasa nova pozicija X-a je [%d] [%d]\n",start_position[0],start_position[1]);
                   send_message_(mes, cli->uid);
                  
                   if((start_position[0])>3) 
                  {
                    strcpy(mes,"Izasli ste van labirinta. Ovaj potez se ne racuna.");
                    send_message_(mes, cli->uid);
                    start_position[0]=start_position[0]-1;
                  }
                  
                   if(lab[start_position[0]][start_position[1]]=='$') //da vidimo je li nakon pomaka dosao na cilj
                  {
                     memset(mes,30,0);
                     strcpy(mes,"\nSTIGLI STE DO CILJA! CESTITAMO!");
                     send_message_(mes, cli->uid);
                  }
                  
                  continue;
                }
              
              else
              {
                 strcpy(mes,"Udarili ste zid.");
                 send_message_(mes, cli->uid);
                 continue;
              }

             }
            
             else if( buff_out[strlen(cli->name)+2]=='l' && buff_out[strlen(cli->name)+3]=='i' && buff_out[strlen(cli->name)+4]=='v' && buff_out[strlen(cli->name)+5]=='o') 
            {
               cli->broj_koraka++;

                if(lab[start_position[0]][start_position[1]-1]!='#') //ako tu di se pomakao je prazno ili cilj
                {
                   start_position[1]=start_position[1]-1;
                   sprintf(mes,"Pomakli ste se prema lijevo. Vasa nova pozicija X-a je [%d] [%d]\n",start_position[0],start_position[1]);
                   send_message_(mes, cli->uid);
                  
                   if((start_position[1])<0) //ide gore pa mu se pozicija retka umanjuje
                  {
                    strcpy(mes,"Izasli ste van labirinta. Ovaj potez se ne racuna.");
                    send_message_(mes, cli->uid);
                    start_position[1]=start_position[1]+1;
                  }
                  
                  if(lab[start_position[0]][start_position[1]]=='$') //da vidimo je li nakon pomaka dosao na cilj
                  {
                     memset(mes,30,0);
                     strcpy(mes,"\nSTIGLI STE DO CILJA! CESTITAMO!");
                     send_message_(mes, cli->uid);
                  }
                  continue;
                }
              
              else
              {
                 strcpy(mes,"Udarili ste zid.");
                 send_message_(mes, cli->uid);
                 continue;
              }
               
             }
            
            
            else if( buff_out[strlen(cli->name)+2]=='d' && buff_out[strlen(cli->name)+3]=='e' && buff_out[strlen(cli->name)+4]=='s' && buff_out[strlen(cli->name)+5]=='n' 

&& buff_out[strlen(cli->name)+6]=='o') 
            {
               
               cli->broj_koraka++;

                if(lab[start_position[0]][start_position[1]+1]!='#') //ako tu di se pomakao je prazno ili cilj
                {
                   start_position[1]=start_position[1]+1;//ide desno pa mu se pozicija stupca uvecava
                   sprintf(mes,"Pomakli ste se prema desno. Vasa nova pozicija X-a je [%d] [%d]\n",start_position[0],start_position[1]);
                   send_message_(mes, cli->uid);
                  
                   if((start_position[0])<0) 
                  {
                    strcpy(mes,"Izasli ste van labirinta. Ovaj potez se ne racuna.");
                    send_message_(mes, cli->uid);
                    start_position[1]=start_position[1]-1;
                  }
                  
                   if(lab[start_position[0]][start_position[1]]=='$') //da vidimo je li nakon pomaka dosao na cilj
                  {
                     memset(mes,30,0);
                     strcpy(mes,"\nSTIGLI STE DO CILJA! CESTITAMO!");
                     send_message_(mes, cli->uid);
                  }
                  
                  continue;
                }
              
              else
              {
                 strcpy(mes,"Udarili ste zid.");
                 send_message_(mes, cli->uid);
                 continue;
              }

             }
            
             else if( buff_out[strlen(cli->name)+2]=='p' && buff_out[strlen(cli->name)+3]=='r' && buff_out[strlen(cli->name)+4]=='a' && buff_out[strlen(cli->name)+5]=='v' && buff_out[strlen(cli->name)+6]=='i' && buff_out[strlen(cli->name)+7]=='l' && buff_out[strlen(cli->name)+8]=='a') 
             {
                strcpy(mes,"PRAVILA: \ncilj je doci do $, # je zid, a vasa pozicija X\n\nZa gore napisite: gore\nZa dole napisite: dole\nZa lijevo napisite: livo\nZa desno napisite: desno\n");
               send_message_(mes, cli->uid);
               
             }
       }
      }

        else 
        {
          printf("ERROR: -1\n");
          leave_flag = 1;
        }

        bzero(buff_out, BUFFER_SZ);
	}

  /* Poziv brisanja klijenta iz niza prikljucenih klijenata i uklanjanje njegovog threada */
  memset(mes,30,0);
  sprintf(mes, "\nREZULTAT: Broj koraka vam je %d\n", cli->broj_koraka); //da korisnika koji odlazi obavijestimo o njegovom rezultatu
  send_message_(mes, cli->uid);
	close(cli->sockfd); //zatvaramo konekciju s njim
  memset(mes,30,0);
  if(cli->broj_koraka>0)
  {
    sprintf(mes, "Broj koraka tog korisnika bio je %d\n", cli->broj_koraka); //da ostalim korisnicima javimo o njegovom rezultatu
    send_message(mes, cli->uid);
  }
  queue_remove(cli->uid); //micemo ga iz niza aktivnih korisnika
  free(cli);
  cli_count--;
  pthread_detach(pthread_self());

	return NULL;
}

int main(int argc, char** argv)
{
	if(argc != 2) //ako nije dobro uneseno prilikom poziva programa
  {
		printf("Unesite: %s <port>\n", argv[0]); //ocekuje se da se unese i broj porta
		return EXIT_FAILURE;
	}

	char* ip = "127.0.0.1"; 
	int port = atoi(argv[1]); //atoi da pretvori u string u int i da se ono sto je napisano prilikom poziva programa se spremi kao port
	int option = 1;
	int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  pthread_t tid;

  /* Kreiranje socketa za server */
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_port = htons(port);

  /* Ignore pipe signals */
	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0)
  {
		perror("ERROR: setsockopt failed");
    return EXIT_FAILURE;
	}

	/* Bind */
  if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
  {
    perror("ERROR: Socket binding failed");
    return EXIT_FAILURE;
  }
  /* Funkcijom bind() se povezuje deskriptor soketa sa IP
     adresom soketa. Funkcija vraæa -1 ako je došlo do greške.*/


  /* Listen */
  if (listen(listenfd, 10) < 0) 
  {
    perror("ERROR: Socket listening failed");
    return EXIT_FAILURE;
	}
  
  /*Funkcijom listen() soket se postavlja u pasivno stanje u kojemu èeka na nove
    zahtjeve na prethodno specificiranom portu. Prvi argument koji funkcija prima je
    deskriptor soketa, a drugi je maksimalni broj zahtjeva koje klijenti upute serveru,
    a koje se mogu nalaziti u redu èekanja servera. Funkcija vraæa -1 u sluèaju greške..*/

	printf("=== POKRENIO SE SERVER SA IGROM LABIRINTA ===\n");

	while(1)
  {
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
    /*Funkcijom accept() se prihvaæa zahtjev za konekcijom na soket èiji je deskriptor
      listenfd (to je soket na serveru). Deskriptor novog soketa se nazove connfd (to je soket na 
      klijentu). Svojstva tog soketa se postave na ista svojstva koja ima i soked listenfd,
      varijablu cli_addr se upisuje adresa soketa na klijentu,
      a u clilen se upisuje velièina (u bajtovima) te adrese.*/

		/* Trebamo prvo provjeriti je li vec prikljucen max broj klijenata */
		if((cli_count + 1) == MAX_CLIENTS)
    {
			printf("Maksimalan broj klijenata je postignut, pokusajte poslije. ");
			close(connfd);
			continue;
		}

		/* Stvaramo novi cvor za novog klijenta i postavljamo vrijednosti unutar strukture */
		client_t* cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;
    cli->broj_koraka=0;

		/* Pozivamo funkcije da se taj novostvoreni cvor klijenta doda u niz vec stvorenih klijenata*/
		queue_add(cli);
		pthread_create(&tid, NULL, &handle_client, (void*)cli);

		/* Da malo smanjimo upotrebu procesora */
		sleep(1);
	}

	return EXIT_SUCCESS;
}