#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// RAM 記憶體
/* xv6 uses only 128MiB of memory. */
#define RAM_SIZE (1024 * 1024 * 128)

/* Same as QEMU virt machine, RAM starts at 0x80000000. */
#define RAM_BASE 0x80000000

// Core Local Interruptor (CLINT) 局部中斷 (內部)
#define CLINT_BASE 0x2000000
#define CLINT_SIZE 0x10000
#define CLINT_MTIMECMP (CLINT_BASE + 0x4000)
#define CLINT_MTIME (CLINT_BASE + 0xbff8)

// Platform-Level Interrupt Controller (PLIC) 全局中斷 (外部)
#define PLIC_BASE 0xc000000
#define PLIC_SIZE 0x4000000
#define PLIC_PENDING (PLIC_BASE + 0x1000)
#define PLIC_SENABLE (PLIC_BASE + 0x2080)
#define PLIC_SPRIORITY (PLIC_BASE + 0x201000)
#define PLIC_SCLAIM (PLIC_BASE + 0x201004)

// UART (透過 UART 傳送/接收 終端機 的訊息)
#define UART_BASE 0x10000000
#define UART_SIZE 0x100
enum { UART_RHR = UART_BASE, UART_THR = UART_BASE };
enum { UART_LCR = UART_BASE + 3, UART_LSR = UART_BASE + 5 };
enum { UART_LSR_RX = 1, UART_LSR_TX = 1 << 5 };

// VIRTIO : 磁碟控制介面。
#define VIRTIO_BASE 0x10001000
#define VIRTIO_SIZE 0x1000
#define VIRTIO_MAGIC (VIRTIO_BASE + 0x000)
#define VIRTIO_VERSION (VIRTIO_BASE + 0x004)
#define VIRTIO_DEVICE_ID (VIRTIO_BASE + 0x008)
#define VIRTIO_VENDOR_ID (VIRTIO_BASE + 0x00c)
#define VIRTIO_DEVICE_FEATURES (VIRTIO_BASE + 0x010)
#define VIRTIO_DRIVER_FEATURES (VIRTIO_BASE + 0x020)
#define VIRTIO_GUEST_PAGE_SIZE (VIRTIO_BASE + 0x028)
#define VIRTIO_QUEUE_SEL (VIRTIO_BASE + 0x030)
#define VIRTIO_QUEUE_NUM_MAX (VIRTIO_BASE + 0x034)
#define VIRTIO_QUEUE_NUM (VIRTIO_BASE + 0x038)
#define VIRTIO_QUEUE_PFN (VIRTIO_BASE + 0x040)
#define VIRTIO_QUEUE_NOTIFY (VIRTIO_BASE + 0x050)
#define VIRTIO_STATUS (VIRTIO_BASE + 0x070)

enum { VIRTIO_VRING_DESC_SIZE = 16, VIRTIO_DESC_NUM = 8 };

/* Range check
 * For any variable range checking:
 *     if (x >= minx && x <= maxx) ...
 * it is faster to use bit operation:
 *     if ((signed)((x - minx) | (maxx - x)) >= 0) ...
 */
#define RANGE_CHECK(x, minx, size) \
    ((int32_t)((x - minx) | (minx + size - 1 - x)) >= 0)

/* Machine level CSRs */
enum { MSTATUS = 0x300, MEDELEG = 0x302, MIDELEG, MIE, MTVEC };
enum { MEPC = 0x341, MCAUSE, MTVAL, MIP };

/* Supervisor level CSRs */
enum { SSTATUS = 0x100, SIE = 0x104, STVEC };
enum { SEPC = 0x141, SCAUSE, STVAL, SIP, SATP = 0x180 };

enum { MIP_SSIP = 1ULL << 1, MIP_MSIP = 1ULL << 3, MIP_STIP = 1ULL << 5 };
enum { MIP_MTIP = 1ULL << 7, MIP_SEIP = 1ULL << 9, MIP_MEIP = 1ULL << 11 };

#define PAGE_SIZE 4096 /* should be configurable */

typedef enum {
    OK = -1,
    INSTRUCTION_ADDRESS_MISALIGNED = 0,
    INSTRUCTION_ACCESS_FAULT = 1,
    ILLEGAL_INSTRUCTION = 2,
    BREAKPOINT = 3,
    LOAD_ADDRESS_MISALIGNED = 4,
    LOAD_ACCESS_FAULT = 5,
    STORE_AMO_ADDRESS_MISALIGNED = 6,
    STORE_AMO_ACCESS_FAULT = 7,
    INSTRUCTION_PAGE_FAULT = 12,
    LOAD_PAGE_FAULT = 13,
    STORE_AMO_PAGE_FAULT = 15,
} exception_t;

typedef enum {
    NONE = -1,
    SUPERVISOR_SOFTWARE_INTERRUPT = 1,
    MACHINE_SOFTWARE_INTERRUPT = 3,
    SUPERVISOR_TIMER_INTERRUPT = 5,
    MACHINE_TIMER_INTERRUPT = 7,
    SUPERVISOR_EXTERNAL_INTERRUPT = 9,
    MACHINE_EXTERNAL_INTERRUPT = 11,
} interrupt_t;

bool exception_is_fatal(const exception_t e)
{
    switch (e) {
    case INSTRUCTION_ADDRESS_MISALIGNED:
    case INSTRUCTION_ACCESS_FAULT:
    case LOAD_ACCESS_FAULT:
    case STORE_AMO_ADDRESS_MISALIGNED:
    case STORE_AMO_ACCESS_FAULT:
        return true;
    default:
        return false;
    }
}

// RAM 記憶體
struct ram {
    uint8_t *data;
};

struct ram *ram_new(const uint8_t *code, const size_t code_size)
{
    struct ram *ram = calloc(1, sizeof(struct ram));
    ram->data = calloc(RAM_SIZE, 1);
    memcpy(ram->data, code, code_size);
    return ram;
}

exception_t ram_load(const struct ram *ram,
                     const uint64_t addr,
                     const uint64_t size,
                     uint64_t *result)
{
    uint64_t index = addr - RAM_BASE, tmp = 0;
    switch (size) {
    case 64:
        tmp |= (uint64_t)(ram->data[index + 7]) << 56;
        tmp |= (uint64_t)(ram->data[index + 6]) << 48;
        tmp |= (uint64_t)(ram->data[index + 5]) << 40;
        tmp |= (uint64_t)(ram->data[index + 4]) << 32;
    case 32:
        tmp |= (uint64_t)(ram->data[index + 3]) << 24;
        tmp |= (uint64_t)(ram->data[index + 2]) << 16;
    case 16:
        tmp |= (uint64_t)(ram->data[index + 1]) << 8;
    case 8:
        tmp |= (uint64_t)(ram->data[index + 0]) << 0;
        *result = tmp;
        return OK;
    default:
        return LOAD_ACCESS_FAULT;
    }
}

