# 

* [在MSYS2上配置SDL2](https://blog.csdn.net/willian113/article/details/106538384)
* [msys2和SDL2环境搭建](https://dongqiceo.github.io/the-post-9982/)

```
Hero3C@DESKTOP-O093POU MINGW64 /c/ccc/riscv2os/riscv2os/test/sdl2msys2fail (master)
$ ./build.sh                                                                                                   
C:\Users\Hero3C\AppData\Local\Temp\ccFNXuPY.o:sdl1.c:(.text+0x15): undefined reference to `SDL_Init' 
C:\Users\Hero3C\AppData\Local\Temp\ccFNXuPY.o:sdl1.c:(.text+0x2b): undefined reference to `SDL_Delay'
C:\Users\Hero3C\AppData\Local\Temp\ccFNXuPY.o:sdl1.c:(.text+0x30): undefined reference to `SDL_Quit' 
collect2.exe: error: ld returned 1 exit status
```
