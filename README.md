# Sistemas Operativos 2024/2025

Este repositório reúne três trabalhos práticos sobre comunicação entre processos
(IPC), processos concorrentes e sincronização em Linux. Os trabalhos representam
uma evolução de um sistema de suporte a inscrições de alunos:

| Trabalho | Tema principal | Componentes |
| --- | --- | --- |
| 1 | FIFO nomeado básico | `student.c`, `suporte_desk.sh`, `suporte_agente.sh` |
| 2 | Inscrições com resposta e threads | `student.c`, `suporte_agente.c`, `suporte_desk.sh` |
| 3 | Inscrições concorrentes e interface administrativa | `student.c`, `support_agent.c`, `admin.c`, `support_desk.sh` |

## Requisitos

- Linux ou outro sistema POSIX;
- `gcc`, `make` e Bash;
- permissões para criar FIFOs em `/tmp`;
- `pthread`, disponível na libc de sistemas Linux.

Os programas usam caminhos fixos em `/tmp`, nomeadamente `/tmp/suporte`,
`/tmp/admin` e `/tmp/student_*`. Os scripts removem estes pipes durante a
limpeza, por isso não devem ser executados simultaneamente com outra cópia do
sistema.

## Compilação

Os Makefiles existentes foram originalmente preparados para uma estrutura com
as pastas `src/`, `include/` e `lib/`. Atualmente, os ficheiros `.c` estão na
raiz de cada pasta, pelo que `make` termina com `gcc: fatal error: no input files`.
Até essa estrutura ser corrigida, use os comandos abaixo.

```bash
cd Trabalho1
gcc -Wall -Wextra -g student.c -o student
```

```bash
cd Trabalho2
gcc -Wall -Wextra -g student.c -o student
gcc -Wall -Wextra -g -pthread suporte_agente.c -o suporte_agente
```

```bash
cd Trabalho3
gcc -Wall -Wextra -g student.c -o student
gcc -Wall -Wextra -g -pthread support_agent.c -o support_agent
gcc -Wall -Wextra -g admin.c -o admin
```

Os comandos devem ser executados dentro do respetivo diretório. Os scripts
também esperam encontrar os executáveis nesse diretório.

## Trabalho 1

### Objetivo

Demonstrar a criação e utilização de um FIFO nomeado. Um processo `student`
envia uma mensagem para um pipe indicado na linha de comandos. O agente lê o
pipe e termina quando recebe a mensagem `quit`.

### Execução

Num terminal, a partir de `Trabalho1/`:

```bash
mkfifo /tmp/trabalho1_pipe
./suporte_agente.sh /tmp/trabalho1_pipe
```

Noutro terminal:

```bash
./student /tmp/trabalho1_pipe "Pedido1"
echo quit > /tmp/trabalho1_pipe
```

O `student` aceita exatamente dois argumentos:

```text
./student <nome_do_pipe> <mensagem>
```

O script `suporte_desk.sh` automatiza o cenário e aceita:

```text
./suporte_desk.sh <nome_do_pipe> <numero_de_students>
```

Exemplo:

```bash
./suporte_desk.sh /tmp/trabalho1_pipe 3
```

### Implementação

- `student.c` abre o FIFO em modo de escrita e envia a mensagem recebida;
- `suporte_agente.sh` espera pelo FIFO, lê pedidos e reconhece `quit`;
- `suporte_desk.sh` cria o FIFO, inicia o agente, lança os estudantes e envia
	o comando de término.

Este trabalho não implementa respostas aos estudantes nem gestão de vagas.

## Trabalho 2

### Objetivo

Implementar pedidos de inscrição através de um FIFO principal e respostas
através de um FIFO individual por estudante. O agente usa threads para tratar
os pedidos e um mutex para proteger a estrutura de disciplinas e horários.

### Execução

Depois de compilar, a partir de `Trabalho2/`:

```bash
./suporte_desk.sh <NALUN> <NDISCIP> <NLUG> <NSTUD>
```

Exemplo:

```bash
./suporte_desk.sh 20 10 2 4
```

Os parâmetros significam:

- `NALUN`: número total de alunos;
- `NDISCIP`: número de disciplinas informado ao script, mas não usado pelo
	programa;
- `NLUG`: número de lugares informado ao script, mas não usado pelo agente;
- `NSTUD`: número de processos `student` a lançar.

O script cria `/tmp/suporte` e os pipes `/tmp/student_<id>`, inicia o agente e
distribui os alunos pelos processos. Cada pedido tem o formato:

```text
<aluno_inicial> <numero_de_alunos> <pipe_de_resposta>
```

