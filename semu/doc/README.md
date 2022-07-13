# semu

本專案 fork 自 https://github.com/jserv/semu.git

## clone

```
$ git clone https://github.com/jserv/semu.git
Cloning into 'semu'...
remote: Enumerating objects: 20, done.
remote: Counting objects: 100% (20/20), done.
remote: Compressing objects: 100% (16/16), done.
remote: Total 20 (delta 5), reused 17 (delta 3), pack-reused 0 
Receiving objects: 100% (20/20), 11.98 KiB | 2.40 MiB/s, done. 
Resolving deltas: 100% (5/5), done.
$ cd semu
```

## 

## build & run

```
$ make
cc -O2 -Wall -o semu semu.c -lpthread
$ make check
cc -O2 -Wall -o semu semu.c -lpthread
scripts/download.sh
Downloading...
kernel.bin: OK
fs.img: OK
./semu kernel.bin fs.img

xv6 kernel is booting

init: starting sh
$ ls
ls
.              1 1 1024
..             1 1 1024
README         2 2 2059
cat            2 3 25016
echo           2 4 23800
forktest       2 5 13688
grep           2 6 28568
init           2 7 24896
kill           2 8 23752
ln             2 9 23704
ls             2 10 27320
mkdir          2 11 23856
rm             2 12 23840
sh             2 13 43824
stressfs       2 14 24864
usertests      2 15 159064
grind          2 16 39424
wc             2 17 26208
zombie         2 18 23224
console        3 19 0
$ wc README
wc README
45 295 2059 README
$ cat README
cat README
xv6 is a re-implementation of Dennis Ritchie's and Ken Thompson's Unix
Version 6 (v6).  xv6 loosely follows the structure and style of v6,
but is implemented for a modern RISC-V multiprocessor using ANSI C.

ACKNOWLEDGMENTS

xv6 is inspired by John Lions's Commentary on UNIX 6th Edition 
(Peer
to Peer Communications; ISBN: 1-57398-013-7; 1st edition (June 
14,
2000)). See also https://pdos.csail.mit.edu/6.828/, which
provides pointers to on-line resources for v6.

The following people have made contributions: Russ Cox (contet 
 switching,
locking), Cliff Frey (MP), Xiao Yu (MP), Nickolai Zeldovich, and Austin
Clements.

We are also grateful for the bug reports and patches contributed by
Silas Boyd-Wickizer, Anton Burtsev, Dan Cross, Cody Cutler, Mike CAT,
Tej Chajed, Asami Doi, eyalz800, , Nelson Elhage, Saar Ettinger, Alice
Ferrazzi, Nathaniel Filardo, Peter Froehlich, Yakir Goaron,Shivam
Handa, Bryan Henry, jaichenhengjie, Jim Huang, Alexander Kapshuk,
Anders Kaseorg, kehao95, Wolfgang Keller, Jonathan Kimmitt, Eddie
Kohler, Austin Liew, Imbar Marinescu, Yandong Mao, Matan Shabtay,
Hitoshi Mitake, Carmi Merimovich, Mark Morrissey, mtasm, Joel Nider,
Greg Price, Ayan Shafqat, Eldar Sehayek, Yongming Shen, Fumiya 
Shigemitsu, Takahiro, Cam Tenny, tyfkda, Rafael Ubal, Warren Toomey,
Stephen Tu, Pablo Ventura, Xi Wang, Keiichi Watanabe, Nicolas
Wolovick, wxdao, Grant Wu, Jindong Zhang, Icenowy Zheng, and Zou Chang
Wei.

The code in the files that constitute xv6 is
Copyright 2006-2020 Frans Kaashoek, Robert Morris, and Russ Cox.

ERROR REPORTS

Please send errors and suggestions to Frans Kaashoek and Robert Morris
(kaashoek,rtm@mit.edu). The main purpose of xv6 is as a teaching
operating system for MIT's 6.S081, so we are more interested insimplifications and clarifications than new features.

BUILDING AND RUNNING XV6

You will need a RISC-V "newlib" tool chain from
https://github.com/riscv/riscv-gnu-toolchain, and qemu compiled for
riscv64-softmmu. Once they are installed, and in your shell    
search path, you can run "make qemu".
$ 
```