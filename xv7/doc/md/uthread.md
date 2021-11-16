# uthread

```
qemu-system-riscv64 -machine virt -bios none -kernel kernel/kernel -m 128M -smp 3 -nographic -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0 -netdev user,id=net0,hostfwd=udp::29772-:2000 -object filter-dump,id=net0,netdev=net0,file=packets.pcap -device e1000,netdev=net0,bus=pcie.0

xv6 kernel is booting

virtio disk init 0
hart 1 starting
hart 2 starting
init: starting sh
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2025
xargstest.sh   2 3 99
cat            2 4 24048
echo           2 5 22904
forktest       2 6 13224
grep           2 7 27384
init           2 8 23656
kill           2 9 22872
ln             2 10 22864
ls             2 11 26272
mkdir          2 12 22984
rm             2 13 22976
sh             2 14 41928
stressfs       2 15 23912
usertests      2 16 121360
wc             2 17 25168
zombie         2 18 22384
nettests       2 19 36232
cowtest        2 20 30200
uthread        2 21 28768
call           2 22 22936
testsh         2 23 40744
kalloctest     2 24 27552
bcachetest     2 25 30320
alloctest      2 26 26512
bigfile        2 27 24592
console        3 28 0
$ uthread
thread_a started
thread_b started
thread_c started
thread_c 0
thread_a 0
thread_b 0
thread_c 1
thread_a 1
thread_b 1
thread_c 2
thread_a 2
thread_b 2
thread_c 3
thread_a 3
thread_b 3
thread_c 4
thread_a 4
thread_b 4
thread_c 5
thread_a 5
thread_b 5
thread_c 6
thread_a 6
thread_b 6
thread_c 7
thread_a 7
thread_b 7
thread_c 8
thread_a 8
thread_b 8
thread_c 9
thread_a 9
thread_b 9
thread_c 10
thread_a 10
thread_b 10
thread_c 11
thread_a 11
thread_b 11
thread_c 12
thread_a 12
thread_b 12
thread_c 13
thread_a 13
thread_b 13
thread_c 14
thread_a 14
thread_b 14
thread_c 15
thread_a 15
thread_b 15
thread_c 16
thread_a 16
thread_b 16
thread_c 17
thread_a 17
thread_b 17
thread_c 18
thread_a 18
thread_b 18
thread_c 19
thread_a 19
thread_b 19
thread_c 20
thread_a 20
thread_b 20
thread_c 21
thread_a 21
thread_b 21
thread_c 22
thread_a 22
thread_b 22
thread_c 23
thread_a 23
thread_b 23
thread_c 24
thread_a 24
thread_b 24
thread_c 25
thread_a 25
thread_b 25
thread_c 26
thread_a 26
thread_b 26
thread_c 27
thread_a 27
thread_b 27
thread_c 28
thread_a 28
thread_b 28
thread_c 29
thread_a 29
thread_b 29
thread_c 30
thread_a 30
thread_b 30
thread_c 31
thread_a 31
thread_b 31
thread_c 32
thread_a 32
thread_b 32
thread_c 33
thread_a 33
thread_b 33
thread_c 34
thread_a 34
thread_b 34
thread_c 35
thread_a 35
thread_b 35
thread_c 36
thread_a 36
thread_b 36
thread_c 37
thread_a 37
thread_b 37
thread_c 38
thread_a 38
thread_b 38
thread_c 39
thread_a 39
thread_b 39
thread_c 40
thread_a 40
thread_b 40
thread_c 41
thread_a 41
thread_b 41
thread_c 42
thread_a 42
thread_b 42
thread_c 43
thread_a 43
thread_b 43
thread_c 44
thread_a 44
thread_b 44
thread_c 45
thread_a 45
thread_b 45
thread_c 46
thread_a 46
thread_b 46
thread_c 47
thread_a 47
thread_b 47
thread_c 48
thread_a 48
thread_b 48
thread_c 49
thread_a 49
thread_b 49
thread_c 50
thread_a 50
thread_b 50
thread_c 51
thread_a 51
thread_b 51
thread_c 52
thread_a 52
thread_b 52
thread_c 53
thread_a 53
thread_b 53
thread_c 54
thread_a 54
thread_b 54
thread_c 55
thread_a 55
thread_b 55
thread_c 56
thread_a 56
thread_b 56
thread_c 57
thread_a 57
thread_b 57
thread_c 58
thread_a 58
thread_b 58
thread_c 59
thread_a 59
thread_b 59
thread_c 60
thread_a 60
thread_b 60
thread_c 61
thread_a 61
thread_b 61
thread_c 62
thread_a 62
thread_b 62
thread_c 63
thread_a 63
thread_b 63
thread_c 64
thread_a 64
thread_b 64
thread_c 65
thread_a 65
thread_b 65
thread_c 66
thread_a 66
thread_b 66
thread_c 67
thread_a 67
thread_b 67
thread_c 68
thread_a 68
thread_b 68
thread_c 69
thread_a 69
thread_b 69
thread_c 70
thread_a 70
thread_b 70
thread_c 71
thread_a 71
thread_b 71
thread_c 72
thread_a 72
thread_b 72
thread_c 73
thread_a 73
thread_b 73
thread_c 74
thread_a 74
thread_b 74
thread_c 75
thread_a 75
thread_b 75
thread_c 76
thread_a 76
thread_b 76
thread_c 77
thread_a 77
thread_b 77
thread_c 78
thread_a 78
thread_b 78
thread_c 79
thread_a 79
thread_b 79
thread_c 80
thread_a 80
thread_b 80
thread_c 81
thread_a 81
thread_b 81
thread_c 82
thread_a 82
thread_b 82
thread_c 83
thread_a 83
thread_b 83
thread_c 84
thread_a 84
thread_b 84
thread_c 85
thread_a 85
thread_b 85
thread_c 86
thread_a 86
thread_b 86
thread_c 87
thread_a 87
thread_b 87
thread_c 88
thread_a 88
thread_b 88
thread_c 89
thread_a 89
thread_b 89
thread_c 90
thread_a 90
thread_b 90
thread_c 91
thread_a 91
thread_b 91
thread_c 92
thread_a 92
thread_b 92
thread_c 93
thread_a 93
thread_b 93
thread_c 94
thread_a 94
thread_b 94
thread_c 95
thread_a 95
thread_b 95
thread_c 96
thread_a 96
thread_b 96
thread_c 97
thread_a 97
thread_b 97
thread_c 98
thread_a 98
thread_b 98
thread_c 99
thread_a 99
thread_b 99
thread_c: exit after 100
thread_a: exit after 100
thread_b: exit after 100
thread_schedule: no runnable threads
```