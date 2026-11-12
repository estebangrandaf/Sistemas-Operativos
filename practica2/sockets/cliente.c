#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PUERTO 9999
#define IP_SERVIDOR "127.0.0.1" // Nos conectamos a nuestro propio ordenador de momento

int main() {
    int socket_cliente;
    struct sockaddr_in direccion_servidor;

    // Crear el enchufe (socket) del cliente
    socket_cliente = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_cliente < 0) {
        perror("Error: No se pudo crear el socket del cliente");
        exit(EXIT_FAILURE);
    }

    // Configurar a donde nos vamos a conectar
    memset(&direccion_servidor, 0, sizeof(direccion_servidor));
    direccion_servidor.sin_family = AF_INET;
    direccion_servidor.sin_port = htons(PUERTO);
    
    // Convertir la IP a formato de red para que el socket la entienda
    if (inet_pton(AF_INET, IP_SERVIDOR, &direccion_servidor.sin_addr) <= 0) {
        perror("Error: IP no valida o no soportada");
        close(socket_cliente);
        exit(EXIT_FAILURE);
    }

    // Conectar al servidor
    printf("Conectando al servidor en %s:%d...\n", IP_SERVIDOR, PUERTO);
    if (connect(socket_cliente, (struct sockaddr *)&direccion_servidor, sizeof(direccion_servidor)) < 0) {
        perror("Error de conexion. Comprueba que el servidor esta encendido");
        close(socket_cliente);
        exit(EXIT_FAILURE);
    }

    printf("¡Conectado con exito al servidor!\n");

    // Creamos un archivo nuevo para guardar lo que nos llegue
    FILE *archivo = fopen("Google_recibido.html", "w");
    if (archivo == NULL) {
        perror("Error al crear el archivo de destino");
        close(socket_cliente);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    ssize_t bytes_recibidos;

    printf("Recibiendo datos del servidor...\n");

    // Leemos del enchufe en bloques de 1KB y los escribimos en nuestro archivo
    while ((bytes_recibidos = read(socket_cliente, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, bytes_recibidos, archivo);
    }

    if (bytes_recibidos < 0) {
        perror("Error al leer los datos del socket");
    } else {
        printf("Archivo descargado correctamente como 'Google_recibido.html'\n");
    }

    fclose(archivo);
    
    close(socket_cliente);
    return 0;
}