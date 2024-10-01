#!/bin/bash

# Verificar argumentos
if [ $# -ne 1 ]; then
    echo "Uso: $0 <nome_do_pipe>"
    exit 1
fi

PIPE_NAME="$1"
echo "Agente de suporte iniciado, aguardando pipe $PIPE_NAME"

# Esperar até que o pipe seja criado (máximo 10 segundos)
for ((i=0; i<10; i++)); do
    if [ -p "$PIPE_NAME" ]; then
        echo "Pipe $PIPE_NAME encontrado. Iniciando processamento de pedidos."
        break
    fi
    echo "Esperando 1 segundo"
    sleep 1
done

# Verificar se o pipe existe
if [ ! -p "$PIPE_NAME" ]; then
    echo "Erro: O pipe $PIPE_NAME não existe."
    exit 1
fi

# Processar pedidos
while true; do
    if read pedido < "$PIPE_NAME"; then
        if [ "$pedido" = "quit" ]; then
            echo "Recebido comando para encerrar."
            break
       
        fi
    fi
done

echo "Agente de suporte encerrado."
