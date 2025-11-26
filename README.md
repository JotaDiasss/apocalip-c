# apocalip-c (simple)

Projeto simples em C inspirado em Vampire Survivors. Implementação minimalista para exercício escolar.

Características
- Usa `cli_lib` (pequeno wrapper sobre SDL2 incluído neste repositório) para janela, desenho e entrada.
- Conceitos demonstrados: structs, ponteiros, alocação dinâmica (malloc/free), listas encadeadas, matrizes (Grid), leitura/escrita de arquivo (`scores.txt`).
- Cross-platform: Windows (MSYS2 MinGW) e Linux (pkg-config / libsdl2-dev).

Como compilar (MSYS2 MinGW64 - Windows)
# apocalip-c

Jogo simples em C inspirado por Vampire Survivors. Usa SDL2 e um pequeno wrapper (`cli_lib`) que fornece desenho básico, input e texto via `SDL2_ttf` quando disponível.

Requisitos
- SDL2 (dev headers e libs)
- SDL2_ttf (opcional, para texto na janela)
- GCC (MinGW/MSYS2 no Windows ou GCC no Linux)

Compilar (Windows PowerShell com MinGW/MSYS2) — exemplo rápido:
```powershell
cd 'c:\Users\jotaa\OneDrive\Documentos\Vscode\apocalip-c'
gcc main.c cli_lib.c -o apocalip.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -O2 -Wall -Wextra
```

Compilar (Linux):
```bash
gcc main.c cli_lib.c -o apocalip -lSDL2 -lSDL2_ttf -O2 -Wall -Wextra
```

Executar
- Windows: `.
apocalip.exe`
# apocalip-c

Jogo simples em C inspirado por Vampire Survivors. Usa SDL2 e um pequeno wrapper (`cli_lib`) que fornece desenho básico, input e texto via `SDL2_ttf` quando disponível.

Requisitos
- SDL2 (dev headers e libs)
- SDL2_ttf (opcional, para texto na janela)
- GCC (MinGW/MSYS2 no Windows ou GCC no Linux)

Compilar (Windows PowerShell com MinGW/MSYS2) — exemplo rápido:
```powershell
cd 'c:\Users\jotaa\OneDrive\Documentos\Vscode\apocalip-c'
gcc main.c cli_lib.c -o apocalip.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -O2 -Wall -Wextra
```

Compilar (Linux):
```bash
gcc main.c cli_lib.c -o apocalip -lSDL2 -lSDL2_ttf -O2 -Wall -Wextra
```

Executar
- Windows: `.
apocalip.exe`
- Linux: `./apocalip`

Controles
- `WASD`: mover
- Mouse: mirar
- Clique esquerdo: atirar
- `Space`: dash (3s cooldown)

Persistência
- Pontuações são salvas em `scores.txt` no formato: `<nome> <pontos> <onda>` (append).

Arquivos principais
- `main.c`: lógica do jogo (entrada, movimento, tiros, inimigos, ondas, HUD, pódio)
- `cli_lib.c` / `cli_lib.h`: wrapper mínimo sobre SDL2 e SDL2_ttf para facilitar desenho e entrada

Git
- Adicionado `.gitignore` para binários, artefatos de build e arquivos de editor. Principais entradas:
	- `apocalip.exe`, `apocalip`, `main` (binários)
	- `*.o`, `*.obj`, `*.exe`, `*.dll`, `*.lib`, `*.a`
	- `.vscode/`, `*.code-workspace`
	- arquivos temporários: `*~`, `*.swp`, `*.bak`

Observações
- Os comentários do código foram removidos por solicitação do autor.
- Nomes longos são truncados na tela do pódio para evitar problemas de formatação; `scores.txt` mantém o nome completo.

Licença
- Use por conta e risco; este é um projeto educativo.
