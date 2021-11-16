# mmap

```
qemu-system-riscv64 -machine virt -bios none -kernel kernel/kernel -m 128M -smp 3 -nographic -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 -netdev user,id=net0,hostfwd=udp::29772-:2000 -object filter-dump,id=net0,netdev=net0,file=packets.pcap -device e1000,netdev=net0,bus=pcie.0

xv6 kernel is booting

virtio disk init 0
hart 2 starting
hart 1 starting
init: starting sh
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2025
xargstest.sh   2 3 99
cat            2 4 24152
echo           2 5 23016
forktest       2 6 13344
grep           2 7 27496
init           2 8 23768
kill           2 9 22992
ln             2 10 22968
ls             2 11 26392
mkdir          2 12 23104
rm             2 13 23088
sh             2 14 42040
stressfs       2 15 24016
usertests      2 16 121464
wc             2 17 25280
zombie         2 18 22504
nettests       2 19 36344
cowtest        2 20 30312
uthread        2 21 28888
call           2 22 23048
testsh         2 23 40848
kalloctest     2 24 27664
bcachetest     2 25 30432
alloctest      2 26 26632
bigfile        2 27 24712
mmaptest       2 28 34152
console        3 29 0
$ mmaptest
mmap_test starting
mmap_test OK
fork_test starting
fork_test OK
mmaptest: all tests succeeded
$ usertests
usertests starting
test reparent2: OK
test pgbug: OK
test sbrkbugs: usertrap(): unexpected scause 0x000000000000000c (instruction page fault) pid=3209
            sepc=0x00000000000044f8 stval=0x00000000000044f8
usertrap(): unexpected scause 0x000000000000000c (instruction page fault) pid=3210
            sepc=0x00000000000044f8 stval=0x00000000000044f8
OK
test badarg: OK
test reparent: OK
test twochildren: OK
test forkfork: OK
test forkforkfork: OK
test argptest: OK
test createdelete: OK
test linkunlink: OK
test linktest: OK
test unlinkread: OK
test concreate: OK
test subdir: OK
test fourfiles: OK
test sharedfd: OK
test exectest: OK
test bigargtest: OK
test bigwrite: OK
test bsstest: OK
test sbrkbasic: OK
test sbrkmuch: OK
test kernmem: usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6134
            sepc=0x0000000000000770 stval=0x0000000080000000
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6135
            sepc=0x0000000000000770 stval=0x000000008000c350
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6136
            sepc=0x0000000000000770 stval=0x00000000800186a0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6137
            sepc=0x0000000000000770 stval=0x00000000800249f0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6138
            sepc=0x0000000000000770 stval=0x0000000080030d40
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6139
            sepc=0x0000000000000770 stval=0x000000008003d090
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6140
            sepc=0x0000000000000770 stval=0x00000000800493e0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6141
            sepc=0x0000000000000770 stval=0x0000000080055730
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6142
            sepc=0x0000000000000770 stval=0x0000000080061a80
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6143
            sepc=0x0000000000000770 stval=0x000000008006ddd0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6144
            sepc=0x0000000000000770 stval=0x000000008007a120
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6145
            sepc=0x0000000000000770 stval=0x0000000080086470
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6146
            sepc=0x0000000000000770 stval=0x00000000800927c0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6147
            sepc=0x0000000000000770 stval=0x000000008009eb10
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6148
            sepc=0x0000000000000770 stval=0x00000000800aae60
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6149
            sepc=0x0000000000000770 stval=0x00000000800b71b0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6150
            sepc=0x0000000000000770 stval=0x00000000800c3500
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6151
            sepc=0x0000000000000770 stval=0x00000000800cf850
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6152
            sepc=0x0000000000000770 stval=0x00000000800dbba0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6153
            sepc=0x0000000000000770 stval=0x00000000800e7ef0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6154
            sepc=0x0000000000000770 stval=0x00000000800f4240
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6155
            sepc=0x0000000000000770 stval=0x0000000080100590
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6156
            sepc=0x0000000000000770 stval=0x000000008010c8e0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6157
            sepc=0x0000000000000770 stval=0x0000000080118c30
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6158
            sepc=0x0000000000000770 stval=0x0000000080124f80
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6159
            sepc=0x0000000000000770 stval=0x00000000801312d0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6160
            sepc=0x0000000000000770 stval=0x000000008013d620
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6161
            sepc=0x0000000000000770 stval=0x0000000080149970
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6162
            sepc=0x0000000000000770 stval=0x0000000080155cc0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6163
            sepc=0x0000000000000770 stval=0x0000000080162010
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6164
            sepc=0x0000000000000770 stval=0x000000008016e360
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6165
            sepc=0x0000000000000770 stval=0x000000008017a6b0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6166
            sepc=0x0000000000000770 stval=0x0000000080186a00
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6167
            sepc=0x0000000000000770 stval=0x0000000080192d50
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6168
            sepc=0x0000000000000770 stval=0x000000008019f0a0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6169
            sepc=0x0000000000000770 stval=0x00000000801ab3f0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6170
            sepc=0x0000000000000770 stval=0x00000000801b7740
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6171
            sepc=0x0000000000000770 stval=0x00000000801c3a90
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6172
            sepc=0x0000000000000770 stval=0x00000000801cfde0
usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6173
            sepc=0x0000000000000770 stval=0x00000000801dc130
OK
test sbrkfail: usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6181
            sepc=0x000000000000399c stval=0x0000000000010000
OK
test sbrkarg: OK
test validatetest: OK
test stacktest: usertrap(): unexpected scause 0x000000000000000d (load page fault) pid=6185
            sepc=0x00000000000007d8 stval=0x000000000000dc50
OK
test opentest: OK
test writetest: OK
test writebig: OK
test createtest: OK
test openiput: OK
test exitiput: OK
test iput: OK
test mem: OK
test pipe1: OK
test preempt: kill... wait... OK
test exitwait: OK
test rmdot: OK
test fourteen: OK
test bigfile: OK
test dirfile: OK
test iref: OK
test forktest: OK
test bigdir: OK
ALL TESTS PASSED
$                    
```
