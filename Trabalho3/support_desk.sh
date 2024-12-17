#!/bin/bash

# Definir caminhos para pipes e arquivo de lock
PIPE_SUPPORT="/tmp/suporte"
PIPE_ADMIN="/tmp/admin"
LOCKFILE="/tmp/suporte_desk.lock"

# Capturar sinais de interrupção para limpeza
trap 'rm -f $PIPE_SUPPORT /tmp/student_* $PIPE_ADMIN $LOCKFILE; pkill -f support_agent; exit' INT TERM EXIT

# Validar número de argumentos
if [ $# -ne 4 ]; then
     echo "Uso: $0 <NALUN> <NDISCIP> <NLUG> <NSTUD>"
     exit 1
fi

# Capturar parâmetros de entrada
NALUN=$1     # Número de alunos
NDISCIP=$2   # Número de disciplinas
NLUG=$3      # Número de lugares
NSTUD=$4     # Número de processos de estudantes

# Remover pipes antigos para evitar conflitos
echo "Removendo pipes antigos..."
if [ -e "$PIPE_SUPPORT" ]; then
     rm -f "$PIPE_SUPPORT"
     echo "Pipe principal existente removido: $PIPE_SUPPORT"
fi

if [ -e "$PIPE_ADMIN" ]; then
     rm -f "$PIPE_ADMIN"
     echo "Pipe admin existente removido: $PIPE_ADMIN"
fi

# Remover pipes individuais de estudantes
for i in $(seq 0 $((NSTUD-1))); do
     if [ -e "/tmp/student_$i" ]; then
         rm -f "/tmp/student_$i"
         echo "Pipe do estudante removido: /tmp/student_$i"
     fi
done

# Criar pipe principal
echo "Criando pipe principal ($PIPE_SUPPORT)..."
mkfifo "$PIPE_SUPPORT" || { echo "Erro ao criar $PIPE_SUPPORT"; exit 1; }
chmod 0666 "$PIPE_SUPPORT"

# Criar pipe admin
echo "Criando pipe admin ($PIPE_ADMIN)..."
if [ ! -p "$PIPE_ADMIN" ]; then
     mkfifo "$PIPE_ADMIN" || { echo "Erro ao criar $PIPE_ADMIN"; exit 1; }
fi
chmod 0666 "$PIPE_ADMIN"

# Iniciar agente de suporte
echo "Iniciando o agente de suporte..."
./support_agent $NALUN & 
sleep 1  # Pequeno delay para garantir inicialização

# Calcular distribuição de alunos por processo
total_processes=5
students_per_process=$((NSTUD / total_processes))
if [ $((NSTUD % total_processes)) -ne 0 ]; then
     students_per_process=$((students_per_process + 1))
fi

# Lançar processos de estudantes
for process_id in $(seq 0 $((total_processes-1))); do
     start_student=$((process_id * students_per_process))
     if [ $start_student -lt $NSTUD ]; then
         echo "Iniciando processo de alunos $process_id: inicial=$start_student, total=$students_per_process"
         ./student $process_id $start_student $students_per_process &
         sleep 0.1
     fi
done

# Aguardar conclusão de todos os processos
wait

# Limpeza final 
echo "Limpando pipes..."
rm -f "$PIPE_SUPPORT"