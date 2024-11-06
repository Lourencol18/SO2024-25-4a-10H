#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define FIFO_IN "/tmp/suporte"
#define BSIZE 256

int main() {
    char msg_in[BSIZE];
    char msg_out[BSIZE];

    // Abre o named pipe para leitura
    int fd = open(FIFO_IN, O_RDONLY);
    if (fd == -1) {
        perror("Erro ao abrir o pipe");
        return 1;
    }

    while (1) {
        // Lê a mensagem do pipe
        ssize_t bytes_read = read(fd, msg_in, BSIZE);
        if (bytes_read > 0) {
            msg_in[bytes_read] = '\0';  // Garante que a string termina em '\0'
            printf("Mensagem recebida: %s\n", msg_in);

            // Extrai o nome do pipe de resposta do conteúdo da mensagem recebida
            // (Considerando que o nome do pipe está no final da mensagem recebida)
            char nome_resp[BSIZE];
            sscanf(msg_in, "%*s %*s %s", nome_resp); // Ignora os primeiros dois parâmetros e lê o nome do pipe de resposta

            // Abre o named pipe de resposta para o student
            int fd_out = open(nome_resp, O_WRONLY);
            if (fd_out == -1) {
                perror("Erro ao abrir o pipe de resposta");
                continue;
            }

            // Envia a resposta
            snprintf(msg_out, BSIZE, "Resposta do Support Agent");
            write(fd_out, msg_out, strlen(msg_out) + 1);

            // Fecha o pipe de resposta
            close(fd_out);
        } else if (bytes_read == 0) {
            break;  // o pipe foi fechado
        } else {
            perror("Erro ao ler do pipe");
            break;
        }
    }

    close(fd);
    printf("Support Agent: Encerrado\n");

    return 0;
}