exception_t ram_store(struct ram *ram,
                      const uint64_t addr,
                      const uint64_t size,
                      const uint64_t value)
{
    uint64_t index = addr - RAM_BASE;
    switch (size) {
    case 64:
        ram->data[index + 7] = (value >> 56) & 0xff;
        ram->data[index + 6] = (value >> 48) & 0xff;
        ram->data[index + 5] = (value >> 40) & 0xff;
        ram->data[index + 4] = (value >> 32) & 0xff;
    case 32:
        ram->data[index + 3] = (value >> 24) & 0xff;
        ram->data[index + 2] = (value >> 16) & 0xff;
    case 16:
        ram->data[index + 1] = (value >> 8) & 0xff;
    case 8:
        ram->data[index + 0] = (value >> 0) & 0xff;
        return OK;
    default:
        return STORE_AMO_ACCESS_FAULT;
    }
}

static void fatal(const char *msg)
{
    fprintf(stderr, "ERROR: Failed to %s.\n", msg);
    exit(1);
}

// Core Local Interruptor (CLINT) 局部中斷 (內部)
struct clint {
    uint64_t mtime, mtimecmp; // 時間控制暫存器
};

struct clint *clint_new()
{
    return calloc(1, sizeof(struct clint));
}

inline exception_t clint_load(const struct clint *clint,
                              const uint64_t addr,
                              const uint64_t size,
                              uint64_t *result)
{
    if (size != 64)
        return LOAD_ACCESS_FAULT;

    switch (addr) {
    case CLINT_MTIMECMP:
        *result = clint->mtimecmp;
        break;
    case CLINT_MTIME:
        *result = clint->mtime;
        break;
    default:
        *result = 0;
    }
    return OK;
}

inline exception_t clint_store(struct clint *clint,
                               const uint64_t addr,
                               const uint64_t size,
                               const uint64_t value)
{
    if (size != 64)
        return STORE_AMO_ACCESS_FAULT;

    switch (addr) {
    case CLINT_MTIMECMP:
        clint->mtimecmp = value;
        break;
    case CLINT_MTIME:
        clint->mtime = value;
        break;
    }
    return OK;
}

// Platform-Level Interrupt Controller (PLIC) 全局中斷 (外部)
struct plic {
    uint64_t pending;
    uint64_t senable;
    uint64_t spriority;
    uint64_t sclaim;
};

struct plic *plic_new()
{
    return calloc(1, sizeof(struct plic));
}

exception_t plic_load(const struct plic *plic,
                      const uint64_t addr,
                      const uint64_t size,
                      uint64_t *result)
{
    if (size != 32)
        return LOAD_ACCESS_FAULT;

    switch (addr) {
    case PLIC_PENDING:
        *result = plic->pending;
        break;
    case PLIC_SENABLE:
        *result = plic->senable;
        break;
    case PLIC_SPRIORITY:
        *result = plic->spriority;
        break;
    case PLIC_SCLAIM:
        *result = plic->sclaim;
        break;
    default:
        *result = 0;
    }
    return OK;
}

exception_t plic_store(struct plic *plic,
                       const uint64_t addr,
                       const uint64_t size,
                       const uint64_t value)
{
    if (size != 32)
        return STORE_AMO_ACCESS_FAULT;

    switch (addr) {
    case PLIC_PENDING:
        plic->pending = value;
        break;
    case PLIC_SENABLE:
        plic->senable = value;
        break;
    case PLIC_SPRIORITY:
        plic->spriority = value;
        break;
    case PLIC_SCLAIM:
        plic->sclaim = value;
        break;
    }
    return OK;
}

// UART (透過 UART 傳送/接收 終端機 的訊息)
struct uart {
    uint8_t data[UART_SIZE];
    bool interrupting;

    pthread_t tid;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

static void *uart_thread_func(void *priv)
{
    struct uart *uart = (struct uart *) priv;
    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) <= 0)  /* an error or EOF */
            continue;
        pthread_mutex_lock(&uart->lock);
        while ((uart->data[UART_LSR - UART_BASE] & UART_LSR_RX) == 1)
            pthread_cond_wait(&uart->cond, &uart->lock);

        uart->data[0] = c;
        uart->interrupting = true;
        uart->data[UART_LSR - UART_BASE] |= UART_LSR_RX;
        pthread_mutex_unlock(&uart->lock);
    }

    /* should not reach here */
    return NULL;
}

struct uart *uart_new()
{
    struct uart *uart = calloc(1, sizeof(struct uart));
    uart->data[UART_LSR - UART_BASE] |= UART_LSR_TX;
    pthread_mutex_init(&uart->lock, NULL);
    pthread_cond_init(&uart->cond, NULL);

    pthread_create(&uart->tid, NULL, uart_thread_func, (void *) uart);
    return uart;
}

exception_t uart_load(struct uart *uart,
                      const uint64_t addr,
                      const uint64_t size,
                      uint64_t *result)
{
    if (size != 8)
        return LOAD_ACCESS_FAULT;

    pthread_mutex_lock(&uart->lock);
    switch (addr) {
    case UART_RHR:
        pthread_cond_broadcast(&uart->cond);
        uart->data[UART_LSR - UART_BASE] &= ~UART_LSR_RX;
    default:
        *result = uart->data[addr - UART_BASE];
    }
    pthread_mutex_unlock(&uart->lock);
    return OK;
}

exception_t uart_store(struct uart *uart,
                       const uint64_t addr,
                       const uint64_t size,
                       const uint64_t value)
{
    if (size != 8)
        return STORE_AMO_ACCESS_FAULT;

    pthread_mutex_lock(&uart->lock);
    switch (addr) {
    case UART_THR:
        putchar(value & 0xff);
        fflush(stdout);
        break;
    default:
        uart->data[addr - UART_BASE] = value & 0xff;
    }
    pthread_mutex_unlock(&uart->lock);
    return OK;
}

bool uart_is_interrupting(struct uart *uart)
{
    pthread_mutex_lock(&uart->lock);
    bool interrupting = uart->interrupting;
    uart->interrupting = false;
    pthread_mutex_unlock(&uart->lock);
    return interrupting;
}

// VIRTIO : 磁碟控制介面的物件
struct virtio {
    uint64_t id;
    uint32_t driver_features;
    uint32_t page_size;
    uint32_t queue_sel;
    uint32_t queue_num;
    uint32_t queue_pfn;
    uint32_t queue_notify;
    uint32_t status;
    uint8_t *disk;
};

struct virtio *virtio_new(uint8_t *disk)
{
    struct virtio *vio = calloc(1, sizeof(struct virtio));
    vio->disk = disk;
    vio->queue_notify = -1;
    return vio;
}

