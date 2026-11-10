#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PUERTO 9999
#define COLA_CLIENTES 5

int main() {
    int conector, nuevo_socket;
    struct sockaddr_in servidor_addr, cliente_addr;
    socklen_t cliente_len = sizeof(cliente_addr);

    // Crear el socket (AF_INET para IPv4, SOCK_STREAM para TCP)
    conector = socket(AF_INET, SOCK_STREAM, 0);
    if (conector < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&servidor_addr, 0, sizeof(servidor_addr));
    servidor_addr.sin_family = AF_INET;
    servidor_addr.sin_addr.s_addr = INADDR_ANY; 
    servidor_addr.sin_port = htons(PUERTO);

    // Asignar la dirección al socket (bind)
    if (bind(conector, (struct sockaddr *)&servidor_addr, sizeof(servidor_addr)) < 0) {
        perror("Error en bind");
        close(conector);
        exit(EXIT_FAILURE);
    }

    // Poner el servidor en modo escucha
    if (listen(conector, COLA_CLIENTES) < 0) {
        perror("Error en listen");
        close(conector);
        exit(EXIT_FAILURE);
    }

    printf("Servidor escuchando en el puerto %d...\n", PUERTO);

    // Bucle infinito para atender clientes
    while(1) {
        nuevo_socket = accept(conector, (struct sockaddr *)&cliente_addr, &cliente_len);
        if (nuevo_socket < 0) {
            perror("Error en accept");
            continue; 
        }
        
        printf("¡Cliente conectado!\n");
        
        //  Crear hijo para que atienda al cliente
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Error al hacer fork");
            close(nuevo_socket);
            continue;
        }
        
        if (pid == 0) {
            close(conector); // Al hijo solo le interesa su cliente, no el socket principal
            
            printf("Hijo %d atendiendo peticion...\n", getpid());
            
            // Abrimos el archivo en modo lectura
            FILE *archivo = fopen("Google.html", "r");
            if (archivo == NULL) {
                perror("Error: No se encuentra Google.html en la carpeta");
                close(nuevo_socket);
                exit(EXIT_FAILURE);
            }
            
            char buffer[1024];
            size_t bytes_leidos;
            
            // Leemos y enviamos en bloques de 1KB para no saturar la memoria
            while ((bytes_leidos = fread(buffer, 1, sizeof(buffer), archivo)) > 0) {
                if (write(nuevo_socket, buffer, bytes_leidos) < 0) {
                    perror("Error enviando el archivo");
                    break;
                }
            }
            
            printf("Archivo transferido. Cerrando conexion.\n");
            fclose(archivo);
            close(nuevo_socket);
            exit(EXIT_SUCCESS); 
        } else {
            close(nuevo_socket); // El padre cierra el socket del cliente y vuelve a escuchar
        }
    }

    close(conector);
    return 0;
}