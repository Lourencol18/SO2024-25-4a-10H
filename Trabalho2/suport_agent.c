#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
    const char *pipe;
    if (argc == 2) {
        pipe = argv[1];
    } else {
        pipe = "/tmp/support_pipe";
    }

    FILE *pipeFile = fopen(pipe, "r");
    if (pipeFile == NULL) {
        perror("Erro ao abrir o pipe");
        return 1;
    }

    char request[256];
    srand(time(NULL)); // Seed para gerar números aleatórios

    while (fgets(request, sizeof(request), pipeFile) != NULL) {
        // Remover o newline do final da string, se existir
        request[strcspn(request, "\n")] = 0;

        if (strcmp(request, "quit") == 0) {
            printf("Support Agent: quit\n");
            break;
        }

        printf("Support Agent processing: %s\n", request);

        // Simula o tempo de processamento (1 a 5 segundos)
        int delay = 1 + rand() % 5;
        sleep(delay);
    }

    fclose(pipeFile);
    printf("Support Agent: read request terminated\n");

    return 0;
}