exception_t virtio_load(const struct virtio *vio,
                        const uint64_t addr,
                        const uint64_t size,
                        uint64_t *result)
{
    if (size != 32)
        return LOAD_ACCESS_FAULT;

    switch (addr) {
    case VIRTIO_MAGIC:
        *result = 0x74726976;
        break;
    case VIRTIO_VERSION:
        *result = 0x1;
        break;
    case VIRTIO_DEVICE_ID:
        *result = 0x2;
        break;
    case VIRTIO_VENDOR_ID:
        *result = 0x554d4551;
        break;
    case VIRTIO_DEVICE_FEATURES:
        *result = 0;
        break;
    case VIRTIO_DRIVER_FEATURES:
        *result = vio->driver_features;
        break;
    case VIRTIO_QUEUE_NUM_MAX:
        *result = 8;
        break;
    case VIRTIO_QUEUE_PFN:
        *result = vio->queue_pfn;
        break;
    case VIRTIO_STATUS:
        *result = vio->status;
        break;
    default:
        *result = 0;
    }
    return OK;
}

exception_t virtio_store(struct virtio *vio,
                         const uint64_t addr,
                         const uint64_t size,
                         const uint64_t value)
{
    if (size != 32)
        return STORE_AMO_ACCESS_FAULT;

    switch (addr) {
    case VIRTIO_DEVICE_FEATURES:
        vio->driver_features = value;
        break;
    case VIRTIO_GUEST_PAGE_SIZE:
        vio->page_size = value;
        break;
    case VIRTIO_QUEUE_SEL:
        vio->queue_sel = value;
        break;
    case VIRTIO_QUEUE_NUM:
        vio->queue_num = value;
        break;
    case VIRTIO_QUEUE_PFN:
        vio->queue_pfn = value;
        break;
    case VIRTIO_QUEUE_NOTIFY:
        vio->queue_notify = value;
        break;
    case VIRTIO_STATUS:
        vio->status = value;
        break;
    }
    return OK;
}

inline bool virtio_is_interrupting(struct virtio *vio)
{
    if (vio->queue_notify != -1) {
        vio->queue_notify = -1;
        return true;
    }
    return false;
}

inline uint64_t virtio_desc_addr(const struct virtio *vio)
{
    return (uint64_t) vio->queue_pfn * (uint64_t) vio->page_size;
}

inline uint64_t virtio_disk_read(const struct virtio *vio, uint64_t addr)
{
    return vio->disk[addr];
}

inline void virtio_disk_write(struct virtio *vio, uint64_t addr, uint64_t value)
{
    vio->disk[addr] = (uint8_t) value;
}

inline uint64_t virtio_new_id(struct virtio *vio)
{
    return ++(vio->id);
}

// BUS 匯流排物件的資料結構，會轉給 RAM/CLINT/PLIC/UART/VIRTIO 處理。
struct bus {
    struct ram *ram;       // RAM 記憶體
    struct clint *clint;   // Core Local Interruptor (CLINT) 局部中斷 (內部)
    struct plic *plic;     // Platform-Level Interrupt Controller (PLIC) 全局中斷 (外部)
    struct uart *uart;     // UART (透過 UART 傳送/接收 終端機 的訊息)
    struct virtio *virtio; // VIRTIO : 磁碟控制介面。
};

// BUS 匯流排物件
struct bus *bus_new(struct ram *ram, struct virtio *vio)
{
    struct bus *bus = calloc(1, sizeof(struct bus));
    bus->ram = ram, bus->virtio = vio;
    bus->clint = clint_new(), bus->plic = plic_new(), bus->uart = uart_new();
    return bus;
}

// 模擬 BUS 從 addr 載入 size bits 的資料到 *result 中。
exception_t bus_load(const struct bus *bus,
                     const uint64_t addr,
                     const uint64_t size,
                     uint64_t *result)
{
    if (RANGE_CHECK(addr, CLINT_BASE, CLINT_SIZE))
        return clint_load(bus->clint, addr, size, result);
    if (RANGE_CHECK(addr, PLIC_BASE, PLIC_SIZE))
        return plic_load(bus->plic, addr, size, result);
    if (RANGE_CHECK(addr, UART_BASE, UART_SIZE))
        return uart_load(bus->uart, addr, size, result);
    if (RANGE_CHECK(addr, VIRTIO_BASE, VIRTIO_SIZE))
        return virtio_load(bus->virtio, addr, size, result);
    if (RAM_BASE <= addr)
        return ram_load(bus->ram, addr, size, result);

    return LOAD_ACCESS_FAULT;
}

// 模擬 BUS 儲存 size bits 的 value 到 addr 中。
exception_t bus_store(struct bus *bus,
                      const uint64_t addr,
                      const uint64_t size,
                      const uint64_t value)
{
    if (RANGE_CHECK(addr, CLINT_BASE, CLINT_SIZE))
        return clint_store(bus->clint, addr, size, value);
    if (RANGE_CHECK(addr, PLIC_BASE, PLIC_SIZE))
        return plic_store(bus->plic, addr, size, value);
    if (RANGE_CHECK(addr, UART_BASE, UART_SIZE))
        return uart_store(bus->uart, addr, size, value);
    if (RANGE_CHECK(addr, VIRTIO_BASE, VIRTIO_SIZE))
        return virtio_store(bus->virtio, addr, size, value);
    if (RAM_BASE <= addr)
        return ram_store(bus->ram, addr, size, value);

    return STORE_AMO_ACCESS_FAULT;
}

