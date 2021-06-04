//KLIJENT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define _CRT_SECURE_NO_WARNINGS
#define LENGTH 2048

// Globalne varijable
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

void str_trim_lf (char* arr, int length)  //funkcija koja takodjer sluzi za lipsi ispis, tj da se ne ide u novi red
{
  int i;
  for (i = 0; i < length; i++) 
  { // trim \n
    if (arr[i] == '\n') 
    {
      arr[i] = '\0';
      break;
    }
  }
}


void str_overwrite_stdout()  
{
  printf("%s", "> ");
  fflush(stdout);
}

void catch_ctrl_c_and_exit(int sig) 
{
    flag = 1;
}

/* funkcija koja manipulira slanjem poruka od strane klijenta*/
void send_msg_handler() 
{
  char message[LENGTH] = {};
    char buffer[LENGTH + 32] = {};

  while(1) 
  {
  	str_overwrite_stdout();
    fgets(message, LENGTH, stdin);

    if (strcmp(message, "exit") == 0) 
    {
			break;
    } 
    
    else 
    {
      sprintf(buffer, "%s: %s\n", name, message);
      send(sockfd, buffer, strlen(buffer), 0);
     /*Funkcija send () inicirat æe prijenos poruke iz navedenog socketa na svoj peer. Funkcija send () poslati æe poruku samo kad je socket prikljuèen (ukljuèujuæi i kada je ravnoteža            socketa bez veze postavljena putem connect ()).
       Funkcija send () uzima sljedeæe argumente: sockfd- navodi deskriptor socketa, buffer - Pokazuje na meðuspremnik koji sadrži poruku za slanje, strlen(buffer)- odreðuje duljinu poruke        u bajtovima. 0, tj zastave - odreðuju vrstu prijenosa poruke */
    }

		bzero(message, LENGTH);
    bzero(buffer, LENGTH + 32);
  }
  catch_ctrl_c_and_exit(2);
}

/* funkcija koja manipulira porukama koje klijent prima*/
void recv_msg_handler() 
{
	char message[LENGTH] = {};

  while (1)
  {
		int receive = recv(sockfd, message, LENGTH, 0);
    /* Funkcija recv () prima podatke na socketu i sprema ih u meðuspremnik. Poziv recv () odnosi se samo na povezane sockete.
       Parametar sockfd je deskriptor socketa, a message je pokazivac na meðuspremnik koji prima podatke, LEMGHT je diljina meðuspremnika
       na koji pokazuje message,  a parametar flags se postavlja na 0 ako nije navedeno nekakvo razdvajanje ili sl*/

    if (receive > 0) 
    {
      memset(message,LENGTH,0);
      if(message[0]=='#') //prvo ce se primit labirint i onda ispisujemo posebno da ide svako 4 elementa u novi red
      {
        printf("Izgled labirinta: \n\n");
        for(int i=0;i<16;i++)
        { 
         printf("%c\t", message[i]);
         if((i+1)%4==0 && i!=0) //da svako 4.ide u novi red
         printf("\n");
        }
        printf("\n\n");
      }
      
      else if( message[1]=='R' && message[2]=='E' && message[3]=='Z' && message[4]=='U' && message[5]=='L' && message[6]=='T' && message[7]=='A' && message[8]=='T' ) //kad se dodje do kraja poslat ce se poruka REZULTAT
      {
        printf(" %s", message);
        flag=1;
        break;
        
      }
      
      else if( message[0]=='E' && message[1]=='R' && message[2]=='R' && message[3]=='O' && message[4]=='R' ) //ako se unilo isto ime poslat ce se ERROR
      {
        printf(" %s", message);
        flag=1;
        break;
        
      }
      
      else
      {
        printf(" %s", message);
        str_overwrite_stdout();
        
        printf("\n");
      }
    } 
    
    else if (receive == 0) 
    {
			break;
    } 
   
		memset(message, 0, sizeof(message));
  }
}

int main(int argc, char** argv)
{
	if(argc != 2)
  {
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);

	signal(SIGINT, catch_ctrl_c_and_exit);

	printf("Molim vas unesite svoje ime: ");
  fgets(name, 32, stdin);
  str_trim_lf(name, strlen(name));


	if (strlen(name) > 32 || strlen(name) < 2)
  {
		printf("Ime mora biti vece od 2 znaka i manje od 30 znakova.\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr;

	/* Stvaranje socketa za klijenta */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);


  /* Povezivanje na server */
  int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) 
  {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	/* Slanje imena */
	send(sockfd, name, 32, 0);

	printf("=== DOBRODOSLI U IGRU LABIRINTA ===\n\n -> Za pravila igre napisite: pravila <- \n");

	pthread_t send_msg_thread;
  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0)
  {
		printf("ERROR: pthread\n");
    return EXIT_FAILURE;
  }

	pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0)
  {
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1)
  {
		if(flag)
    {
			printf("\nPozdrav, nadam se da vam se svidila igra.\n");
			break;
    }
	}

	close(sockfd);

	return EXIT_SUCCESS;
}