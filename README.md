# Trabalhos de Sistemas Operativos

Implementação progressiva de um sistema de inscrições académicas, desenvolvido
no âmbito da unidade curricular de Sistemas Operativos.

O projeto explora comunicação entre processos, processos concorrentes, threads,
sincronização e operações com FIFOs nomeados em Linux.

## Evolução do projeto

| Trabalho | Descrição |
| --- | --- |
| **1** | Comunicação básica entre processos através de um FIFO. |
| **2** | Gestão de pedidos de inscrição com respostas individuais e threads. |
| **3** | Sistema completo com inscrições, controlo de vagas e interface administrativa. |

## Requisitos

- Linux ou outro sistema POSIX;
- GCC e Bash;
- suporte para `pthread` e FIFOs nomeados.

## Compilação

Os fontes estão na raiz de cada diretório. Execute os comandos correspondentes:

```bash
# Trabalho 1
cd Trabalho1
gcc -Wall -Wextra -g student.c -o student

# Trabalho 2
cd ../Trabalho2
gcc -Wall -Wextra -g student.c -o student
gcc -Wall -Wextra -g -pthread suporte_agente.c -o suporte_agente

# Trabalho 3
cd ../Trabalho3
gcc -Wall -Wextra -g student.c -o student
gcc -Wall -Wextra -g -pthread support_agent.c -o support_agent
gcc -Wall -Wextra -g admin.c -o admin
```

## Execução

### Trabalho 1

```bash
cd Trabalho1
./suporte_desk.sh /tmp/trabalho1_pipe 3
```

O script cria um FIFO, inicia o agente de suporte e lança três processos de
estudante.

### Trabalho 2

```bash
cd Trabalho2
./suporte_desk.sh 20 10 2 4
```

Os argumentos representam, respetivamente, o número de alunos, disciplinas,
vagas e processos de estudantes.

### Trabalho 3

```bash
cd Trabalho3
./support_desk.sh 20 10 2 4
```

Para abrir a interface administrativa, execute `./admin` noutro terminal,
dentro do mesmo diretório. É possível consultar horários, exportar inscrições
para CSV e terminar o agente de suporte.

## Organização

Cada trabalho está isolado no seu próprio diretório e inclui o código-fonte e o
script de execução correspondente. Os trabalhos utilizam FIFOs temporários em
`/tmp`, removidos no final das execuções normais.


