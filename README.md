# apocalip-c (simple)

Projeto simples em C inspirado em Vampire Survivors. Implementação minimalista para exercício escolar.

Características
- Usa `cli_lib` (pequeno wrapper sobre SDL2 incluído neste repositório) para janela, desenho e entrada.
- Conceitos demonstrados: structs, ponteiros, alocação dinâmica (malloc/free), listas encadeadas, matrizes (Grid), leitura/escrita de arquivo (`scores.txt`).
- Cross-platform: Windows (MSYS2 MinGW) e Linux (pkg-config / libsdl2-dev).

Como compilar (MSYS2 MinGW64 - Windows)
```bash
# abra "MSYS2 MinGW 64-bit"
pacman -Syu
pacman -Su
pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-SDL2 mingw-w64-x86_64-pkg-config
cd /c/Users/jotaa/OneDrive/Documentos/Vscode/apocalip-c
make
./apocalip
```

Como compilar (Linux Debian/Ubuntu)
```bash
sudo apt update
sudo apt install build-essential libsdl2-dev pkg-config
cd ~/path/to/apocalip-c
make
./apocalip
```

Controles
- `WASD`: mover
- Mouse: mirar; clique esquerdo atira

Salvar e high scores
- Ao terminar a run a pontuação é salva em `scores.txt` (append). Ao final o programa imprime um pódio com os top 3.

Limitações e notas
- Este projeto usa SDL2; instale as dependências conforme o seu sistema.
- A interface textual no HUD é mínima (evitei SDL_ttf para manter dependências baixas). Se quiser, posso adicionar `SDL2_ttf` para desenhar texto na janela.
# apocalip C

Um pequeno jogo em C inspirado em Vampire Survivors: movimento com WASD, tiros em direção ao mouse, inimigos zumbis surgindo e perseguindo o jogador.

Requisitos
- SDL2 (headers e libs) instalados. No Windows recomendo usar MSYS2.


Build (MSYS2 / Linux)
```bash
# MSYS2 (Windows) - open the "MSYS2 MinGW 64-bit" shell
pacman -Syu
pacman -Su
pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-SDL2 mingw-w64-x86_64-pkg-config mingw-w64-x86_64-make
cd /c/Users/jotaa/OneDrive/Documentos/Vscode/apocalip-c
make

# Linux (Debian/Ubuntu) - install SDL2 dev package and build
# sudo apt install build-essential libsdl2-dev pkg-config
cd ~/path/to/apocalip-c
make
```

Executar
```bash
./apocalip   # on Linux or MSYS2 the executable will be named 'apocalip' (Windows will create apocalip.exe)
```

Notas sobre o jogo
- Substituí a barra de pontuação por um timer de partida (60s por padrão).
- O jogo salva a maior pontuação em `highscore.txt` no diretório do jogo.
- Para ver a `highscore` atual, verifique a saída no console ao iniciar/terminar o jogo.
 - O jogo usa `SDL2_ttf` (se disponível) para desenhar o timer, score e highscore com texto.
 - Caso não haja fonte no diretório, o jogo tenta localizar fontes comuns em Linux e Windows.

Build scripts
- `build.sh`: script POSIX para Linux / MSYS2 (usa `pkg-config` quando disponível).
- `build.bat`: script para Windows que configura temporariamente `PATH` para MSYS2 mingw64 e compila.

Font
- O código tenta carregar `DejaVuSans.ttf` se estiver no diretório do jogo. Se não, tente instalar/usar uma fonte do sistema ou coloque uma `DejaVuSans.ttf` ao lado do executável.

Controles
- `WASD`: mover
- Mouse: mirar
- Left mouse button / auto-fire: atirar

Notas
- Código usa renderização de formas simples (retângulos). Para texto/score mais completo, adicione SDL_ttf.
- Ajuste constantes de velocidade, spawn e vida para balancear.
