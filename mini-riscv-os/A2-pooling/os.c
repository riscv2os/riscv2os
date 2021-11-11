#include "lib.h"

#define interval 10000000*10

reg_t timeLast = 0;

int timeup() {
	reg_t timeNow = *(reg_t*)CLINT_MTIME;
	if (timeNow - timeLast > interval) {
		timeLast = timeNow;
		return timeNow/interval;
	}
	return 0;
}

int os_main(void)
{
	while (1) {
		if (lib_kbhit()) {
			char ch = lib_getc();
			lib_putc(ch);
			if (ch == '\n' || ch == '\r') lib_printf("\n");
		}
		int time = timeup();
		if (time) {
			lib_printf("time=%d\n", time);
		}
	}
	while (1) {} // 讓主程式卡在這裡，用無窮迴圈停止！
	return 0;
}
