#!/bin/bash

# Script para abrir arquivos .ui no Qt Designer
# Uso: ./open-ui-file.sh arquivo.ui

if [ $# -eq 0 ]; then
    echo "Uso: $0 <arquivo.ui>"
    exit 1
fi

UI_FILE="$1"

# Verifica se o arquivo existe
if [ ! -f "$UI_FILE" ]; then
    echo "Erro: Arquivo '$UI_FILE' não encontrado"
    exit 1
fi

# Caminho para o Qt Designer
DESIGNER_PATH="$(dirname "$0")/../Qt-link/6.9.1/gcc_64/bin/designer"

# Verifica se o Qt Designer existe
if [ ! -x "$DESIGNER_PATH" ]; then
    echo "Erro: Qt Designer não encontrado em '$DESIGNER_PATH'"
    exit 1
fi

# Abre o arquivo no Qt Designer
echo "Abrindo '$UI_FILE' no Qt Designer..."
"$DESIGNER_PATH" "$UI_FILE" &

echo "Qt Designer foi iniciado em background."