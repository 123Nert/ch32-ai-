#ifndef __OLED_DEMO_H
#define __OLED_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v30x.h"

/*
 * 文件说明：
 * 1. 该模块只处理 SH1106 演示效果，不关心底层总线实现细节。
 * 2. main.c 通过这里暴露的 3 个接口完成启动提示、开机自检和循环演示。
 */

/* 打印 OLED 初始化结果和当前接线说明。 */
void OLED_Demo_PrintStartup(void);

/* 显示上电自检画面，确认屏幕已经正常响应。 */
void OLED_Demo_BootSplash(void);

/* 运行一整轮演示效果。 */
void OLED_Demo_RunCycle(void);

#ifdef __cplusplus
}
#endif

#endif
