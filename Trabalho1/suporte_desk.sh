#!/bin/bash

# Verificar argumentos
if [ $# -ne 2 ]; then
    echo "Uso: $0 <nome_do_pipe> <numero_de_students>"
    exit 1
fi

PIPE_NAME="$1"
NUM_STUDENTS="$2"
CURRENT_DIR="/home/a22305861/SO2024-25-4a-10H/Trabalho1"

echo "Iniciando suporte_desk.sh com pipe $PIPE_NAME e $NUM_STUDENTS students"
echo "Criando pipe $PIPE_NAME"

# Criar o named pipe se não existir
if [ ! -p "$PIPE_NAME" ]; then
    mkfifo "$PIPE_NAME"
fi

echo "Executando suporte_agente.sh"
# Executar suporte_agente em background
./suporte_agente.sh "$PIPE_NAME" &

echo "Executando $NUM_STUDENTS instâncias de student"
# Executar students em background
for ((i=1; i<=$NUM_STUDENTS; i++)); do
    echo "Executando student $i"
    "$CURRENT_DIR/student" "$PIPE_NAME" "Pedido$i" &
done

# Esperar 1 segundo
echo "Esperando 1 segundo"
sleep 1

# Enviar 'quit' para o pipe
echo "Enviando 'quit' para o pipe"
echo "quit" > "$PIPE_NAME"

# Esperar todos os processos terminarem
echo "Esperando processos terminarem"
wait

# Remover o named pipe
echo "Removendo pipe $PIPE_NAME"
rm "$PIPE_NAME"

echo "Todos os processos terminaram. O pipe foi removido."
