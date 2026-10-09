#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

pid_t pid_ejec, pid_A, pid_B;
pid_t pid_X, pid_Y, pid_Z;
int tiempo;

int alarma_disparada = 0;
int señal_recibida = 0;

void manejador_vacio(int sig) { señal_recibida = 1; }

void manejador_Z_alarm(int sig) { 
    alarma_disparada = 1; 
    kill(pid_A, SIGUSR1); 
}

void manejador_A_usr1(int sig) {
    printf("\n/*Resultado del comando \"pstree\" */\n");
    if (fork() == 0) {
        char pid_str[20];
        sprintf(pid_str, "%d", pid_ejec); 
        execlp("pstree", "pstree", "-c", "-p", pid_str, NULL);
        perror("Error pstree");
        exit(1);
    } else {
        wait(NULL); 
        kill(pid_B, SIGUSR2); 
    }
}

void logica_nodo_normal(char nombre, pid_t padre, pid_t abuelo, pid_t bisabuelo) {
    printf("Soy el proceso %c: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d\n", nombre, getpid(), padre, abuelo, bisabuelo);
    signal(SIGUSR2, manejador_vacio);
    while(!señal_recibida) pause(); 
    printf("Soy %c (%d) y muero\n", nombre, getpid());
    exit(0);
}

void logica_Z() {
    printf("Soy el proceso Z: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d\n", getpid(), pid_B, pid_A, pid_ejec);
    signal(SIGALRM, manejador_Z_alarm);
    signal(SIGUSR2, manejador_vacio);
    
    alarm(tiempo); 
    while(!alarma_disparada) pause(); 
    while(!señal_recibida) pause(); 
    
    printf("Soy Z (%d) y muero\n", getpid());
    exit(0);
}

void crear_nodos_hoja() {
    pid_X = fork();
    if (pid_X == 0) logica_nodo_normal('X', pid_B, pid_A, pid_ejec);

    pid_Y = fork();
    if (pid_Y == 0) logica_nodo_normal('Y', pid_B, pid_A, pid_ejec);

    pid_Z = fork();
    if (pid_Z == 0) logica_Z();
}

void rutina_destruccion() {
    kill(pid_Z, SIGUSR2); wait(NULL);
    kill(pid_Y, SIGUSR2); wait(NULL);
    kill(pid_X, SIGUSR2); wait(NULL);
}

void logica_B() {
    pid_B = getpid();
    printf("Soy el proceso B: mi pid es %d. Mi padre es %d. Mi abuelo es %d\n", pid_B, pid_A, pid_ejec);
    signal(SIGUSR2, manejador_vacio);

    crear_nodos_hoja();
    
    while(!señal_recibida) pause();
    
    rutina_destruccion();
    
    printf("Soy B (%d) y muero\n", pid_B);
    exit(0);
}

void logica_A() {
    pid_A = getpid();
    printf("Soy el proceso A: mi pid es %d. Mi padre es %d\n", pid_A, pid_ejec);
    signal(SIGUSR1, manejador_A_usr1);

    pid_B = fork();
    if (pid_B == 0) {
        logica_B();
    } else {
        wait(NULL);
        printf("Soy A (%d) y muero\n", pid_A);
        exit(0);
    }
}
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <segundos>\n", argv[0]);
        return 1;
    }
    tiempo = atoi(argv[1]);
    pid_ejec = getpid();
    printf("Soy el proceso ejec: mi pid es %d\n", pid_ejec);

    pid_A = fork();
    if (pid_A == 0) {
        logica_A();
    } else {
        wait(NULL);
        printf("Soy ejec (%d) y muero\n", pid_ejec);
        exit(0);
    }
    return 0;
}