O agente responde com um inteiro que representa o número de alunos alocados.
As disciplinas são inicializadas no código com 100 disciplinas, 20 horários e
2 vagas por horário.

### Fluxo

1. O script cria o FIFO principal.
2. Cada processo `student` cria o seu FIFO de resposta.
3. O pedido é enviado para `/tmp/suporte`.
4. `suporte_agente.c` cria uma thread para o pedido.
5. A thread aloca vagas sob proteção de `pthread_mutex_t`.
6. O resultado é escrito no FIFO individual do estudante.
7. Os pipes temporários são removidos no fim da execução.

## Trabalho 3

### Objetivo

Expandir o sistema com inscrições individuais, escolha de horário, consulta
administrativa, exportação para ficheiro e término controlado do agente.

### Execução

Compile os três executáveis e, a partir de `Trabalho3/`, execute:

```bash
./support_desk.sh <NALUN> <NDISCIP> <NLUG> <NSTUD>
```

Exemplo:

```bash
./support_desk.sh 20 10 2 4
```

O script cria os FIFOs `/tmp/suporte` e `/tmp/admin`, inicia o agente e lança
os processos de estudantes. Para usar o menu administrativo, abra outro
terminal no mesmo diretório e execute:

```bash
./admin
```

O menu disponibiliza:

1. consultar os horários de um aluno;
2. gravar as inscrições num ficheiro CSV;
3. terminar o agente de suporte;
0. sair apenas do programa administrativo.

### IPC e concorrência

- `/tmp/suporte`: pedidos de inscrição dos estudantes;
- `/tmp/admin`: comandos enviados pelo administrador;
- `/tmp/student_<id>`: resposta individual para cada aluno;
- `/tmp/admin_resp`: resposta às operações administrativas;
- uma thread administrativa processa comandos do `admin`;
- cada pedido de inscrição é processado por uma thread própria;
- existe um mutex por disciplina para proteger as vagas e um mutex global para
	controlar o término do agente.

Cada aluno tenta realizar cinco inscrições em disciplinas diferentes. O agente
tem 10 disciplinas, 5 horários por disciplina e um limite de vagas configurável
por horário. O estudante envia `-1` quando não existe vaga e tenta novamente.
No fim, cada processo apresenta as inscrições realizadas no formato
`disciplina/horário`.

### Comandos administrativos

O agente interpreta mensagens com estes formatos:

```text
1,<id_aluno>,<pipe_de_resposta>
2,<nome_do_ficheiro>,<pipe_de_resposta>
3,<pipe_de_resposta>
```

O comando `1` devolve os horários do aluno. O comando `2` cria um CSV com uma
linha por aluno e uma coluna por disciplina. O comando `3` pede o término do
agente e devolve `Ok` quando o pedido é aceite.

## Limitações conhecidas

- Os três Makefiles não correspondem à estrutura atual do repositório e não
	compilam sem alteração.
- Os scripts e programas dependem de caminhos absolutos ou de executáveis no
	diretório corrente.
- O Trabalho 1 usa um caminho absoluto antigo para executar `student`, pelo
	que `suporte_desk.sh` pode precisar de ser ajustado noutra máquina.
- No Trabalho 2, `NDISCIP` e `NLUG` não configuram efetivamente o agente; os
	valores usados estão definidos em `suporte_agente.c`.
- No Trabalho 2, a distribuição de alunos usa divisão inteira e pode não
	distribuir o resto quando `NALUN` não é divisível por `NSTUD`.
- No Trabalho 3, `NDISCIP` também não altera o número de disciplinas, que está
	fixado em 10 no código.
- Os identificadores e FIFOs são previsíveis e os pipes têm permissões `0666`;
	esta implementação é adequada a um exercício académico, não a um serviço
	multiutilizador exposto.
- A limpeza depende dos scripts terminarem normalmente ou receberem sinais;
	uma interrupção forçada pode deixar FIFOs em `/tmp`.

## Limpeza manual

Se uma execução for interrompida, remova os recursos temporários antes de
voltar a executar os trabalhos:

```bash
rm -f /tmp/suporte /tmp/admin /tmp/admin_resp /tmp/student_*
```

Verifique os processos ativos antes de usar `pkill`, porque o nome do processo
pode coincidir com outra execução local.

## Estado da validação

Foi confirmada a compilação direta de todos os programas com `gcc`. O Trabalho
3 produz apenas um aviso de compilação sobre o parâmetro não utilizado da
thread administrativa. A compilação através dos Makefiles continua pendente de
correção da configuração `SRC := src`.