// 當磁碟中斷發生後，模擬完成磁碟讀寫動作
void bus_disk_access(struct bus *bus)
{
    uint64_t desc_addr = virtio_desc_addr(bus->virtio);
    uint64_t avail_addr = desc_addr + 0x40, used_addr = desc_addr + 4096;

    uint64_t offset;
    if (bus_load(bus, avail_addr + 1, 16, &offset) != OK)
        fatal("read offset");

    uint64_t index;
    if (bus_load(bus, avail_addr + (offset % VIRTIO_DESC_NUM) + 2, 16,
                 &index) != OK)
        fatal("read index");

    uint64_t desc_addr0 = desc_addr + VIRTIO_VRING_DESC_SIZE * index;
    uint64_t addr0;
    if (bus_load(bus, desc_addr0, 64, &addr0) != OK)
        fatal("read address field in descriptor");
    uint64_t next0;
    if (bus_load(bus, desc_addr0 + 14, 16, &next0) != OK)
        fatal("read next field in descriptor");
    uint64_t desc_addr1 = desc_addr + VIRTIO_VRING_DESC_SIZE * next0;
    uint64_t addr1;
    if (bus_load(bus, desc_addr1, 64, &addr1) != OK)
        fatal("read address field in descriptor");
    uint64_t len1;
    if (bus_load(bus, desc_addr1 + 8, 32, &len1) != OK)
        fatal("read length field in descriptor");
    uint64_t flags1;
    if (bus_load(bus, desc_addr1 + 12, 16, &flags1) != OK)
        fatal("read flags field in descriptor");

    uint64_t blk_sector;
    if (bus_load(bus, addr0 + 8, 64, &blk_sector) != OK)
        fatal("read sector field in virtio_blk_outhdr");

    if ((flags1 & 2) == 0) {
        /* Read RAM data and write it to a disk directly (DMA). */
        for (uint64_t i = 0; i < len1; i++) { // 寫入 len1 bytes 到磁碟中
            uint64_t data;
            if (bus_load(bus, addr1 + i, 8, &data) != OK)
                fatal("read from RAM");
            virtio_disk_write(bus->virtio, blk_sector * 512 + i, data);
        }
    } else {
        /* Read disk data and write it to RAM directly (DMA). */
        for (uint64_t i = 0; i < len1; i++) { // 從磁碟中讀取 len1 bytes
            uint64_t data = virtio_disk_read(bus->virtio, blk_sector * 512 + i);
            if (bus_store(bus, addr1 + i, 8, data) != OK)
                fatal("write to RAM");
        }
    }

    uint64_t new_id = virtio_new_id(bus->virtio);
    if (bus_store(bus, used_addr + 2, 16, new_id % 8) != OK)
        fatal("write to RAM");
}

typedef enum { USER = 0x0, SUPERVISOR = 0x1, MACHINE = 0x3 } cpu_mode_t;

// CPU 物件的結構
struct cpu {
    uint64_t regs[32], pc; // 暫存器 regs (x0..x31), 程式計數器 PC (Proram Counter)
    uint64_t csrs[4096];   // 控制暫存器
    cpu_mode_t mode;       // CPU 模式 (S:Supervisor/M:Machine/U:User Mode)
    struct bus *bus;       // BUS 匯流排
    bool enable_paging;    // 啟動分頁
    uint64_t pagetable;    // 分頁表 (SATP 暫存器所指向的位置)
};

// 創建 CPU 物件
struct cpu *cpu_new(uint8_t *code, const size_t code_size, uint8_t *disk)
{
    struct cpu *cpu = calloc(1, sizeof(struct cpu));

    /* Initialize the sp(x2) register. */
    cpu->regs[2] = RAM_BASE + RAM_SIZE;

    cpu->bus = bus_new(ram_new(code, code_size), virtio_new(disk));
    cpu->pc = RAM_BASE, cpu->mode = MACHINE;

    return cpu;
}

uint64_t cpu_load_csr(const struct cpu *cpu, const uint16_t addr);

// 若修改 SATP ，則更新分頁表
inline void cpu_update_paging(struct cpu *cpu, const uint16_t csr_addr)
{
    if (csr_addr != SATP)
        return;

    cpu->pagetable =
        (cpu_load_csr(cpu, SATP) & (((uint64_t) 1 << 44) - 1)) * PAGE_SIZE;
    cpu->enable_paging = (8 == (cpu_load_csr(cpu, SATP) >> 60));
}

// 模擬 MMU 分頁表的位址轉換過程。
exception_t cpu_translate(const struct cpu *cpu,
                          const uint64_t addr,
                          const exception_t e,
                          uint64_t *result)
{
    if (!cpu->enable_paging) {
        *result = addr;
        return OK;
    }

    const uint64_t vpn[] = { // 虛擬頁號 virtual page number (VPN)
        (addr >> 12) & 0x1ff,
        (addr >> 21) & 0x1ff,
        (addr >> 30) & 0x1ff,
    };
    int level = sizeof(vpn) / sizeof(vpn[0]) - 1;

    uint64_t a = cpu->pagetable; // 取得分頁表位址
    uint64_t pte;
    while (1) { // 一層一層載入 PTE (Page Table Entry)
        exception_t exc = bus_load(cpu->bus, a + vpn[level] * 8, 64, &pte);
        if (exc != OK)
            return exc;
        bool v = pte & 1;
        bool r = (pte >> 1) & 1, w = (pte >> 2) & 1, x = (pte >> 3) & 1;
        if (!v || (!r && w))
            return e;

        if (r || x) // 可讀 或 可執行，就開始進下一步取得實體位址??
            break;

        a = ((pte >> 10) & 0x0fffffffffff) * PAGE_SIZE;
        if (--level < 0)
            return e;
    }

    const uint64_t ppn[] = { // 實體頁號 physical page number (PPN)
        (pte >> 10) & 0x1ff,
        (pte >> 19) & 0x1ff,
        (pte >> 28) & 0x03ffffff,
    };

    uint64_t offset = addr & 0xfff;
    switch (level) {
    case 0:
        *result = (((pte >> 10) & 0x0fffffffffff) << 12) | offset;
        return OK;
    case 1:
        *result = (ppn[2] << 30) | (ppn[1] << 21) | (vpn[0] << 12) | offset;
        return OK;
    case 2:
        *result = (ppn[2] << 30) | (vpn[1] << 21) | (vpn[0] << 12) | offset;
        return OK;
    default:
        return e;
    }
}

// 擷取指令
/* Fetch an instruction from current PC from RAM. */
inline exception_t cpu_fetch(struct cpu *cpu, uint64_t *result)
{
    uint64_t ppc;
    exception_t e = cpu_translate(cpu, cpu->pc, INSTRUCTION_PAGE_FAULT, &ppc); // 經過 MMU 轉換位址後取得指令。
    if (e != OK)
        return e;

    if (bus_load(cpu->bus, ppc, 32, result) != OK)
        return INSTRUCTION_ACCESS_FAULT;
    return OK;
}

// 載入控制暫存器 Control-Status Register (CSR)
inline uint64_t cpu_load_csr(const struct cpu *cpu, const uint16_t addr)
{
    if (addr == SIE)
        return cpu->csrs[MIE] & cpu->csrs[MIDELEG];
    return cpu->csrs[addr];
}

// 儲存控制暫存器 Control-Status Register (CSR)
inline void cpu_store_csr(struct cpu *cpu,
                          const uint16_t addr,
                          const uint64_t value)
{
    if (addr == SIE) {
        cpu->csrs[MIE] = (cpu->csrs[MIE] & ~cpu->csrs[MIDELEG]) |
                         (value & cpu->csrs[MIDELEG]);
        return;
    }
    cpu->csrs[addr] = value;
}

// 從 addr 載入 size bits 的內容到 *result
inline exception_t cpu_load(struct cpu *cpu,
                            const uint64_t addr,
                            const uint64_t size,
                            uint64_t *result)
{
    uint64_t pa;
    exception_t e = cpu_translate(cpu, addr, LOAD_PAGE_FAULT, &pa);
    if (e != OK)
        return e;
    return bus_load(cpu->bus, pa, size, result);
}

