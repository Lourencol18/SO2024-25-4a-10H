#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#define PIPE_SUPPORT "/tmp/suporte"
#define MAX_DISCIPLINES 10
#define DISCIPLINES_PER_STUDENT 5
#define BUFFER_SIZE 512  // Aumentado para garantir espaço
#define MAX_RESPONSE 32

typedef struct {
    int discipline;
    int slot;
} Registration;

// Function to generate random disciplines
void get_random_disciplines(int *disciplines) {
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
    
    int used[MAX_DISCIPLINES] = {0};
    int count = 0;
    
    while (count < DISCIPLINES_PER_STUDENT) {
        int disc = rand() % MAX_DISCIPLINES;
        if (!used[disc]) {
            disciplines[count++] = disc;
            used[disc] = 1;
        }
    }
}

int le_resposta(int fd, char *buffer, int size) {
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

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <student_id> <initial_student> <num_students>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int student_id = atoi(argv[1]);
    int initial_student = atoi(argv[2]);
    int num_students = atoi(argv[3]);

    printf("student %d: aluno inicial=%d, número de alunos=%d\n", 
           student_id, initial_student, num_students);

    char student_pipe[BUFFER_SIZE];
    int n = snprintf(student_pipe, sizeof(student_pipe), "/tmp/student_%d", student_id);
    if (n < 0 || n >= sizeof(student_pipe)) {
        fprintf(stderr, "Error: student pipe name too long\n");
        exit(EXIT_FAILURE);
    }

    // Create student pipe
    unlink(student_pipe);
    if (mkfifo(student_pipe, 0666) == -1) {
        perror("[student] mkfifo");
        exit(EXIT_FAILURE);
    }

    // Process each student
    for (int current_student = initial_student; 
         current_student < initial_student + num_students; 
         current_student++) {
        
        int disciplines[DISCIPLINES_PER_STUDENT];
        Registration registrations[DISCIPLINES_PER_STUDENT];
        get_random_disciplines(disciplines);

        // Try to register each discipline
        for (int i = 0; i < DISCIPLINES_PER_STUDENT; i++) {
            char msg[BUFFER_SIZE];
            n = snprintf(msg, sizeof(msg), "%d,%d,%s", 
                        current_student, disciplines[i], student_pipe);
            if (n < 0 || n >= sizeof(msg)) {
                fprintf(stderr, "Error: message too long\n");
                continue;
            }

            int fd = open(PIPE_SUPPORT, O_WRONLY);
            if (fd == -1) {
                perror("[student] Error opening support pipe");
                continue;
            }
            write(fd, msg, strlen(msg) + 1);
            close(fd);

            // Wait for response
            fd = open(student_pipe, O_RDONLY);
            if (fd == -1) {
                perror("[student] Error opening response pipe");
                continue;
            }
            
            char response[MAX_RESPONSE];
            if (le_resposta(fd, response, sizeof(response)) > 0) {
                registrations[i].discipline = disciplines[i];
                registrations[i].slot = atoi(response);
            } else {
                registrations[i].discipline = disciplines[i];
                registrations[i].slot = -1;
            }
            close(fd);
        }

        // Print results
        printf("student %d, aluno %d: ", student_id, current_student);
        for (int i = 0; i < DISCIPLINES_PER_STUDENT; i++) {
            printf("%d/%d%s", 
                   registrations[i].discipline, 
                   registrations[i].slot,
                   i < DISCIPLINES_PER_STUDENT - 1 ? ", " : "\n");
        }
    }

    unlink(student_pipe);
    return 0;
}