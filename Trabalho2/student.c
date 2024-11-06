#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define BSIZE 128  // Aumente o tamanho para evitar o truncamento
#define NOMEFIFO "/tmp/suporte"  // Pipe comum para comunicação com support_agent

int main(int argc, char const *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <nstud> <aluno_inicial> <num_alunos>\n", argv[0]);
        exit(1);
    }

    int nstud = atoi(argv[1]);
    int aluno_inicial = atoi(argv[2]);
    int num_alunos = atoi(argv[3]);
    char student_fifo[BSIZE];
    int fd, fd_response;
    char buf[BSIZE];

    // Exibe informações iniciais
    printf("student %d: aluno inicial=%d, número de alunos=%d\n", nstud, aluno_inicial, num_alunos);

    // Criação do pipe nomeado específico para este student
    snprintf(student_fifo, BSIZE, "/tmp/student_%d", nstud);
    if (mkfifo(student_fifo, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        exit(1);
    }

    // Abre o pipe para comunicação com o support_agent
    if ((fd = open(NOMEFIFO, O_WRONLY)) < 0) {
        perror("open");
        unlink(student_fifo);
        exit(1);
    }

    // Envia o pedido para o support_agent no formato "aluno_inicial num_alunos student_fifo"
    int len = snprintf(buf, BSIZE, "%d %d %s", aluno_inicial, num_alunos, student_fifo);
    if (len >= BSIZE) {
        fprintf(stderr, "Aviso: mensagem truncada\n");
    }
    write(fd, buf, strlen(buf) + 1);
    close(fd);

    // Abre o pipe de resposta para receber o número de alunos inscritos como string
    if ((fd_response = open(student_fifo, O_RDONLY)) < 0) {
        perror("open response fifo");
        unlink(student_fifo);
        exit(1);
    }

    // Recebe e faz a leitura da resposta como string
    char alunos_inscritos_str[BSIZE];
    ssize_t bytes_read = read(fd_response, alunos_inscritos_str, BSIZE - 1);
    if (bytes_read > 0) {
        alunos_inscritos_str[bytes_read] = '\0';  // Adiciona o terminador de string
    }
    close(fd_response);

    // Exibe a resposta final recebida como string
    printf("student %d: alunos inscritos=%s\n", nstud, alunos_inscritos_str);

    // Remove o pipe específico do student
    unlink(student_fifo);
    
    return 0;
}
