#include "lib.h"
#include "trap.h"

int os_init() {
	uart_init();
	trap_init();
	plic_init();
}

int os_main(void)
{
	lib_printf("os_main() enter\n");
	lib_printf("os_init()...\n");
	os_init();
	lib_printf("user:");
	char user[100];
	lib_gets(user);
	lib_printf("user=%s\n", user);
	while (1) {} // 讓主程式卡在這裡，用無窮迴圈停止！
	lib_printf("os_main() exit\n");
	return 0;
}
