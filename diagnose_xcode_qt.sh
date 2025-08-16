#!/usr/bin/env bash
set -euo pipefail

GREEN="$(tput setaf 2 2>/dev/null || true)"; RED="$(tput setaf 1 2>/dev/null || true)"
YELLOW="$(tput setaf 3 2>/dev/null || true)"; BOLD="$(tput bold 2>/dev/null || true)"
RESET="$(tput sgr0 2>/dev/null || true)"
FIX=${1:-}

headline(){ echo -e "\n${BOLD}$*${RESET}"; }
ok(){ echo -e "${GREEN}✔${RESET} $*"; }
warn(){ echo -e "${YELLOW}⚠${RESET} $*"; }
err(){ echo -e "${RED}✖${RESET} $*"; }

headline "Resumo do sistema"
echo "macOS: $(sw_vers -productVersion)  | Kernel: $(uname -a | awk '{print $1,$3,$12}')"
echo "Arquitetura: $(uname -m)  | Shell: $SHELL"

headline "1) Verificando seleção do Xcode/CLT (xcode-select)"
XCODE_DIR="$(/usr/bin/xcode-select -p 2>/dev/null || true)"
if [[ -z "${XCODE_DIR}" ]]; then
  err "xcode-select não aponta para nada."
  if [[ "${FIX}" == "--fix" ]]; then
    sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer || true
    XCODE_DIR="$(/usr/bin/xcode-select -p 2>/dev/null || true)"
  fi
else
  ok "xcode-select -> ${XCODE_DIR}"
fi

headline "2) Aceite de licença do Xcode"
if ! /usr/bin/xcodebuild -checkFirstLaunchStatus >/dev/null 2>&1; then
  warn "Licença do Xcode possivelmente não aceita."
  if [[ "${FIX}" == "--fix" ]]; then
    echo "Abrindo aceite de licença (exige interação no terminal com Page Down e 'agree')..."
    sudo /usr/bin/xcodebuild -license || true
  fi
else
  ok "Licença do Xcode OK."
fi

headline "3) Caminho do SDK ativo (xcrun)"
SDK_PATH="$(/usr/bin/xcrun --show-sdk-path 2>/dev/null || true)"
if [[ -z "${SDK_PATH}" || ! -d "${SDK_PATH}" ]]; then
  err "SDK do macOS não localizado via xcrun."
else
  ok "SDK ativo: ${SDK_PATH}"
fi

headline "4) Compiladores e versões"
CLANG_BIN="$(/usr/bin/xcrun --find clang 2>/dev/null || which clang || true)"
CLANGPP_BIN="$(/usr/bin/xcrun --find clang++ 2>/dev/null || which clang++ || true)"
echo "clang:   ${CLANG_BIN}"
echo "clang++: ${CLANGPP_BIN}"
if [[ -x "${CLANG_BIN}" ]]; then
  "${CLANG_BIN}" --version || true
else
  err "clang não encontrado."
fi

headline "5) Procurando o header 'type_traits'"
FOUND_HEADER=""
if [[ -n "${SDK_PATH:-}" ]]; then
  FOUND_HEADER="$(/usr/bin/find "${SDK_PATH}" -path '*/usr/include/c++/v1/type_traits' -print -quit 2>/dev/null || true)"
fi
if [[ -z "${FOUND_HEADER}" ]]; then
  # fallback em CLT
  if [[ -d "/Library/Developer/CommandLineTools" ]]; then
    FOUND_HEADER="$(/usr/bin/find /Library/Developer/CommandLineTools -path '*/usr/include/c++/v1/type_traits' -print -quit 2>/dev/null || true)"
  fi
fi
if [[ -n "${FOUND_HEADER}" ]]; then
  ok "Encontrado: ${FOUND_HEADER}"
else
  err "'type_traits' não foi encontrado no SDK/CLT."
  warn "Provável CLT ausente/corrompido OU xcode-select apontando errado."
  if [[ "${FIX}" == "--fix" ]]; then
    echo "Tentando corrigir: reconfigurar xcode-select e reinstalar CLT (se necessário)."
    if [[ -d "/Applications/Xcode.app" ]]; then
      sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer || true
    fi
    if [[ -d "/Library/Developer/CommandLineTools" ]]; then
      echo "Removendo CLT antigo..."
      sudo rm -rf /Library/Developer/CommandLineTools || true
    fi
    echo "Iniciando instalador dos Command Line Tools..."
    /usr/bin/xcode-select --install || true
    warn "Após instalar CLT, rode este script novamente."
  fi
fi

headline "6) Variáveis de ambiente que podem interferir"
for v in SDKROOT CPATH CPLUS_INCLUDE_PATH MACOSX_DEPLOYMENT_TARGET; do
  echo "$v=${!v-<não definido>}"
done

headline "7) Verificando conflitos no PATH (Homebrew gcc/llvm)"
echo "which -a clang:"
which -a clang || true
echo "which -a gcc:"
which -a gcc || true
if command -v brew >/dev/null 2>&1; then
  if brew list --versions llvm >/dev/null 2>&1; then
    warn "Pacote llvm do Homebrew detectado. Pode conflitar com headers/SDK."
  fi
else
  ok "Homebrew não encontrado ou não em PATH (sem conflitos óbvios)."
fi

headline "8) Teste de compilação mínima C++ (inclui <type_traits>)"
TMPDIR="$(mktemp -d)"
cat > "${TMPDIR}/test.cpp" <<'CPP'
#include <type_traits>
int main(){ static_assert(std::is_integral<int>::value, "ok"); return 0; }
CPP

if [[ -x "${CLANGPP_BIN:-}" ]]; then
  set +e
  "${CLANGPP_BIN}" -std=gnu++17 -isysroot "${SDK_PATH:-/}" "${TMPDIR}/test.cpp" -o "${TMPDIR}/a.out" 2> "${TMPDIR}/build.log"
  STATUS=$?
  set -e
  if [[ ${STATUS} -eq 0 ]]; then
    ok "Compilação de teste OK."
  else
    err "Falha ao compilar teste. Log:"
    cat "${TMPDIR}/build.log"
    warn "Se o erro for 'file not found: type_traits', volte às etapas 1–5."
  fi
else
  err "clang++ indisponível — não foi possível testar compilação."
fi

headline "Sugestões rápidas de correção manual (sem --fix)"
cat <<'TXT'
1) Reapontar para o Xcode recém-instalado:
   sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
2) Aceitar licença:
   sudo xcodebuild -license
3) Reinstalar Command Line Tools (se suspeita de corrupção):
   sudo rm -rf /Library/Developer/CommandLineTools
   xcode-select --install
4) Reconfigurar seu build CMake apontando explicitamente SDK e compiladores:
   cmake -S . -B build \
     -DCMAKE_C_COMPILER="$(xcrun --find clang)" \
     -DCMAKE_CXX_COMPILER="$(xcrun --find clang++)" \
     -DCMAKE_OSX_SYSROOT="$(xcrun --show-sdk-path)"
5) No Qt Creator (Preferences → Kits):
   - Remova Compilers antigos
   - Deixe o kit usar o clang/clang++ do Xcode atual
TXT

ok "Diagnóstico concluído."

