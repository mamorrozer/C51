/**
 * @file main.c
 * @brief 主程序文件
 * @details 实现串口数据接收、回显和EEPROM数据保存功能
 */

#include <REGX52.H>
#include "AT24C02.h"
#include "UART.h"

/**
 * @var save
 * @brief 保存当前数据字节
 * @details 用于存储从串口接收的数据，以及写入EEPROM的数据
 */
volatile unsigned char save = 0x00;

/**
 * @var need_save
 * @brief 是否需要将数据写入EEPROM的标志位
 * @details 使用位变量节省资源，当需要保存数据时设置为1
 */
volatile bit need_save = 0;

/**
 * @brief 主函数
 * @details 初始化系统，读取EEPROM数据，处理串口数据保存
 * @return 无
 */
void main()
{
    // 1. 初始化串口（波特率9600，模式1，允许接收）
    UART_Init();

    // 2. 上电后从EEPROM地址1读取之前保存的数据
    save = AT24C02_ReadByte(1);

    // 3. 将读取的数据通过串口发送到电脑，用于显示断电前的值
    UART_Transfer(save);

    // 4. 主循环：检测need_save标志，将串口新收到的数据写入EEPROM
    while (1)
    {
        if (need_save)              // 中断中设置标志，避免在中断内执行耗时操作
        {
            need_save = 0;          // 清除标志
            AT24C02_WriteByte(1, save);  // 将数据写入EEPROM（含5ms延时，不影响中断）
        }
    }
}

/**
 * @brief 串口中断服务函数
 * @details 处理串口接收中断，读取数据并设置保存标志
 * @note 中断号4，对应串口中断
 */
void UART_IT() interrupt 4
{
    if (RI)                         // 判断是否为接收中断
    {
        save = SBUF;                // 读取接收到的字节
        RI = 0;                     // 清除接收中断标志
        UART_Transfer(save);        // 回显：将收到的数据立即发送回电脑
        need_save = 1;              // 设置标志，通知主循环需要保存到EEPROM
        // 注意：原代码在此处直接调用AT24C02_WriteByte，其内部有5ms延时，会阻塞后续接收。
        // 优化后改为设置标志，在主循环中执行写入，保证串口中断响应及时，避免丢数据。
    }
}