// 儲存 size bits 的 value 到 addr
inline exception_t cpu_store(struct cpu *cpu,
                             const uint64_t addr,
                             const uint64_t size,
                             const uint64_t value)
{
    uint64_t pa;
    exception_t e = cpu_translate(cpu, addr, STORE_AMO_PAGE_FAULT, &pa);
    if (e != OK)
        return e;

    return bus_store(cpu->bus, pa, size, value);
}

// 執行指令
exception_t cpu_execute(struct cpu *cpu, const uint64_t inst)
{
    uint64_t opcode = inst & 0x7f;
    uint64_t rd = (inst >> 7) & 0x1f;
    uint64_t rs1 = (inst >> 15) & 0x1f, rs2 = (inst >> 20) & 0x1f;
    uint64_t funct3 = (inst >> 12) & 0x7, funct7 = (inst >> 25) & 0x7f;

    cpu->regs[0] = 0; /* x0 register is always zero */

    exception_t e;
    switch (opcode) {
    case 0x03: {
        uint64_t imm = (int32_t) inst >> 20;
        uint64_t addr = cpu->regs[rs1] + imm;
        switch (funct3) {
        case 0x0: /* lb */ {
            uint64_t result;
            if ((e = cpu_load(cpu, addr, 8, &result)) != OK)
                return e;
            cpu->regs[rd] = (int8_t) result;
            break;
        }
        case 0x1: /* lh */ {
            uint64_t result;
            if ((e = cpu_load(cpu, addr, 16, &result)) != OK)
                return e;
            cpu->regs[rd] = (int16_t) result;
            break;
        }
        case 0x2: /* lw */ {
            uint64_t result;
            if ((e = cpu_load(cpu, addr, 32, &result)) != OK)
                return e;
            cpu->regs[rd] = (int32_t) result;
            break;
        }
        case 0x3: /* ld */ {
            uint64_t result;
            if ((e = cpu_load(cpu, addr, 64, &result)) != OK)
                return e;
            cpu->regs[rd] = result;
            break;
        }
        case 0x4: /* lbu */ {
            uint64_t result;
            if ((e = cpu_load(cpu, addr, 8, &result)) != OK)
                return e;
            cpu->regs[rd] = result;
            break;
        }
        case 0x5: /* lhu */ {
            uint64_t result;
            if ((e = cpu_load(cpu, addr, 16, &result)) != OK)
                return e;
            cpu->regs[rd] = result;
            break;
        }
        case 0x6: /* lwu */ {
            uint64_t result;
            if ((e = cpu_load(cpu, addr, 32, &result)) != OK)
                return e;
            cpu->regs[rd] = result;
            break;
        }
        default:
            return ILLEGAL_INSTRUCTION;
        }
        break;
    }
    case 0x0f:
        switch (funct3) {
        case 0x0: /* fence */
            /* TODO: implemented */
            break;
        default:
            return ILLEGAL_INSTRUCTION;
        }
        break;
    case 0x13: {
        uint64_t imm = (int32_t)(inst & 0xfff00000) >> 20;
        uint32_t shamt = imm & 0x3f;

        switch (funct3) {
        case 0x0: /* addi */
            cpu->regs[rd] = cpu->regs[rs1] + imm;
            break;
        case 0x1: /* slli */
            cpu->regs[rd] = cpu->regs[rs1] << shamt;
            break;
        case 0x2: /* slti */
            cpu->regs[rd] = !!((int64_t) cpu->regs[rs1] < (int64_t) imm);
            break;
        case 0x3: /* sltiu */
            cpu->regs[rd] = !!(cpu->regs[rs1] < imm);
            break;
        case 0x4: /* xori */
            cpu->regs[rd] = cpu->regs[rs1] ^ imm;
            break;
        case 0x5:
            switch (funct7 >> 1) {
            case 0x00: /* srli */
                cpu->regs[rd] = cpu->regs[rs1] >> shamt;
                break;
            case 0x10: /* srai */
                cpu->regs[rd] = (int64_t)(cpu->regs[rs1]) >> shamt;
                break;
            default:
                return ILLEGAL_INSTRUCTION;
            }
            break;
        case 0x6: /* ori */
            cpu->regs[rd] = cpu->regs[rs1] | imm;
            break;
        case 0x7: /* andi */
            cpu->regs[rd] = cpu->regs[rs1] & imm;
            break;
        default:
            return ILLEGAL_INSTRUCTION;
        }
        break;
    }
    case 0x17: /* auipc */ {
        uint64_t imm = (int32_t)(inst & 0xfffff000);
        cpu->regs[rd] = cpu->pc + imm - 4;
        break;
    }
    case 0x1b: {
        uint64_t imm = (int32_t) inst >> 20;
        uint32_t shamt = imm & 0x1f;
        switch (funct3) {
        case 0x0: /* addiw */
            cpu->regs[rd] = (int32_t)(cpu->regs[rs1] + imm);
            break;
        case 0x1: /* slliw */
            cpu->regs[rd] = (int32_t)(cpu->regs[rs1] << shamt);
            break;
        case 0x5: {
            switch (funct7) {
            case 0x00: /* srliw */
                cpu->regs[rd] = (int32_t)((uint32_t) cpu->regs[rs1] >> shamt);
                break;
            case 0x20: /* sraiw */
                cpu->regs[rd] = (int32_t)(cpu->regs[rs1]) >> shamt;
                break;
            default:
                return ILLEGAL_INSTRUCTION;
            }
            break;
        }
        default:
            return ILLEGAL_INSTRUCTION;
        }
        break;
    }
    case 0x23: {
        uint64_t imm = (uint64_t)((int32_t)(inst & 0xfe000000) >> 20) |
                       ((inst >> 7) & 0x1f);
        uint64_t addr = cpu->regs[rs1] + imm;
        switch (funct3) {
        case 0x0: /* sb */
            if ((e = cpu_store(cpu, addr, 8, cpu->regs[rs2])) != OK)
                return e;
            break;
        case 0x1: /* sh */
            if ((e = cpu_store(cpu, addr, 16, cpu->regs[rs2])) != OK)
                return e;
            break;
        case 0x2: /* sw */
            if ((e = cpu_store(cpu, addr, 32, cpu->regs[rs2])) != OK)
                return e;
            break;
        case 0x3: /* sd */
            if ((e = cpu_store(cpu, addr, 64, cpu->regs[rs2])) != OK)
                return e;
            break;
        default:
            return ILLEGAL_INSTRUCTION;
        }
        break;
    }
    case 0x2f: {
        uint64_t funct5 = (funct7 & 0x7c) >> 2;
        if (funct3 == 0x2 && funct5 == 0x00) { /* amoadd.w */
            uint64_t t;
            if ((e = cpu_load(cpu, cpu->regs[rs1], 32, &t)) != OK)
                return e;
            if ((e = cpu_store(cpu, cpu->regs[rs1], 32, t + cpu->regs[rs2])) !=
                OK)
                return e;
            cpu->regs[rd] = t;
        } else if (funct3 == 0x3 && funct5 == 0x00) { /* amoadd.d */
            uint64_t t;
            if ((e = cpu_load(cpu, cpu->regs[rs1], 64, &t)) != OK)
                return e;
            if ((e = cpu_store(cpu, cpu->regs[rs1], 64, t + cpu->regs[rs2])) !=
                OK)
                return e;
            cpu->regs[rd] = t;
        } else if (funct3 == 0x2 && funct5 == 0x01) { /* amoswap.w */
            uint64_t t;
            if ((e = cpu_load(cpu, cpu->regs[rs1], 32, &t)) != OK)
                return e;
            if ((e = cpu_store(cpu, cpu->regs[rs1], 32, cpu->regs[rs2])) != OK)
                return e;
            cpu->regs[rd] = t;
        } else if (funct3 == 0x3 && funct5 == 0x01) { /* amoswap.d */
            uint64_t t;
            if ((e = cpu_load(cpu, cpu->regs[rs1], 64, &t)) != OK)
                return e;
            if ((e = cpu_store(cpu, cpu->regs[rs1], 64, cpu->regs[rs2])) != OK)
                return e;
            cpu->regs[rd] = t;
        } else {
            return ILLEGAL_INSTRUCTION;
        }
        break;
    }
    case 0x33: {
        uint32_t shamt = cpu->regs[rs2] & 0x3f;
        if (funct3 == 0x0 && funct7 == 0x00) { /* add */
            cpu->regs[rd] = cpu->regs[rs1] + cpu->regs[rs2];
        } else if (funct3 == 0x0 && funct7 == 0x01) { /* mul */
            cpu->regs[rd] = cpu->regs[rs1] * cpu->regs[rs2];
        } else if (funct3 == 0x0 && funct7 == 0x20) { /* sub */
            cpu->regs[rd] = cpu->regs[rs1] - cpu->regs[rs2];
        } else if (funct3 == 0x1 && funct7 == 0x00) { /* sll */
            cpu->regs[rd] = cpu->regs[rs1] << shamt;
        } else if (funct3 == 0x2 && funct7 == 0x00) { /* slt */
            cpu->regs[rd] =
                !!((int64_t) cpu->regs[rs1] < (int64_t) cpu->regs[rs2]);
        } else if (funct3 == 0x3 && funct7 == 0x00) { /* sltu */
            cpu->regs[rd] = !!(cpu->regs[rs1] < cpu->regs[rs2]);
        } else if (funct3 == 0x4 && funct7 == 0x00) { /* xor */
            cpu->regs[rd] = cpu->regs[rs1] ^ cpu->regs[rs2];
        } else if (funct3 == 0x5 && funct7 == 0x00) { /* srl */
            cpu->regs[rd] = cpu->regs[rs1] >> shamt;
        } else if (funct3 == 0x5 && funct7 == 0x20) { /* sra */
            cpu->regs[rd] = (int64_t) cpu->regs[rs1] >> shamt;
        } else if (funct3 == 0x6 && funct7 == 0x00) { /* or */
            cpu->regs[rd] = cpu->regs[rs1] | cpu->regs[rs2];
        } else if (funct3 == 0x7 && funct7 == 0x00) { /* and */
            cpu->regs[rd] = cpu->regs[rs1] & cpu->regs[rs2];
        } else {
            return ILLEGAL_INSTRUCTION;
        }
        break;
    }
    case 0x37: /* lui */
        cpu->regs[rd] = (int32_t)(inst & 0xfffff000);
        break;
    case 0x3b: {
        uint32_t shamt = cpu->regs[rs2] & 0x1f;
        if (funct3 == 0x0 && funct7 == 0x00) { /* addw */
            cpu->regs[rd] = (int32_t)(cpu->regs[rs1] + cpu->regs[rs2]);
        } else if (funct3 == 0x0 && funct7 == 0x20) { /* subw */
            cpu->regs[rd] = (int32_t)(cpu->regs[rs1] - cpu->regs[rs2]);
        } else if (funct3 == 0x1 && funct7 == 0x00) { /* sllw */
            cpu->regs[rd] = (int32_t)((uint32_t) cpu->regs[rs1] << shamt);
        } else if (funct3 == 0x5 && funct7 == 0x00) { /* srlw */
            cpu->regs[rd] = (int32_t)((uint32_t) cpu->regs[rs1] >> shamt);
        } else if (funct3 == 0x5 && funct7 == 0x01) { /* divu */
            cpu->regs[rd] =
                cpu->regs[rs2] == 0 ? -1 : cpu->regs[rs1] / cpu->regs[rs2];
        } else if (funct3 == 0x5 && funct7 == 0x20) { /* sraw */
            cpu->regs[rd] = (int32_t) cpu->regs[rs1] >> (int32_t) shamt;
        } else if (funct3 == 0x7 && funct7 == 0x01) { /* remuw */
            cpu->regs[rd] = cpu->regs[rs2] == 0
                                ? cpu->regs[rs1]
                                : (int32_t)((uint32_t) cpu->regs[rs1] %
                                            (uint32_t) cpu->regs[rs2]);
        } else {
            return ILLEGAL_INSTRUCTION;
        }
        break;
    }
    case 0x63: {
        uint64_t imm = (uint64_t)((int32_t)(inst & 0x80000000) >> 19) |
                       ((inst & 0x80) << 4) | ((inst >> 20) & 0x7e0) |
                       ((inst >> 7) & 0x1e);

        switch (funct3) {
        case 0x0: /* beq */
            if (cpu->regs[rs1] == cpu->regs[rs2])
                cpu->pc += imm - 4;
            break;
        case 0x1: /* bne */
            if (cpu->regs[rs1] != cpu->regs[rs2])
                cpu->pc += imm - 4;
            break;
        case 0x4: /* blt */
            if ((int64_t) cpu->regs[rs1] < (int64_t) cpu->regs[rs2])
                cpu->pc += imm - 4;
            break;
        case 0x5: /* bge */
            if ((int64_t) cpu->regs[rs1] >= (int64_t) cpu->regs[rs2])
                cpu->pc += imm - 4;
            break;
        case 0x6: /* bltu */
            if (cpu->regs[rs1] < cpu->regs[rs2])
                cpu->pc += imm - 4;
            break;
        case 0x7: /* bgeu */
            if (cpu->regs[rs1] >= cpu->regs[rs2])
                cpu->pc += imm - 4;
            break;
        default:
            return ILLEGAL_INSTRUCTION;
        }
        break;
    }
    case 0x67: { /* jalr */
        uint64_t t = cpu->pc;
        uint64_t imm = (int32_t)(inst & 0xfff00000) >> 20;
        cpu->pc = (cpu->regs[rs1] + imm) & ~1;

        cpu->regs[rd] = t;
        break;
    }
    case 0x6f: { /* jal */
        cpu->regs[rd] = cpu->pc;

        uint64_t imm = (uint64_t)((int32_t)(inst & 0x80000000) >> 11) |
                       (inst & 0xff000) | ((inst >> 9) & 0x800) |
                       ((inst >> 20) & 0x7fe);

        cpu->pc += imm - 4;
        break;
    }
    case 0x73: {
        uint16_t addr = (inst & 0xfff00000) >> 20;
        switch (funct3) {
        case 0x0:
            if (rs2 == 0x0 && funct7 == 0x0) { /* ecall */
                switch (cpu->mode) {
                case USER:
                case SUPERVISOR:
                case MACHINE:
                    return 8 + cpu->mode; /* ECALL_FROM_{U,S,M}MODE */
                }
            } else if (rs2 == 0x1 && funct7 == 0x0) { /* ebreak */
                return BREAKPOINT;
            } else if (rs2 == 0x2 && funct7 == 0x8) { /* sret */
                cpu->pc = cpu_load_csr(cpu, SEPC);
                cpu->mode =
                    ((cpu_load_csr(cpu, SSTATUS) >> 8) & 1) ? SUPERVISOR : USER;
                cpu_store_csr(cpu, SSTATUS,
                              ((cpu_load_csr(cpu, SSTATUS) >> 5) & 1)
                                  ? cpu_load_csr(cpu, SSTATUS) | (1 << 1)
                                  : cpu_load_csr(cpu, SSTATUS) & ~(1 << 1));
                cpu_store_csr(cpu, SSTATUS,
                              cpu_load_csr(cpu, SSTATUS) | (1 << 5));
                cpu_store_csr(cpu, SSTATUS,
                              cpu_load_csr(cpu, SSTATUS) & ~(1 << 8));
            } else if (rs2 == 0x2 && funct7 == 0x18) { /* mret */
                cpu->pc = cpu_load_csr(cpu, MEPC);
                uint64_t mpp = (cpu_load_csr(cpu, MSTATUS) >> 11) & 3;
                cpu->mode = mpp == 2 ? MACHINE : (mpp == 1 ? SUPERVISOR : USER);
                cpu_store_csr(cpu, MSTATUS,
                              ((cpu_load_csr(cpu, MSTATUS) >> 7) & 1)
                                  ? cpu_load_csr(cpu, MSTATUS) | (1 << 3)
                                  : cpu_load_csr(cpu, MSTATUS) & ~(1 << 3));
                cpu_store_csr(cpu, MSTATUS,
                              cpu_load_csr(cpu, MSTATUS) | (1 << 7));
                cpu_store_csr(cpu, MSTATUS,
                              cpu_load_csr(cpu, MSTATUS) & ~(3 << 11));
            } else if (funct7 == 0x9) { /* sfence.vma */
                /* TODO: implemented */
            } else {
                return ILLEGAL_INSTRUCTION;
            }
            break;
        case 0x1: { /* csrrw */
            uint64_t t = cpu_load_csr(cpu, addr);
            cpu_store_csr(cpu, addr, cpu->regs[rs1]);
            cpu->regs[rd] = t;
            cpu_update_paging(cpu, addr);
            break;
        }
        case 0x2: { /* csrrs */
            uint64_t t = cpu_load_csr(cpu, addr);
            cpu_store_csr(cpu, addr, t | cpu->regs[rs1]);
            cpu->regs[rd] = t;
            cpu_update_paging(cpu, addr);
            break;
        }
        case 0x3: { /* csrrc */
            uint64_t t = cpu_load_csr(cpu, addr);
            cpu_store_csr(cpu, addr, t & ~cpu->regs[rs1]);
            cpu->regs[rd] = t;
            cpu_update_paging(cpu, addr);
            break;
        }
        case 0x5: /* csrrwi */
            cpu->regs[rd] = cpu_load_csr(cpu, addr);
            cpu_store_csr(cpu, addr, rs1);
            cpu_update_paging(cpu, addr);
            break;
        case 0x6: { /* csrrsi */
            uint64_t t = cpu_load_csr(cpu, addr);
            cpu_store_csr(cpu, addr, t | rs1);
            cpu->regs[rd] = t;
            cpu_update_paging(cpu, addr);
            break;
        }
        case 0x7: { /* csrrci */
            uint64_t t = cpu_load_csr(cpu, addr);
            cpu_store_csr(cpu, addr, t & ~rs1);
            cpu->regs[rd] = t;
            cpu_update_paging(cpu, addr);
            break;
        }
        default:
            return ILLEGAL_INSTRUCTION;
        }
        break;
    }
    default:
        return ILLEGAL_INSTRUCTION;
    }

    return OK;
}

