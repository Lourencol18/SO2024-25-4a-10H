#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define BSIZE 80
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
    snprintf(buf, BSIZE, "%d %d %s", aluno_inicial, num_alunos, student_fifo);
    write(fd, buf, strlen(buf) + 1);
    close(fd);

    // Abre o pipe de resposta para receber o número de alunos inscritos
    if ((fd_response = open(student_fifo, O_RDONLY)) < 0) {
        perror("open response fifo");
        unlink(student_fifo);
        exit(1);
    }
 
   // Recebe e faz a leitura
    int alunos_inscritos;
    read(fd_response, &alunos_inscritos, sizeof(int));
    close(fd_response);

    // Exibe informações finais
    printf("student %d: alunos inscritos=%d\n", nstud, alunos_inscritos);

    // Remove o pipe específico do student
    unlink(student_fifo);
    
    return 0;
}
