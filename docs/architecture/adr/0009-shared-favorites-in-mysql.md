# ADR 0009: Favoritos compartilhados armazenados no próprio MySQL do usuário

## Status
Aceito (em uso).

## Contexto
O SequelFast permite salvar queries/filtros favoritos por tabela. O README menciona "shared favorites" — favoritos visíveis para outros usuários/máquinas que acessam o mesmo servidor.

## Decisão
Em vez de um backend próprio de sincronização, os favoritos compartilhados são gravados **dentro do próprio servidor MySQL/MariaDB de destino**, em um schema dedicado (`_SequelFast`) criado automaticamente na primeira detecção. A identificação de cada favorito é feita por uma **string composta** (`fav^host^schema^table^color^nome[^usuário]`), correlacionada por índice em arrays paralelos (`favName`/`favValue`) em vez de um ID estável.

## Consequências
- **Positivas**: sincronização "de graça" entre qualquer máquina/usuário que já tenha acesso ao mesmo servidor MySQL — nenhuma infraestrutura adicional (sem servidor próprio do SequelFast, sem conta na nuvem).
- **Negativas observadas**: a chave composta por string e a correlação por índice em arrays paralelos é frágil — colisões de caracteres separadores (`^`) ou mudanças de schema/tabela quebram a correlação; qualquer mutação de favorito exige recarregar a lista inteira em vez de uma atualização pontual por ID.

## Alternativas consideradas (inferidas)
- Um ID (UUID/auto-increment) real como chave de favorito: mais robusto, mas exigiria migrar o schema `_SequelFast` já em produção — troca de custo (migração) por robustez que aparentemente não foi feita ainda.
