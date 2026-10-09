#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void validar_argumentos(int argc, char *argv[], int *x, int *y) {
    if (argc != 3) {
        printf("Uso: %s <filas_x> <columnas_y>\n", argv[0]);
        exit(1);
    }
    *x = atoi(argv[1]);
    *y = atoi(argv[2]);
}

void generar_rama(int nivel_actual, int profundidad_max) {
    if (nivel_actual < profundidad_max) {
        if (fork() > 0) { 
            wait(NULL);   
            exit(0);
        } else {
            generar_rama(nivel_actual + 1, profundidad_max); 
        }
    } else {
        sleep(5);
        exit(0);
    }
}

void ejecutar_pstree() {
    sleep(1); 
    if (fork() == 0) {
        char pid_str[20];
        sprintf(pid_str, "%d", getppid()); 
        execlp("pstree", "pstree", "-c", "-p", pid_str, NULL);
        perror("Error al ejecutar pstree");
        exit(1);
    } else {
        wait(NULL); 
    }
}

void recolectar_hijos(int cantidad) {
    for (int c = 0; c < cantidad; c++) {
        wait(NULL);
    }
}

int main(int argc, char *argv[]) {
    int x, y;
    
    validar_argumentos(argc, argv, &x, &y);

    for (int c = 0; c < y; c++) {
        if (fork() == 0) {
            generar_rama(1, x); 
        }
    }

    ejecutar_pstree();
    recolectar_hijos(y);
    
    return 0;
}