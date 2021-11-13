# 01-Hello OS

## Build & Run

```sh
$ make qemu
Press Ctrl-A and then X to exit QEMU
qemu-system-riscv32 -vga std -smp 4 -machine virt -bios none -kernel os.elf
VNC server running on ::1:5900
```

然後用 TightVNC 連上 127.0.0.1::5900


* http://www.ep.ph.bham.ac.uk/general/support/vncsetup.html
* [VNC遠端連線使用教學](https://kipppan.pixnet.net/blog/post/52284166-vnc%E9%81%A0%E7%AB%AF%E9%80%A3%E7%B7%9A%E4%BD%BF%E7%94%A8%E6%95%99%E5%AD%B8)
* https://ena.support.keysight.com/e5080a/manuals/webhelp/tcn/s0_start/vnc.htm
* [VNC 遠端桌面與 KVM 虛擬機](https://dywang.csie.cyut.edu.tw/dywang/download/pdf/vnc+kvm.pdf)
* [實戰經驗：QEMU啟動虛擬機只顯示VNC server running on ::1:5901](https://kknews.cc/code/kvlyvkq.html)
* [Qemu VNC中的主機鼠標指針未與訪客鼠標指針對齊](https://tw.2md-tuning-klub.com/591201-host-mouse-pointer-not-aligned-KSLIEO)

安裝 TightVNC

* https://www.tightvnc.com/download.php

* [軟體王:TightVNC]https://www.softking.com.tw/26289/)
