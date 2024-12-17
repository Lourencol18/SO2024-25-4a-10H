#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#define PIPE_SUPPORT "/tmp/suporte"
#define MAX_DISCIPLINES 10
#define MAX_HORARIOS 5
#define INSCRICOES_POR_ALUNO 5

int main(int argc, char *argv[])
{
    // Validar argumentos
    if (argc != 4)
    {
        fprintf(stderr, "Uso: %s <id_processo> <aluno_inicial> <total_alunos>\n", argv[0]);
        exit(1);
    }

    // Converter argumentos
    int id_processo = atoi(argv[1]);
    int aluno_inicial = atoi(argv[2]);
    int total_alunos = atoi(argv[3]);

    printf("student %d: aluno inicial=%d, número de alunos=%d\n",
           id_processo, aluno_inicial, total_alunos);

    // Delay escalonado
    usleep(200000 * (id_processo + 1));

    // Inicializar semente randômica
    srand(time(NULL) ^ (aluno_inicial + 1) ^ (getpid()));

    // Preparar pipe do aluno
    char pipe_aluno[256];
    snprintf(pipe_aluno, sizeof(pipe_aluno), "/tmp/student_%d", aluno_inicial);

    // Criar pipe
    unlink(pipe_aluno);
    if (mkfifo(pipe_aluno, 0666) == -1)
    {
        perror("[aluno] Erro ao criar pipe");
        return 1;
    }

    // Controle de inscrições
    int disciplinas_inscritas[MAX_DISCIPLINES] = {0};
    int inscricoes[INSCRICOES_POR_ALUNO][2] = {0};
    int num_inscricoes = 0;

    // Loop de inscrições
    while (num_inscricoes < INSCRICOES_POR_ALUNO)
    {
        // Escolher disciplina e horário aleatórios
        int disciplina = rand() % MAX_DISCIPLINES;
        int horario_desejado = rand() % MAX_HORARIOS;  // Novo: horário aleatório

        if (disciplinas_inscritas[disciplina])
            continue;

        // Abrir pipe de suporte
        int fd_suporte = open(PIPE_SUPPORT, O_WRONLY);
        if (fd_suporte == -1)
        {
            perror("[aluno] Erro ao abrir pipe de suporte");
            continue;
        }

        // Enviar mensagem com horário desejado
        char mensagem[512];
        snprintf(mensagem, sizeof(mensagem), "%d %d %d %s", 
                 aluno_inicial, disciplina, horario_desejado, pipe_aluno);
        write(fd_suporte, mensagem, strlen(mensagem) + 1);
        close(fd_suporte);

        // Receber resposta
        int fd_aluno = open(pipe_aluno, O_RDONLY);
        int horario;
        ssize_t bytes_read = read(fd_aluno, &horario, sizeof(horario));
        close(fd_aluno);
        sleep(0.1);

        // Processar resultado
        if (bytes_read == sizeof(horario) && horario >= 0)
        {
            disciplinas_inscritas[disciplina] = 1;
            inscricoes[num_inscricoes][0] = disciplina;
            inscricoes[num_inscricoes][1] = horario;
            
            printf("[aluno %d] Inscrito na disciplina %d, horário %d (%d/5)\n",
                   aluno_inicial, disciplina, horario, num_inscricoes + 1);
            
            num_inscricoes++;
            usleep(50000);
        }
    }

    // Imprimir resumo das inscrições
    if (num_inscricoes > 0)
    {
        printf("student %d, aluno %d:", id_processo, aluno_inicial);
        for (int i = 0; i < num_inscricoes; i++)
        {
            printf("%s%d/%d", i == 0 ? " " : ", ", 
                   inscricoes[i][0], inscricoes[i][1]);
        }
        printf("\n");
    }

    // Limpar pipe
    unlink(pipe_aluno);
    return 0;
}