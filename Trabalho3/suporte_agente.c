#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define PIPE_SUPPORT "/tmp/suporte"
#define PIPE_ADMIN "/tmp/admin"
#define MAX_DISCIPLINES 10
#define MAX_SLOTS 5
#define MAX_STUDENTS 256
#define BUFFER_SIZE 512

// Estrutura para um slot de horário em uma disciplina
typedef struct {
    int vagas;
    int alunos[MAX_STUDENTS];
    int num_alunos;
    pthread_mutex_t lock;
} Slot;

// Estrutura para uma disciplina
typedef struct {
    Slot slots[MAX_SLOTS];
    pthread_mutex_t lock;
} Disciplina;

Disciplina disciplinas[MAX_DISCIPLINES];
int num_total_students = 0;
pthread_mutex_t students_lock = PTHREAD_MUTEX_INITIALIZER;

// Função auxiliar para ler do pipe
int le_pipe(int fd, char *buffer, int size) {
    int total = 0;
    while (total < size - 1) {
        int n = read(fd, buffer + total, 1);
        if (n <= 0) return n;
        if (buffer[total] == '\0') break;
        total++;
    }
    buffer[total] = '\0';
    return total;
}

// Inicializa as estruturas de dados
void init_disciplines() {
    for (int d = 0; d < MAX_DISCIPLINES; d++) {
        pthread_mutex_init(&disciplinas[d].lock, NULL);
        for (int s = 0; s < MAX_SLOTS; s++) {
            disciplinas[d].slots[s].vagas = 20;  // Número fixo de vagas por slot
            disciplinas[d].slots[s].num_alunos = 0;
            pthread_mutex_init(&disciplinas[d].slots[s].lock, NULL);
        }
    }
}

// Tenta inscrever um aluno em uma disciplina
int inscreve_aluno(int aluno, int disciplina) {
    if (disciplina < 0 || disciplina >= MAX_DISCIPLINES) return -1;

    pthread_mutex_lock(&disciplinas[disciplina].lock);
    
    for (int s = 0; s < MAX_SLOTS; s++) {
        pthread_mutex_lock(&disciplinas[disciplina].slots[s].lock);
        
        if (disciplinas[disciplina].slots[s].num_alunos < disciplinas[disciplina].slots[s].vagas) {
            disciplinas[disciplina].slots[s].alunos[disciplinas[disciplina].slots[s].num_alunos] = aluno;
            disciplinas[disciplina].slots[s].num_alunos++;
            
            pthread_mutex_unlock(&disciplinas[disciplina].slots[s].lock);
            pthread_mutex_unlock(&disciplinas[disciplina].lock);
            
            return s;  // Retorna o número do slot
        }
        
        pthread_mutex_unlock(&disciplinas[disciplina].slots[s].lock);
    }
    
    pthread_mutex_unlock(&disciplinas[disciplina].lock);
    return -1;  // Não há vagas disponíveis
}

// Thread para processar pedidos do admin
void *admin_thread(void *arg) {
    printf("[admin_thread] Iniciada\n");
    
    int fd_admin = open(PIPE_ADMIN, O_RDONLY);
    if (fd_admin == -1) {
        perror("[admin_thread] Erro ao abrir pipe admin");
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int len = le_pipe(fd_admin, buffer, BUFFER_SIZE);
        if (len <= 0) continue;

        char *token = strtok(buffer, ",");
        if (!token) continue;
        
        int op = atoi(token);
        switch (op) {
            case 1: {  // Consultar horários
                char *aluno_str = strtok(NULL, ",");
                char *resp_pipe = strtok(NULL, ",");
                if (!aluno_str || !resp_pipe) continue;
                
                int aluno = atoi(aluno_str);
                char response[BUFFER_SIZE];
                snprintf(response, BUFFER_SIZE, "%d", aluno);
                
                for (int d = 0; d < MAX_DISCIPLINES; d++) {
                    pthread_mutex_lock(&disciplinas[d].lock);
                    for (int s = 0; s < MAX_SLOTS; s++) {
                        pthread_mutex_lock(&disciplinas[d].slots[s].lock);
                        for (int i = 0; i < disciplinas[d].slots[s].num_alunos; i++) {
                            if (disciplinas[d].slots[s].alunos[i] == aluno) {
                                char temp[32];
                                snprintf(temp, sizeof(temp), ",%d/%d", d, s);
                                strcat(response, temp);
                            }
                        }
                        pthread_mutex_unlock(&disciplinas[d].slots[s].lock);
                    }
                    pthread_mutex_unlock(&disciplinas[d].lock);
                }
                
                int fd = open(resp_pipe, O_WRONLY);
                if (fd != -1) {
                    write(fd, response, strlen(response) + 1);
                    close(fd);
                }
                break;
            }
            // ... outros casos do switch continuam iguais
        }
    }
    return NULL;
}

// Processa um pedido de inscrição
void processa_pedido(char *msg) {
    char *aluno_str = strtok(msg, ",");
    char *disc_str = strtok(NULL, ",");
    char *resp_pipe = strtok(NULL, ",");
    
    if (!aluno_str || !disc_str || !resp_pipe) {
        printf("[support_agent] Mensagem inválida: %s\n", msg);
        return;
    }
    
    int aluno = atoi(aluno_str);
    int disciplina = atoi(disc_str);
    
    int resultado = inscreve_aluno(aluno, disciplina);
    
    int fd = open(resp_pipe, O_WRONLY);
    if (fd != -1) {
        char response[32];
        snprintf(response, sizeof(response), "%d", resultado);
        write(fd, response, strlen(response) + 1);
        close(fd);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <num_alunos>\n", argv[0]);
        exit(1);
    }

    init_disciplines();
    
    // Criar thread para processar pedidos admin
    pthread_t admin_tid;
    if (pthread_create(&admin_tid, NULL, admin_thread, NULL) != 0) {
        perror("Erro ao criar thread admin");
        exit(1);
    }

    // Loop principal para processar pedidos dos alunos
    while (1) {
        int fd = open(PIPE_SUPPORT, O_RDONLY);
        if (fd == -1) {
            perror("Erro ao abrir pipe de suporte");
            sleep(1);
            continue;
        }

        char buffer[BUFFER_SIZE];
        int len = le_pipe(fd, buffer, BUFFER_SIZE);
        close(fd);

        if (len > 0) {
            processa_pedido(buffer);
        }
    }

    return 0;
}