/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-27     wujian       the first version
 */
#include "ringbuffer.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#define RT_NULL NULL
#define RT_ASSERT assert
#define rt_inline

#define RING_BUFFER_LEN        8
static char  *str = "Hello, World new ringbuffer32";
typedef struct rb_example {
    uint32_t a;
    uint32_t b;
    uint32_t c;
} rb_example_t;

int ringbuffer_force_example(void)
{
    uint8_t test[6] = {1,2,3,4,5,6};
    struct rt_ringbuffer * rb;
    rb = rt_ringbuffer_create(4);
    RT_ASSERT(rb != RT_NULL);

    printf("Put data to   ringbuffer, %d %d %d %d %d %d\n", test[0],test[1],test[2],test[3],test[4],test[5]);
    rt_ringbuffer_put_force(rb, (uint8_t *)&test, sizeof(test));


    uint8_t recv_data[4]={0};
    rt_ringbuffer_get(rb, (uint8_t *)&recv_data, sizeof(test));
    printf("Get data from ringbuffer, %d %d %d %d\n", recv_data[0],recv_data[1],recv_data[2],recv_data[3]);
    printf("write mirror: %d read mirror: %d\n", rb->write_mirror,rb->read_mirror);
    return 0;
}

static void *consumer_thread_char(void *arg)
{
    char ch;
    struct rt_ringbuffer * rb = (struct rt_ringbuffer *)arg;
    while (1)
    {
        if (1 == rt_ringbuffer_getchar(rb, &ch))
        {
            printf("[Consumer] <- %c\n", ch);
        }
        usleep(500000);
    }
}

int test_ringbuffer_char(void)
{
    uint16_t i = 0;
    struct rt_ringbuffer * rb;
    rb = rt_ringbuffer_create(RING_BUFFER_LEN);
    if (rb == RT_NULL)
    {
        printf("Can't create ringbffer");
        return -1;
    }

    pthread_t tid;
	pthread_create(&tid, NULL, consumer_thread_char, rb);

    while (1)
    {
        //printf("[Producer] -> %c\n", str[i]);
        rt_ringbuffer_putchar(rb, str[i++]);
        if (str[i] == '\0')
            i = 0;
        usleep(500000);
    }
    
    pthread_join(tid,NULL);
    rt_ringbuffer_destroy(rb);
    return 0;
}

static void *consumer_thread_buf(void *arg)
{
    char ch;
    rb_example_t recv_data;
    struct rt_ringbuffer * rb = (struct rt_ringbuffer *)arg;
    usleep(500000);
    while (1)
    {
        uint32_t recv = rt_ringbuffer_get(rb, (uint8_t *)&recv_data, sizeof(recv_data));
        if (recv == sizeof(recv_data))
        {
            RT_ASSERT(recv_data.a + 1 == recv_data.b);
            printf("Get data from ringbuffer, a: %d b: %d size: %d\n", recv_data.a, recv_data.b, sizeof(recv_data));
        }

        usleep(200000);
    }
}

int test_ringbuffer_buf(void)
{
    uint32_t length;
    rb_example_t data = {
        .a = 1,
        .b = 2,
    };

    struct rt_ringbuffer * rb = rt_ringbuffer_create(sizeof(rb_example_t) * 4);
    RT_ASSERT(rb != RT_NULL);

    pthread_t tid;
	pthread_create(&tid, NULL, consumer_thread_buf, rb);
    uint32_t idx = 0;
    while (1)
    {
        data.a = idx++;
        data.b = idx++;
        //printf("Put data to   ringbuffer, a: %d b: %d size: %d\n", data.a, data.b, sizeof(data));
        length = rt_ringbuffer_put(rb, (uint8_t *)&data, sizeof(data));
        RT_ASSERT(length == sizeof(data));
        usleep(500000);
    }
    
    pthread_join(tid,NULL);
    rt_ringbuffer_destroy(rb);
    return 0;
}

int main(int argc, char** argv)
{
    //test_ringbuffer_char();
    test_ringbuffer_buf();
}
