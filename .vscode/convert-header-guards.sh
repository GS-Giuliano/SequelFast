#!/bin/bash

# Script para converter header guards tradicionais para #pragma once

echo "Converting header guards to #pragma once..."

# Lista dos arquivos .h no diretório src
files=(
    "src/statistics.h"
    "src/users.h" 
    "src/two_checkbox_delegate.h"
    "src/macroformatdialog.h"
    "src/structure.h"
    "src/restore.h"
    "src/sql.h"
    "src/macroinputdialog.h"
    "src/sqlhighlighter.h"
    "src/two_checkbox_list_model.h"
    "src/functions.h"
    "src/texteditcompleter.h"
    "src/tunnelsqlmanager.h"
)

for file in "${files[@]}"; do
    if [[ -f "$file" ]]; then
        echo "Processing $file..."
        
        # Extrair o nome do header guard baseado no nome do arquivo
        basename=$(basename "$file" .h)
        guard_name="${basename^^}_H"
        
        # Substituir #ifndef e #define pela #pragma once
        sed -i "1s/#ifndef $guard_name/#pragma once/" "$file"
        sed -i "2s/#define $guard_name//" "$file"
        
        # Remover linha vazia deixada pela remoção do #define
        sed -i '2{/^$/d}' "$file"
        
        # Remover #endif do final do arquivo
        sed -i "$ s/#endif.*$//" "$file"
        sed -i '$s/^$//' "$file"  # Remove linha vazia final se existir
        
        echo "✓ Converted $file"
    else
        echo "⚠ File not found: $file"
    fi
done

echo "Header guard conversion completed!"