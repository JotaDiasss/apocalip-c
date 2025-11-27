apocalip.exe`
apocalip.exe`
# apocalip-c

Jogo simples em C inspirado por Vampire Survivors.

Resumo
- Movimento com `WASD`, mira com o mouse e tiro com clique esquerdo.
- Dash com `SPACE` (cooldown 3s) e invulnerabilidade temporária após acerto.
- Ondas de inimigos (10 por onda), HP = número da onda, inimigos crescem por onda.
- Pontuação salva em `scores.txt` e pódio exibido ao final.
- Conta função de salvar o nome do jogador, salvando assim sua pontuação e jogando em uma tabela de acordo com sua pontuação dentro de cada "run".

Requisitos
- Compilador C (GCC/MinGW/WIN64). Use MSYS2 no Windows para facilitar.
- SDL2 e SDL2_ttf instalados no sistema.
- Uma fonte TTF no mesmo diretório (ex.: `DejaVuSans.ttf`). O jogo tenta usar `DejaVuSans.ttf` como fallback.

Instalação (Windows MSYS2)
- Abra o terminal MSYS2 MinGW 64-bit e execute:

```powershell
pacman -Syu ; pacman -Su
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf
```

Instalação (Debian/Ubuntu)

```bash
sudo apt update
sudo apt install build-essential libsdl2-dev libsdl2-ttf-dev
```

Gameplay
- https://github.com/user-attachments/assets/a91e1bbb-3aad-42e9-a3f4-13661ba143c0



Compilar
- Compilação direta (Windows PowerShell/MSYS2):

```powershell
cd 'c:\Users\jotaa\OneDrive\Documentos\Vscode\apocalip-c'
gcc main.c cli_lib.c -o apocalip.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -O2 -Wall -Wextra
```

- Compilar em Linux:

```bash
gcc main.c cli_lib.c -o apocalip -lSDL2 -lSDL2_ttf -O2 -Wall -Wextra
```

Executar
- Windows (PowerShell / MSYS2):

```powershell
.\apocalip.exe
```

- Linux:

```bash
./apocalip
```

Arquivo de scores
- As pontuações são registradas em `scores.txt` no mesmo diretório do executável. Cada linha segue o formato:
  `Nome Pontos Onda` (ex.: `Joao 1234 5`).

Notas e solução de problemas
- Se houver erro de link, verifique se as bibliotecas SDL2 e SDL2_ttf estão instaladas e que seu `PATH`/`LD_LIBRARY_PATH` inclui as dlls/so.
- Se a fonte não for encontrada, coloque um `DejaVuSans.ttf` no diretório do jogo ou modifique o código para usar outra fonte instalada.
- Para compilar no Windows sem MSYS2, adapte as flags de link conforme seu ambiente (Visual Studio exige projeto diferente).

.gitignore
- Há um `.gitignore` incluído para ignorar binários, arquivos objeto e configurações do editor.

Contribuições
- Pull requests são bem-vindos para ajustes de jogabilidade, correções e portabilidade.

Licença
- Código fornecido sem licença explícita; adapte conforme necessário.
