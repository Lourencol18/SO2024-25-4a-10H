#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

int main(int argc, char *argv[]) {
    const char *pipe = "/tmp/support_pipe";
    char buffer[256];
    ssize_t bytesRead;
    int fd;

    // Define o pipe com base no argumento passado ou usa o padrão "/tmp/support_pipe"
    if (argc == 2) {
        pipe = argv[1];
    }

    // Abre o pipe para leitura
    fd = open(pipe, O_RDONLY);
    if (fd == -1) {
        perror("Erro ao abrir o pipe");
        return 1;
    }

    // Seed para o gerador de números aleatórios
    srand(time(NULL));

    // Lê e processa as solicitações
    while ((bytesRead = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';  // Termina a string lida

        // Verifica se a solicitação é "quit"
        if (strcmp(buffer, "quit\n") == 0) {
            printf("Support Agent: quit\n");
            break;
        }

        printf("Support Agent processing: %s", buffer);

        // Simula o tempo de processamento com uma pausa aleatória entre 1 e 5 segundos
        int sleepTime = 1 + rand() % 5;
        sleep(sleepTime);
    }

    if (bytesRead == -1) {
        perror("Erro ao ler o pipe");
    }

    close(fd);
    printf("Support Agent: read request terminated\n");

    return 0;
}
