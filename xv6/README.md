[中文電子書 -- 從 RISC-V 處理器到 UNIX 作業系統](https://github.com/riscv2os/riscv2os/wiki)

# xv6-riscv

本專案來源為 MIT 6.828 課程之 github ，網址如下：

* https://github.com/mit-pdos/xv6-riscv

原本該專案是在 Linux 下編譯的，但是我把它改成在 Windows 用 (windows + git bash + RISC-V gcc + qemu) 編譯。

方法是將 [mkfs.c](https://github.com/mit-pdos/xv6-riscv/tree/riscv/mkfs/mkfs.c) 修改為標準 C 語言，就可以編譯成功了。

## 在 Windows 下編譯本專案：

請先下載:

* [git-bash](https://git-scm.com/download/win)
* [FreedomStudio](https://www.sifive.com/software)

解開 FreedomStudio 壓縮檔後，將 windows 的 PATH 加入 `riscv64-unknown-elf-gcc/bin` and `riscv-qemu/bin` 這兩個路徑 (我電腦中的路徑如下)。

```
D:\install\FreedomStudio-2020-06-3-win64\SiFive\riscv64-unknown-elf-gcc-8.3.0-2020.04.1\bin

D:\install\FreedomStudio-2020-06-3-win64\SiFive\riscv-qemu-4.2.0-2020.04.0\bin
```

然後請用 git-bash 編譯建置你的專案。(我用 Visual Studio Code 中的終端機)


## 在 Windows 建置執行

```
user@DESKTOP-96FRN6B MINGW64 /d/ccc109/co2os/code/04
$ make qemu
qemu-system-riscv64 -machine virt -bios none -kernel kernel/kernel -m 256M -smp 3 -nographic -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

xv6 kernel is booting

hart 2 starting
hart 1 starting
init: starting sh
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2102
cat            2 3 23944
echo           2 4 22776
forktest       2 5 13112
grep           2 6 27256
init           2 7 23904
kill           2 8 22744
ln             2 9 22728
ls             2 10 26152
mkdir          2 11 22864
rm             2 12 22848
sh             2 13 41800
stressfs       2 14 23776
usertests      2 15 151184
grind          2 16 38008
wc             2 17 25040
zombie         2 18 22272
console        3 19 0
$ 
$ QEMU 4.2.0 monitor - type 'help' for more information // 按下 Ctrl-A-C 會進入 QEMU
(qemu) quit
```


## Linux 中的建置方法

請按照下列指引去做：

* https://github.com/mit-pdos/xv6-riscv/

## 授權 License

本專案來源如下

* https://github.com/mit-pdos/xv6-riscv 

請遵照下列授權使用之

* https://github.com/mit-pdos/xv6-riscv/blob/riscv/LICENSE



## 原本的 xv6-riscv 之 README

xv6 is a re-implementation of Dennis Ritchie's and Ken Thompson's Unix
Version 6 (v6).  xv6 loosely follows the structure and style of v6,
but is implemented for a modern RISC-V multiprocessor using ANSI C.

ACKNOWLEDGMENTS

xv6 is inspired by John Lions's Commentary on UNIX 6th Edition (Peer
to Peer Communications; ISBN: 1-57398-013-7; 1st edition (June 14,
2000)). See also https://pdos.csail.mit.edu/6.828/, which
provides pointers to on-line resources for v6.

The following people have made contributions: Russ Cox (context switching,
locking), Cliff Frey (MP), Xiao Yu (MP), Nickolai Zeldovich, and Austin
Clements.

We are also grateful for the bug reports and patches contributed by
Silas Boyd-Wickizer, Anton Burtsev, Dan Cross, Cody Cutler, Mike CAT,
Tej Chajed, eyalz800, Nelson Elhage, Saar Ettinger, Alice Ferrazzi,
Nathaniel Filardo, Peter Froehlich, Yakir Goaron,Shivam Handa, Bryan
Henry, Jim Huang, Alexander Kapshuk, Anders Kaseorg, kehao95, Wolfgang
Keller, Eddie Kohler, Austin Liew, Imbar Marinescu, Yandong Mao, Matan
Shabtay, Hitoshi Mitake, Carmi Merimovich, Mark Morrissey, mtasm, Joel
Nider, Greg Price, Ayan Shafqat, Eldar Sehayek, Yongming Shen, Cam
Tenny, tyfkda, Rafael Ubal, Warren Toomey, Stephen Tu, Pablo Ventura,
Xi Wang, Keiichi Watanabe, Nicolas Wolovick, wxdao, Grant Wu, Jindong
Zhang, Icenowy Zheng, and Zou Chang Wei.

The code in the files that constitute xv6 is
Copyright 2006-2019 Frans Kaashoek, Robert Morris, and Russ Cox.

ERROR REPORTS

Please send errors and suggestions to Frans Kaashoek and Robert Morris
(kaashoek,rtm@mit.edu). The main purpose of xv6 is as a teaching
operating system for MIT's 6.828, so we are more interested in
simplifications and clarifications than new features.

BUILDING AND RUNNING XV6

You will need a RISC-V "newlib" tool chain from
https://github.com/riscv/riscv-gnu-toolchain, and qemu compiled for
riscv64-softmmu. Once they are installed, and in your shell
search path, you can run "make qemu".
