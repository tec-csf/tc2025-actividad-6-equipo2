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


void gestor(int);
void gestorSIGTSTP(int);
void gestorSIGINT(int);
void estadoActual(int);

int semaforo;
int cliente[4];
int CtrlZ;
int CtrlC;
int main(int argc, const char * argv[]){
    sigset_t Signals;
    
    struct sockaddr_in direccion;
    
    int pidReceived;
    int servidor;
    
    ssize_t leidos;
    socklen_t escritos;
    int continuar = 1;
    pid_t pid;
    sigemptyset(&Signals);
    sigaddset(&Signals, SIGINT);
    sigaddset(&Signals, SIGTSTP);
    if (signal(SIGINT, gestorSIGINT) == SIG_ERR){
        printf("LLamada Fallida al Manejador\n");
    }
    else if (signal(SIGTSTP, gestorSIGTSTP) == SIG_ERR){
        printf("LLamada Fallida al Manejador\n");
    }
    if (argc != 2) {
        printf("Use: %s IP_Servidor \n", argv[0]);
        exit(-1);
    }
    
    servidor = socket(PF_INET, SOCK_STREAM, 0);
    inet_aton(argv[1], &direccion.sin_addr);
    direccion.sin_port = htons(TCP_PORT);
    direccion.sin_family = AF_INET;
    
    bind(servidor, (struct sockaddr *) &direccion, sizeof(direccion));
    
    listen(servidor, 4);
    
    escritos = sizeof(direccion);
    
    int * semaforos;
    semaforos=(int*)malloc(4*sizeof(int));
    int * pidSemaforos;
    pidSemaforos=(int*)malloc(4*sizeof(int));
    int * s=semaforos;
    int * i;
    int count=0;
    
    for (i=s; i<(s+4); i++, count++) {
        cliente[count] = accept(servidor, (struct sockaddr *) &direccion, &escritos);
        printf("Aceptando conexion en %s:%d \n",
               inet_ntoa(direccion.sin_addr),
               ntohs(direccion.sin_port));
        pid = fork();
        
        if (pid == 0) {
            if (signal(SIGTSTP, gestor) == SIG_ERR){
                printf("LLamada Fallida al Manejador\n");
            }
            else if (signal(SIGINT, gestor) == SIG_ERR){
                printf("LLamada Fallida al Manejador\n");
            }
            semaforo=cliente[count];
            close(servidor);
            
            if (cliente[count] >= 0) {
                int pidReceivedConv;
                while(leidos = read(semaforo, &pidReceivedConv, sizeof(pidReceivedConv))) {
                    estadoActual(count);
                }
            }
            close(semaforo);
        }
        else {
            pidSemaforos[count] = read(cliente[count], i, sizeof(i));
        }
    }
    
    if (pid > 0) {
        for (int c = 0; c < 4; c++) {
            if (c==3){
                write(cliente[c], &semaforos[0], sizeof(int));
            }
            else{
                write(cliente[c], &semaforos[c+1], sizeof(int));
            }
        }
        
        int convert= htonl(5);
        write(cliente[0], &convert, sizeof(convert));
        while (wait(NULL) != -1);
        close(servidor);
    }
    free(pidSemaforos);
    free(semaforos);
    return 0;
}
void estadoActual(int s) {
    for (int i=0; i<4; i++) {
        if (i == s){
            printf("\033[0;92m");
            printf("Semaforo %d: VERDE\n", i);
            printf("\033[0m");
        }
        else{
            printf("\033[0;91m");
            printf("Semaforo %d: ROJO\n", i);
            printf("\033[0m");
        }
    }
    printf("\n");
}
void gestor(int s){
    if (s == SIGTSTP){
        int ctrlZ=htonl(2);
        write(semaforo, &ctrlZ, sizeof(ctrlZ));
    }
    else if(s==SIGINT){
        int ctrlC=htonl(3);
        write(semaforo, &ctrlC, sizeof(ctrlC));
    }
}
void gestorSIGTSTP(int s){
    if (CtrlZ%2==0){
        printf("\nAcabo de enviar el mensaje ROJO Ctrl+Z a todos los semaforos\n");
        for (int c=0; c<4; c++) {
            printf("\033[0;91m");
            printf("Semaforo %d: ROJO\n", c);
            printf("\033[0m");
        }
    }else{
        printf("\nSemaforos Estado normal\n");
    }
    
    CtrlZ++;
}


void gestorSIGINT(int s){
    
    
    if (CtrlC%2==0){
        printf("\nAcabo de enviar el mensaje ROJO Ctrl+C a todos los semaforos\n");
        for (int c=0; c<4; c++) {
            printf("\033[0;93m");
            printf("Semaforo %d Intermitent\n", c);
            printf("\033[0m");
        }
    }
    else {
        printf("\nSemaforos Estado normal\n");
    }
    
    CtrlC++;
}