// 處理中斷
void cpu_take_trap(struct cpu *cpu, const exception_t e, const interrupt_t intr)
{
    uint64_t exception_pc = cpu->pc - 4;
    cpu_mode_t prev_mode = cpu->mode;

    bool is_interrupt = (intr != NONE);
    uint64_t cause = e;
    if (is_interrupt) // 設定中斷原因 (類型)
        cause = ((uint64_t) 1 << 63) | (uint64_t) intr;

    if (prev_mode <= SUPERVISOR && // 在 U-mode/S-mode 被中斷，此時應進入 S-mode
        (((cpu_load_csr(cpu, MEDELEG) >> (uint32_t) cause) & 1) != 0)) {
        cpu->mode = SUPERVISOR; // 進入 S mode
        if (is_interrupt) { // 中斷重入？
            uint64_t vec = (cpu_load_csr(cpu, STVEC) & 1) ? (4 * cause) : 0; // 載入中斷向量
            cpu->pc = (cpu_load_csr(cpu, STVEC) & ~1) + vec; // 跳到該中斷點
        } else {
            cpu->pc = cpu_load_csr(cpu, STVEC) & ~1;
        }
        // 儲存 S mode PC, CAUSE, TVAL, STATUS
        cpu_store_csr(cpu, SEPC, exception_pc & ~1); 
        cpu_store_csr(cpu, SCAUSE, cause);
        cpu_store_csr(cpu, STVAL, 0);
        cpu_store_csr(cpu, SSTATUS,
                      ((cpu_load_csr(cpu, SSTATUS) >> 1) & 1)
                          ? cpu_load_csr(cpu, SSTATUS) | (1 << 5)
                          : cpu_load_csr(cpu, SSTATUS) & ~(1 << 5));
        cpu_store_csr(cpu, SSTATUS, cpu_load_csr(cpu, SSTATUS) & ~(1 << 1));
        if (prev_mode == USER) {
            cpu_store_csr(cpu, SSTATUS, cpu_load_csr(cpu, SSTATUS) & ~(1 << 8));
        } else {
            cpu_store_csr(cpu, SSTATUS, cpu_load_csr(cpu, SSTATUS) | (1 << 8));
        }
    } else { // 在 M-mode 或特定的 S-mode 狀態被中斷，此時應進入 M-mode
        cpu->mode = MACHINE;

        if (is_interrupt) {
            uint64_t vec = (cpu_load_csr(cpu, MTVEC) & 1) ? 4 * cause : 0;
            cpu->pc = (cpu_load_csr(cpu, MTVEC) & ~1) + vec;
        } else {
            cpu->pc = cpu_load_csr(cpu, MTVEC) & ~1;
        }
        cpu_store_csr(cpu, MEPC, exception_pc & ~1);
        cpu_store_csr(cpu, MCAUSE, cause);
        cpu_store_csr(cpu, MTVAL, 0);
        cpu_store_csr(cpu, MSTATUS,
                      ((cpu_load_csr(cpu, MSTATUS) >> 3) & 1)
                          ? cpu_load_csr(cpu, MSTATUS) | (1 << 7)
                          : cpu_load_csr(cpu, MSTATUS) & ~(1 << 7));
        cpu_store_csr(cpu, MSTATUS, cpu_load_csr(cpu, MSTATUS) & ~(1 << 3));
        cpu_store_csr(cpu, MSTATUS, cpu_load_csr(cpu, MSTATUS) & ~(3 << 11));
    }
}

