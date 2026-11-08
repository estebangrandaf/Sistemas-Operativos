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
            close(conector); // El hijo cierra el socket general porque solo le importa SU cliente
            
            printf("Proceso hijo (PID: %d) creado. Preparando transferencia...\n", getpid());
            
            
            close(nuevo_socket); // Cerramos la conexión con el cliente
            exit(EXIT_SUCCESS);  // Matamos al hijo para que no se quede zombie
        } else {
            close(nuevo_socket); // El padre cierra el socket del cliente y vuelve a escuchar
        }
    }

    close(conector);
    return 0;
}