#!/bin/bash
# Script para gerar compile_commands.json para o projeto Qt

# Limpa builds anteriores
make clean 2>/dev/null

# Gera o Makefile
qmake SequelFast.pro

# Gera compile_commands.json usando bear (se disponível) ou compdb
if command -v bear &> /dev/null; then
    bear -- make
elif command -v compdb &> /dev/null; then
    compdb -p . make
else
    echo "Instale 'bear' ou 'compdb' para gerar compile_commands.json:"
    echo "sudo apt install bear  # ou"
    echo "pip install compdb"
fi