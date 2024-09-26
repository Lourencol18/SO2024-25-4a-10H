#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_MSG_LEN 100

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <nome_do_pipe> <mensagem>\n", argv[0]);
        exit(1);
    }

    const char *pipe_name = argv[1];
    const char *message = argv[2];

    // Abrir o named pipe
    int fd = open(pipe_name, O_WRONLY);
    if (fd == -1) {
        perror("Erro ao abrir o pipe");
        exit(1);
    }

    // Enviar a mensagem
    if (write(fd, message, strlen(message)) == -1) {
        perror("Erro ao escrever no pipe");
        close(fd);
        exit(1);
    }

    printf("Mensagem enviada: %s\n", message);

    // Fechar o pipe
    close(fd);

    return 0;
}
