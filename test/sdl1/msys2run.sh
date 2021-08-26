# gcc -I/mingw64/include/SDL2 $1.c -o $1 -L/mingw64/lib/ -lSDL2 -lSDL2main -lm
gcc -D__WINDOWS__ -I/mingw64/include/SDL2 $1.c -o $1 -L/mingw64/lib/ -lSDL2 -lSDL2main -lSDL2_image -lm
# ./$1