enum { VIRTIO_IRQ = 1, UART_IRQ = 10 };

// 檢查是否有等待的中斷，傳回中斷類型。
interrupt_t cpu_check_pending_interrupt(struct cpu *cpu)
{
    if (cpu->mode == MACHINE && ((cpu_load_csr(cpu, MSTATUS) >> 3) & 1) == 0)
        return NONE;
    if (cpu->mode == SUPERVISOR && ((cpu_load_csr(cpu, SSTATUS) >> 1) & 1) == 0)
        return NONE;

    do {
        uint64_t irq;
        if (uart_is_interrupting(cpu->bus->uart)) {
            irq = UART_IRQ;
        } else if (virtio_is_interrupting(cpu->bus->virtio)) { // 磁碟中斷
            bus_disk_access(cpu->bus); // 呼叫 bus_disk_access() 完成磁碟讀寫
            irq = VIRTIO_IRQ;
        } else
            break;

        bus_store(cpu->bus, PLIC_SCLAIM, 32, irq);
        cpu_store_csr(cpu, MIP, cpu_load_csr(cpu, MIP) | MIP_SEIP);
    } while (0);

    uint64_t pending = cpu_load_csr(cpu, MIE) & cpu_load_csr(cpu, MIP);
    if (pending & MIP_MEIP) {
        cpu_store_csr(cpu, MIP, cpu_load_csr(cpu, MIP) & ~MIP_MEIP);
        return MACHINE_EXTERNAL_INTERRUPT;
    }
    if (pending & MIP_MSIP) {
        cpu_store_csr(cpu, MIP, cpu_load_csr(cpu, MIP) & ~MIP_MSIP);
        return MACHINE_SOFTWARE_INTERRUPT;
    }
    if (pending & MIP_MTIP) {
        cpu_store_csr(cpu, MIP, cpu_load_csr(cpu, MIP) & ~MIP_MTIP);
        return MACHINE_TIMER_INTERRUPT;
    }
    if (pending & MIP_SEIP) {
        cpu_store_csr(cpu, MIP, cpu_load_csr(cpu, MIP) & ~MIP_SEIP);
        return SUPERVISOR_EXTERNAL_INTERRUPT;
    }
    if (pending & MIP_SSIP) {
        cpu_store_csr(cpu, MIP, cpu_load_csr(cpu, MIP) & ~MIP_SSIP);
        return SUPERVISOR_SOFTWARE_INTERRUPT;
    }
    if (pending & MIP_STIP) {
        cpu_store_csr(cpu, MIP, cpu_load_csr(cpu, MIP) & ~MIP_STIP);
        return SUPERVISOR_TIMER_INTERRUPT;
    }

    return NONE;
}

