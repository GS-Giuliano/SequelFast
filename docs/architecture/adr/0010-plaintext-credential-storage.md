# ADR 0010: Armazenamento de credenciais em texto puro

## Status
**Dívida técnica identificada — recomenda-se revisão.** Documentado como ADR porque é uma decisão (implícita) que afeta diretamente a segurança de todos os usuários do app, não porque seja a recomendação deste mapeamento.

## Contexto
O SequelFast precisa lembrar credenciais de conexão (usuário/senha do MySQL e, quando aplicável, usuário/senha SSH) entre sessões, para reconectar com um clique.

## Decisão (como está implementada hoje)
Senhas de banco de dados e de SSH são gravadas **sem criptografia** na tabela `conns` do SQLite local (`preferences.db`). Quando a conexão usa túnel SSH com autenticação por senha, essa senha também é passada como **argumento de linha de comando** ao processo `sshpass` (ver [ADR 0004](0004-ssh-tunnel-via-external-process.md)), ficando visível para outros processos do mesmo usuário via `ps`/`/proc`.

## Consequências
- **Positivas (só do ponto de vista de simplicidade de implementação)**: nenhuma dependência de cofre de credenciais do SO, implementação trivial de salvar/carregar.
- **Negativas**: qualquer pessoa ou processo com acesso de leitura ao arquivo `preferences.db` do usuário (backup, sincronização em nuvem, outro usuário na mesma máquina, malware) obtém as senhas de banco e SSH em claro. A exposição via `ps` amplia isso para qualquer processo local durante a conexão, mesmo sem acesso ao arquivo.

## Recomendação
Considerar, em ordem de esforço crescente:
1. Passar a senha ao `sshpass` via variável de ambiente (`SSHPASS`) ou arquivo temporário com permissão restrita, em vez de argumento de linha de comando — elimina a exposição via `ps` imediatamente.
2. Integrar com o cofre de credenciais nativo do SO (Keychain no macOS, libsecret/KWallet no Linux, Credential Manager no Windows) para as senhas persistidas, via `QtKeychain` ou equivalente.
3. Se persistência própria continuar sendo necessária, cifrar o campo de senha no SQLite com uma chave derivada de um segredo por máquina/usuário (ex.: DPAPI no Windows, Keychain como cofre da chave nas demais plataformas).

## Alternativas consideradas (inferidas)
Não há evidência no código de que uma alternativa mais segura tenha sido avaliada e descartada — este parece ser o caminho mais simples adotado desde o início, ainda não revisitado.
