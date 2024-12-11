#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define PIPE_ADMIN "/tmp/admin"
#define PIPE_ADMIN_RESP "/tmp/admin_resp"
#define BUFFER_SIZE 2048  // Aumentado para garantir espaço suficiente
#define MAX_FILENAME 256

int main() {
    int fd_admin, fd_response;
    char mensagem[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    char nome_arquivo[MAX_FILENAME];
    int escolha, num_aluno;

    // Criar pipe de resposta se não existir
    mkfifo(PIPE_ADMIN_RESP, 0666);

    printf("Menu de Operações - Admin\n");
    printf("1. Consultar horários de um aluno\n");
    printf("2. Gravar em arquivo\n");
    printf("3. Terminar o agente\n");
    printf("0. Sair\n");

    while (1) {
        printf("\nEscolha uma opção: ");
        if (scanf("%d", &escolha) != 1) {
            printf("Entrada inválida. Tente novamente.\n");
            while (getchar() != '\n'); // Limpa o buffer
            continue;
        }

        // Abre o pipe do admin
        fd_admin = open(PIPE_ADMIN, O_WRONLY);
        if (fd_admin == -1) {
            perror("[admin] Erro ao abrir o pipe principal");
            continue;
        }

        switch (escolha) {
            case 1: // Consultar horários de um aluno
                printf("Digite o número do aluno: ");
                scanf("%d", &num_aluno);

                // Montar mensagem para o support_agent
                int n = snprintf(mensagem, sizeof(mensagem), "1,%d,%s", num_aluno, PIPE_ADMIN_RESP);
                if (n < 0 || n >= sizeof(mensagem)) {
                    printf("[admin] Erro: mensagem muito longa\n");
                    close(fd_admin);
                    break;
                }
                write(fd_admin, mensagem, strlen(mensagem) + 1);
                close(fd_admin);

                // Ler a resposta
                fd_response = open(PIPE_ADMIN_RESP, O_RDONLY);
                read(fd_response, buffer, BUFFER_SIZE - 1);
                buffer[BUFFER_SIZE - 1] = '\0';  // Garante terminação
                close(fd_response);
                
                printf("[admin] Horários do aluno %d: %s\n", num_aluno, buffer);
                break;

            case 2: // Gravar em arquivo
                printf("Digite o nome do arquivo para gravação: ");
                scanf("%s", nome_arquivo);
                nome_arquivo[MAX_FILENAME - 1] = '\0';  // Garante terminação

                // Enviar mensagem para o support_agent
                n = snprintf(mensagem, sizeof(mensagem), "2,%s,%s", nome_arquivo, PIPE_ADMIN_RESP);
                if (n < 0 || n >= sizeof(mensagem)) {
                    printf("[admin] Erro: nome do arquivo muito longo\n");
                    close(fd_admin);
                    break;
                }
                write(fd_admin, mensagem, strlen(mensagem) + 1);
                close(fd_admin);

                // Ler a resposta
                fd_response = open(PIPE_ADMIN_RESP, O_RDONLY);
                read(fd_response, buffer, BUFFER_SIZE - 1);
                buffer[BUFFER_SIZE - 1] = '\0';  // Garante terminação
                close(fd_response);

                int num_alunos = atoi(buffer);
                if (num_alunos >= 0) {
                    printf("[admin] Dados salvos no arquivo %s. Total de alunos: %d\n", 
                           nome_arquivo, num_alunos);
                } else {
                    printf("[admin] Erro ao salvar no arquivo %s\n", nome_arquivo);
                }
                break;

            case 3: // Terminar o agente
                n = snprintf(mensagem, sizeof(mensagem), "3,%s", PIPE_ADMIN_RESP);
                if (n < 0 || n >= sizeof(mensagem)) {
                    printf("[admin] Erro: mensagem muito longa\n");
                    close(fd_admin);
                    break;
                }
                write(fd_admin, mensagem, strlen(mensagem) + 1);
                close(fd_admin);

                // Ler confirmação
                fd_response = open(PIPE_ADMIN_RESP, O_RDONLY);
                read(fd_response, buffer, BUFFER_SIZE - 1);
                buffer[BUFFER_SIZE - 1] = '\0';  // Garante terminação
                close(fd_response);

                printf("[admin] Operação: Terminar o support_agent.\n");
                unlink(PIPE_ADMIN_RESP);
                exit(0);

            case 0: // Sair
                close(fd_admin);
                unlink(PIPE_ADMIN_RESP);
                printf("Encerrando o admin.\n");
                exit(0);

            default:
                printf("Opção inválida. Tente novamente.\n");
                close(fd_admin);
                break;
        }
    }

    return 0;
}