// 將整個檔案讀入到 *r 當中。
size_t read_file(FILE *f, uint8_t **r)
{
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *content = malloc(fsize + 1);
    if (fread(content, fsize, 1, f) != 1) /* less than fsize bytes */
        fatal("read file content");

    content[fsize] = 0;
    *r = content;

    return fsize;
}

// 本虛擬機只模擬單核心 single core 的狀況。
int main(int argc, char **argv)
{
    if (argc < 2) { // 使用方法 ./semu kernel.bin fs.img
        printf("Usage: %s <filename> [<image>]\n", argv[0]);
        return 2;
    }

    // 讀取 kernel.bin (作業系統核心程式映像)
    FILE *f = fopen(argv[1], "rb");
    if (!f)
        fatal("open kernel image file");

    uint8_t *binary = NULL, *disk = NULL;
    size_t fsize = read_file(f, &binary);
    fclose(f);

    if (argc == 3) {
        // 讀取 fs.img (檔案系統磁碟映像)
        // 注意：由於沒有寫回 fs.img
        // 所以下次重新執行時，檔案系統的改變會被忘記
        f = fopen(argv[2], "rb");
        if (!f)
            fatal("open disk image");
        read_file(f, &disk);
        fclose(f);
    }

    struct cpu *cpu = cpu_new(binary, fsize, disk); // 創建 CPU 物件
    free(binary);

    // 不斷執行指令並處理例外
    while (1) {
        /* Fetch instruction */
        uint64_t inst;
        exception_t e;
        if ((e = cpu_fetch(cpu, &inst)) != OK) { // 指令擷取
            cpu_take_trap(cpu, e, NONE); // 如果有例外發生
            if (exception_is_fatal(e)) // 若是致命的例外，直接離開！
                break;
            inst = 0;
        }

        cpu->pc += 4; /* advance pc */

        /* decode and execute */
        if ((e = cpu_execute(cpu, inst)) != OK) { // 指令執行
            cpu_take_trap(cpu, e, NONE); // 如果有例外發生
            if (exception_is_fatal(e)) // 若是致命的例外，直接離開！
                break;
        }

        interrupt_t intr;
        if ((intr = cpu_check_pending_interrupt(cpu)) != NONE) // 檢查是否有等待的中斷
            cpu_take_trap(cpu, OK, intr); // 若有就處理中斷
    }

    return 0;
}
