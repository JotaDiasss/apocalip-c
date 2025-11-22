# apocalip C

Um pequeno jogo em C inspirado em Vampire Survivors: movimento com WASD, tiros em direção ao mouse, inimigos zumbis surgindo e perseguindo o jogador.

Requisitos
- SDL2 (headers e libs) instalados. No Windows recomendo usar MSYS2.

Build (MSYS2)
```powershell
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2
cd /c/Users/jotaa/OneDrive/Documentos/Vscode/apocalip-c
make
```

Executar
```powershell
./apocalip
```

Controles
- `WASD`: mover
- Mouse: mirar
- Left mouse button / auto-fire: atirar

Notas
- Código usa renderização de formas simples (retângulos). Para texto/score mais completo, adicione SDL_ttf.
- Ajuste constantes de velocidade, spawn e vida para balancear.
