#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
 
void validar_argumentos(int argc, char *argv[], char **nombre_archivo, int *tamano_trozo) {
    if (argc != 3) {
        char msg[] = "Uso: ./hacha <archivo> <tamaño>\n";
        write(2, msg, strlen(msg));
        exit(1);
    }
    *nombre_archivo = argv[1];
    *tamano_trozo = atoi(argv[2]);
}

int obtener_num_trozos(char *nombre_archivo, int tamano_trozo) {
    struct stat info_archivo;
    
    if (stat(nombre_archivo, &info_archivo) == -1) {
        perror("Error al obtener info del archivo (stat)");
        exit(1);
    }
    
    int tamano_total = info_archivo.st_size;
    int trozos = tamano_total / tamano_trozo;
    
    if (tamano_total % tamano_trozo != 0) {
        trozos++; 
    }
    return trozos;
}

void hijo_escritor(int fd_lectura, char *nombre_origen, int num_trozo) {
    char nombre_destino[256];
    
    sprintf(nombre_destino, "%s.h%02d", nombre_origen, num_trozo);

    int fd_destino = creat(nombre_destino, 0666);
    if (fd_destino < 0) {
        perror("Error al crear el trozo en disco");
        exit(1);
    }

    char buffer[1024];
    int leidos;
    
    while ((leidos = read(fd_lectura, buffer, sizeof(buffer))) > 0) {
        if (write(fd_destino, buffer, leidos) != leidos) {
            perror("Error escribiendo en el archivo de destino");
            exit(1);
        }
    }

    close(fd_lectura);
    close(fd_destino);
    exit(0); 
}
void padre_lector(int fd_escritura, int fd_origen, int tamano_trozo) {
    char buffer[1024];
    int leidos;
    int total_leido_trozo = 0;
    int a_leer;

    while (total_leido_trozo < tamano_trozo) {
        a_leer = sizeof(buffer);
        if (tamano_trozo - total_leido_trozo < a_leer) {
            a_leer = tamano_trozo - total_leido_trozo;
        }

        leidos = read(fd_origen, buffer, a_leer);
        if (leidos <= 0) break; 

        if (write(fd_escritura, buffer, leidos) != leidos) {
            perror("Error al inyectar datos en la tubería");
            exit(1);
        }
        total_leido_trozo += leidos;
    }
    close(fd_escritura);
}
void dividir_archivo(char *nombre_archivo, int tamano_trozo, int num_trozos) {
    int fd_origen = open(nombre_archivo, O_RDONLY);
    if (fd_origen < 0) {
        perror("Error al abrir el archivo original");
        exit(1);
    }

    int fd_tuberia[2];

    for (int i = 0; i < num_trozos; i++) {
        if (pipe(fd_tuberia) == -1) {
            perror("Error al crear la tuberia");
            exit(1);
        }

        if (fork() == 0) {
            close(fd_tuberia[1]); 
            close(fd_origen);     
            hijo_escritor(fd_tuberia[0], nombre_archivo, i);
        } else {
            close(fd_tuberia[0]); 
            padre_lector(fd_tuberia[1], fd_origen, tamano_trozo);
            wait(NULL); 
        }
    }
    close(fd_origen);
}

int main(int argc, char *argv[]) {
    char *nombre_archivo;
    int tamano_trozo;

    validar_argumentos(argc, argv, &nombre_archivo, &tamano_trozo);
    
    int num_trozos = obtener_num_trozos(nombre_archivo, tamano_trozo);
    
    if (num_trozos > 0) {
        dividir_archivo(nombre_archivo, tamano_trozo, num_trozos);
    }
    return 0;
}