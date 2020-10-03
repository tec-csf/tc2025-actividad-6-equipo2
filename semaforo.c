/*
 David Benjamín Ruíz Salazar
 Naji M A Saadat
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define TCP_PORT 8000
#define time 3

void gestorEstado(int signal);
void gestorNextEstado(int signal);

int cliente;
int nextPid;
int estado;

int main(int argc, const char * argv[]){
    
    struct sockaddr_in direccion;
    int pidToSend=getpid();
    int pidConvert=htonl(pidToSend);
    int estadoAnterior;
    int pidGet;
    ssize_t leidos;
    socklen_t escritos;
    sigset_t Signals;
    
    sigemptyset(&Signals);
    sigaddset(&Signals, SIGUSR1);
    sigaddset(&Signals, SIGINT);
    sigaddset(&Signals, SIGTSTP);
    sigaddset(&Signals,SIGALRM);
    
    if (argc != 2){
        printf("Use: %s IP_Servidor \n", argv[0]);
        exit(-1);
    }
    
    int  receivedInt;//*Para el pid que recibimos
    cliente = socket(PF_INET, SOCK_STREAM, 0);
    inet_aton(argv[1], &direccion.sin_addr);
    direccion.sin_port = htons(TCP_PORT);
    direccion.sin_family = AF_INET;
    
    escritos = connect(cliente, (struct sockaddr *) &direccion, sizeof(direccion));
    
    if (escritos == 0){
        printf("Conectado a %s:%d \n",
               inet_ntoa(direccion.sin_addr),
               ntohs(direccion.sin_port));
        
        write(cliente, &pidConvert, sizeof(pidConvert));
        leidos = read(cliente, &pidGet, sizeof(pidGet));
        nextPid=ntohl(pidGet);
        
        if (signal(SIGUSR1, gestorEstado) == SIG_ERR){
            printf("LLamada Fallida al Manejador \n");
        }
        else if (signal(SIGALRM, gestorNextEstado) == SIG_ERR){
            printf("LLamada Fallida al Manejador\n");
        }
        
        while ((leidos = read(cliente, & receivedInt, sizeof( receivedInt)))){
            if (ntohl( receivedInt) == 5){
                raise(SIGUSR1);
            }
            else if (ntohl(receivedInt) == 2 ){
                if( estado != 2){
                    estado = 2;
                    printf("Emergencia\n");
                    sigprocmask(SIG_BLOCK, &Signals, NULL);
                }
                else if ( estado == 2){
                    printf("Normal\n");
                    sigprocmask(SIG_UNBLOCK, &Signals, NULL);
                }
            }
            else if (ntohl(receivedInt) == 3 ){
                if( estado != 3){
                    estado = 2;
                    printf("Intermitente\n");
                    sigprocmask(SIG_BLOCK, &Signals, NULL);
                }
                else if ( estado == 3){
                    printf("Normal\n");
                    sigprocmask(SIG_UNBLOCK, &Signals, NULL);
                }
            }
            
        }
    }
    close(cliente);
    return 0;
}
void gestorEstado(int s){
    int state=htonl(1);
    write(cliente, &state, sizeof(int));
    alarm(time);
}

void gestorNextEstado(int s){
    estado = 0;
    kill(nextPid, SIGUSR1);
}
