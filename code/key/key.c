/*
* Copyright (c) 2006-2018, RT-Thread Development Team
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date           Author       Notes
* 2018-10-17     flybreak      the first version
*/

#include <rtthread.h>
#include <rtdevice.h>

#include "key.h"

extern struct player player;

static struct button btn_last, btn_next, btn_play;

static uint8_t btn_last_read(void)
{
    return rt_pin_read(KEY_LAST_PIN);
}

static uint8_t btn_next_read(void)
{
    return rt_pin_read(KEY_NEXT_PIN);
}

static uint8_t btn_play_read(void)
{
    return rt_pin_read(KEY_PLAY_PIN);
}

static void btn_last_cb(void *btn)
{
    uint32_t btn_event_val;
    uint8_t volume;

    btn_event_val = get_button_event((struct button *)btn);

    switch (btn_event_val)
    {
    case SINGLE_CLICK:
        player_control(&player, PLAYER_CMD_LAST, RT_NULL);

        /*打印一次播放状态*/
        player_show(&player);
        break;
    case LONG_PRESS_HOLD:
        player_control(&player, PLAYER_CMD_GET_VOL, &volume);
        if (volume > 1)
        {
            volume--;
            player_control(&player, PLAYER_CMD_SET_VOL, &volume);
        }
        break;
    }
}

static void btn_next_cb(void *btn)
{
    uint32_t btn_event_val;
    uint8_t volume;

    btn_event_val = get_button_event((struct button *)btn);

    switch (btn_event_val)
    {
    case SINGLE_CLICK:
        player_control(&player, PLAYER_CMD_NEXT, RT_NULL);

        /*打印一次播放状态*/
        player_show(&player);
        break;
    case LONG_PRESS_HOLD:
        player_control(&player, PLAYER_CMD_GET_VOL, &volume);
        if (volume < 99)
        {
            volume++;
            player_control(&player, PLAYER_CMD_SET_VOL, &volume);
        }
        break;
    }
}

static void btn_play_cb(void *btn)
{
    uint32_t btn_event_val;

    btn_event_val = get_button_event((struct button *)btn);

    switch (btn_event_val)
    {
    case SINGLE_CLICK:
        /* 根据当前播放状态切换播放状态 */
        if (player.status == PLAYER_RUNNING)
        {
            player_control(&player, PLAYER_CMD_STOP, RT_NULL);
        }
        else
        {
            player_control(&player, PLAYER_CMD_PLAY, RT_NULL);
        }

        /*打印一次播放状态*/
        player_show(&player);
        break;
    }
}

static void btn_scan(void *p)
{
    button_ticks();
}

int key_init(void)
{
    rt_timer_t timer = RT_NULL;

    /* last key init */
    rt_pin_mode(KEY_LAST_PIN, PIN_MODE_INPUT_PULLUP);/*此处按键是对地的，pin mode设置为PIN_MODE_INPUT_PULLUP（上拉输入）*/
    button_init(&btn_last, btn_last_read, PIN_LOW);
    button_attach(&btn_last, SINGLE_CLICK,     btn_last_cb);
    button_attach(&btn_last, LONG_PRESS_HOLD,  btn_last_cb);
    button_start(&btn_last);

    /* next key init */
    rt_pin_mode(KEY_NEXT_PIN, PIN_MODE_INPUT_PULLUP);
    button_init(&btn_next, btn_next_read, PIN_LOW);
    button_attach(&btn_next, SINGLE_CLICK,     btn_next_cb);
    button_attach(&btn_next, LONG_PRESS_HOLD,  btn_next_cb);
    button_start(&btn_next);

    /* play key init */
    rt_pin_mode(KEY_PLAY_PIN, PIN_MODE_INPUT_PULLDOWN);/*此处按键是对3.3V的，pin mode设置为PIN_MODE_INPUT_PULLDOWN（下拉输入）*/
    button_init(&btn_play, btn_play_read, PIN_LOW);
    button_attach(&btn_play, SINGLE_CLICK,     btn_play_cb);
    button_start(&btn_play);
    
    /* 创建定时器1 */
    timer = rt_timer_create("timer1", /* 定时器名字是 timer1 */
                             btn_scan, /* 超时时回调的处理函数 */
                             RT_NULL, /* 超时函数的入口参数 */
                             RT_TICK_PER_SECOND * TICKS_INTERVAL / 1000, /* 定时长度，以OS Tick为单位，即10个OS Tick */
                             RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER); /* 周期性定时器 */
    /* 启动定时器 */
    if (timer != RT_NULL)
        rt_timer_start(timer);

    return RT_EOK;
}
