#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

// Definições de constantes
#define PIPE_SUPPORT "/tmp/suporte"     // Caminho do pipe principal de comunicação
#define MAX_DISCIPLINES 10               // Número máximo de disciplinas
#define MAX_HORARIOS 5                  // Número máximo de horários
#define INSCRICOES_POR_ALUNO 5           // Limite de inscrições por aluno

int main(int argc, char *argv[])
{
    // Validar número de argumentos
    if (argc != 4)
    {
        // Mensagem de uso correto do programa
        fprintf(stderr, "Uso: %s <id_processo> <aluno_inicial> <total_alunos>\n", argv[0]);
        exit(1);
    }

    // Converter argumentos de linha de comando
    int id_processo = atoi(argv[1]);     // Identificador do processo
    int aluno_inicial = atoi(argv[2]);   // Primeiro aluno deste processo
    int total_alunos = atoi(argv[3]);    // Total de alunos neste processo

    // Imprimir informações sobre o processo de aluno
    printf("student %d: aluno inicial=%d, número de alunos=%d\n",
           id_processo, aluno_inicial, total_alunos);

    // Delay escalonado para evitar sobrecarga de inicialização
    // Multiplica o tempo de espera pelo ID do processo
    usleep(200000 * (id_processo + 1));

    // Inicializar gerador de números aleatórios
    // Usa combinação de tempo atual, ID do aluno e PID para garantir aleatoriedade
    srand(time(NULL) ^ (aluno_inicial + 1) ^ (getpid()));

    // Preparar nome do pipe individual para este aluno
    char pipe_aluno[256];
    snprintf(pipe_aluno, sizeof(pipe_aluno), "/tmp/student_%d", aluno_inicial);

    // Remover pipe existente (se houver)
    unlink(pipe_aluno);

    // Criar novo pipe para comunicação individual
    if (mkfifo(pipe_aluno, 0666) == -1)
    {
        perror("[aluno] Erro ao criar pipe");
        return 1;
    }

    // Controle de inscrições
    int disciplinas_inscritas[MAX_DISCIPLINES] = {0};  // Rastrear disciplinas já inscritas
    int inscricoes[INSCRICOES_POR_ALUNO][2] = {0};     // Armazenar detalhes de inscrições
    int num_inscricoes = 0;  // Contador de inscrições realizadas

    // Loop para realizar inscrições
    while (num_inscricoes < INSCRICOES_POR_ALUNO)
    {
        // Selecionar disciplina aleatoriamente
        int disciplina = rand() % MAX_DISCIPLINES;
        
        // Selecionar horário aleatoriamente
        int horario_desejado = rand() % MAX_HORARIOS;

        // Pular disciplina já inscrita
        if (disciplinas_inscritas[disciplina])
            continue;

        // Abrir pipe de suporte para enviar solicitação
        int fd_suporte = open(PIPE_SUPPORT, O_WRONLY);
        if (fd_suporte == -1)
        {
            perror("[aluno] Erro ao abrir pipe de suporte");
            continue;
        }

        // Preparar mensagem de inscrição com ID do aluno, disciplina e horário desejado
        char mensagem[512];
        snprintf(mensagem, sizeof(mensagem), "%d %d %d %s", 
                 aluno_inicial, disciplina, horario_desejado, pipe_aluno);
        
        // Enviar mensagem de inscrição
        write(fd_suporte, mensagem, strlen(mensagem) + 1);
        close(fd_suporte);

        // Abrir pipe individual para receber resposta
        int fd_aluno = open(pipe_aluno, O_RDONLY);
        int horario;
        
        // Ler horário alocado
        ssize_t bytes_read = read(fd_aluno, &horario, sizeof(horario));
        close(fd_aluno);
        
        // Pequeno delay para evitar sobrecarga
        sleep(0.1);

        // Processar resultado da inscrição
        if (bytes_read == sizeof(horario) && horario >= 0)
        {
            // Marcar disciplina como inscrita
            disciplinas_inscritas[disciplina] = 1;
            
            // Registrar detalhes da inscrição
            inscricoes[num_inscricoes][0] = disciplina;
            inscricoes[num_inscricoes][1] = horario;
            
            // Imprimir informações da inscrição
            printf("[aluno %d] Inscrito na disciplina %d, horário %d (%d/5)\n",
                   aluno_inicial, disciplina, horario, num_inscricoes + 1);
            
            num_inscricoes++;
            
            // Pequeno delay entre inscrições
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

    // Remover pipe individual
    unlink(pipe_aluno);
    return 0;
}