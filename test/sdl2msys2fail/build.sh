# gcc sdl1.c -o sdl -L/C/msys64/mingw64/lib -lmingw64 -mwindows -lSDL2main -lSDL2 -I/C/msys64/mingw64/include/SDL2
# gcc sdl1.c -o sdl1 -L/C/msys64/mingw64/lib -lmingw32 -mwindows -lSDL2main -lSDL2 -I/C/msys64/mingw64/include/SDL2
gcc -L/C/msys64/mingw64/lib -mwindows -lmingw32 -lSDL2main -lSDL2 -I/C/msys64/mingw64/include/SDL2 sdl1.c -o sdl1 
