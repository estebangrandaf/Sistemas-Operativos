#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>


int shmid;
int *shm_ptr = NULL;


void validar_argumentos(int argc, char *argv[], int *x, int *y) {
    if (argc != 3) {
        printf("Uso: %s <x> <y>\n", argv[0]);
        exit(1);
    }
    *x = atoi(argv[1]);
    *y = atoi(argv[2]);
}

void inicializar_shm(int x, int y) {
    // Necesitamos espacio para 'x' PIDs de los padres y 'y' PIDs de los hijos finales
    int tamano = (x + y) * sizeof(int);
    
    shmid = shmget(IPC_PRIVATE, tamano, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Error al crear memoria compartida (shmget)");
        exit(1);
    }
    
    shm_ptr = (int *) shmat(shmid, 0, 0);
    if (shm_ptr == (int *)-1) {
        perror("Error al vincular memoria (shmat)");
        exit(1);
    }
}

void borrar_shm() {
    if (shmdt((char *)shm_ptr) < 0) {
        perror("Error al desligar memoria (shmdt)");
    }
    if (shmctl(shmid, IPC_RMID, 0) < 0) {
        perror("Error al borrar memoria (shmctl)");
    }
}

void logica_subhijos(int x, int indice_y) {
    pid_t mi_pid = getpid();
    
    // Guardo mi PID en la zona reservada para los subhijos (a partir de la posición x)
    shm_ptr[x + indice_y] = mi_pid;

    // Leo los PIDs de mis padres que ya están guardados en las primeras 'x' posiciones
    printf("Soy el subhijo %d, mi padres son: ", mi_pid);
    for (int i = 0; i < x; i++) {
        printf("%d", shm_ptr[i]);
        if (i < x - 1) printf(", ");
    }
    printf("\n");
    exit(0);
}


void crear_nodos_hoja(int x, int y) {
    for (int i = 0; i < y; i++) {
        if (fork() == 0) {
            logica_subhijos(x, i);
        }
    }
    // Este padre espera a que mueran todos sus subhijos para poder salir limpiamente
    for (int i = 0; i < y; i++) {
        wait(NULL);
    }
    exit(0); 
}

void generar_cadena_vertical(int nivel_actual, int x, int y) {
    // Guardo mi PID en la posición que me toca de la cadena
    shm_ptr[nivel_actual] = getpid(); 

    if (nivel_actual < x - 1) {
        if (fork() == 0) {
            // Soy el hijo, sigo bajando la cadena
            generar_cadena_vertical(nivel_actual + 1, x, y);
        } else {
            // Soy el padre, espero a que acabe todo por debajo
            wait(NULL);
            if (nivel_actual > 0) { 
                // Si no soy el superpadre (nivel 0), muero al terminar
                exit(0);
            }
        }
    } else {
        // Hemos llegado al final de la vertical, toca expandir las 'y' hojas
        crear_nodos_hoja(x, y);
    }
}

// Módulo Main
int main(int argc, char *argv[]) {
    int x, y;
    
    validar_argumentos(argc, argv, &x, &y);
    inicializar_shm(x, y);
    
    // empieza la creación del árbol desde el nivel 0
    generar_cadena_vertical(0, x, y);

    // cuando todos los descendientes han terminado y han rellenado la memoria compartida.
    printf("Soy el superpadre (%d): mis hijos finales son: ", getpid());
    for (int i = 0; i < y; i++) {
        printf("%d", shm_ptr[x + i]);
        if (i < y - 1) printf(", ");
    }
    printf("\n");

    borrar_shm();
    return 0;
}