#pragma once

#include <stdint.h>
#include <stdio.h>

#define PKT_MIN_BUFSIZ                          6  // magic(2) + id(1) + pro_len(1) + code(1) + checksum(1)
#define PKT_MAX_BUFSIZ                          BUFSIZ

#define MAGIC1                                  0xFF
#define MAGIC2                                  0xFF

#define CODE_PING                               0x01
#define CODE_READ_DATA                          0x02
#define CODE_WRITE_DATA                         0x03
#define CODE_REGWRITE_DATA                      0x04
#define CODE_ACTION                             0x05
#define CODE_SYCNREAD_DATA                      0x82
#define CODE_SYCNWRITE_DATA                     0x83
#define CODE_RESET                              0x0A
#define CODE_POSITION_CALIBRATION               0x0B
#define CODE_PARAM_RECOVER                      0x06
#define CODE_PARAM_BACKUP                       0x09
#define CODE_REBOOT                             0x08

#define INDEX_MAGIC1                            0
#define INDEX_MAGIC2                            1
#define INDEX_ID                                2
#define INDEX_PROLEN                            3
#define INDEX_CODE                              4
#define INDEX_PARAM_START                       5
#define INDEX_PARAM_ADDR                        5
#define INDEX_PARAM(x)                          (INDEX_PARAM_START + (x))
#define INDEX_PARAM_END(pktsiz)                 (pktsiz - 2) // 最后一个字节是校验和
#define INDEX_CHECKSUM(pktsiz)                  (pktsiz - 1)

#define ADDR_BOOTROM_SW_MAJOR                   0x00
#define ADDR_BOOTROM_SW_MINOR                   0x01
#define ADDR_BOOTROM_HW_ENDIAN                  0x02
#define ADDR_BOOTROM_HW_MAJOR                   0x03
#define ADDR_BOOTROM_HW_MINOR                   0x04

#define ADDR_EEPROM_ID                          0x05 // 舵机ID, 1字节, 初始:1, 读写, 0~253, 号, 总线上唯一的主ID标识
#define ADDR_EEPROM_BAUDRATE                    0x06 // 波特率, 1字节, 初始:0, 读写, 0~7, 0-7分别代表波特率:1000000(0),500000(1),250000(2),128000(3),115200(4),76800(5),57600(6),38400(7)
#define ADDR_EEPROM_RESERVED0                   0x07 // 无定义, 1字节, 读写
#define ADDR_EEPROM_RESP_MODE                   0x08 // 应答状态级别, 1字节, 初始:1, 读写, 0~1, 0:除读/ PING外不返回应答, 1:所有指令返回应答
#define ADDR_EEPROM_MIN_ANGLE_LIMIT             0x09 // 最小角度限制, 2字节, 初始:20, 读写, 0~1023, 步, 小于最大角度限制, =0进入电机模式
#define ADDR_EEPROM_MAX_ANGLE_LIMIT             0x0B // 最大角度限制, 2字节, 初始:1003, 读写, 1~1023, 步, 大于最小角度限制, =0进入电机模式
#define ADDR_EEPROM_MAX_TEMP                    0x0D // 最高温度上限, 1字节, 初始:70, 读写, 0~100, °C
#define ADDR_EEPROM_MAX_VOLTAGE                 0x0E // 最高输入电压, 1字节, 读写, 0~254, 0.1V
#define ADDR_EEPROM_MIN_VOLTAGE                 0x0F // 最低输入电压, 1字节, 初始:40, 读写, 0~254, 0.1V
#define ADDR_EEPROM_MAX_TORQUE                  0x10 // 最大扭矩, 2字节, 初始:1000, 读写, 0~1000, 0.1%, 上电赋值给48号地址转矩限制
#define ADDR_EEPROM_PHASE                       0x12 // 相位, 1字节, 读写, 0~254, 特殊功能字节, 无特别需求不可修改
#define ADDR_EEPROM_UNLOAD_CONDITION            0x13 // 卸载条件, 1字节, 读写, 0~254, 对应位1为开启保护, 0为关闭
#define ADDR_EEPROM_LED_ALARM                   0x14 // LED报警条件, 1字节, 读写, 0~254, 对应位1为开启闪灯报警, 0为关闭
#define ADDR_EEPROM_POS_P                       0x15 // 位置环P比例系数, 1字节, 读写, 0~254
#define ADDR_EEPROM_POS_D                       0x16 // 位置环D微分系数, 1字节, 读写, 0~254
#define ADDR_EEPROM_RESERVED1                   0x17 // 无定义, 1字节, 读写
#define ADDR_EEPROM_MIN_START_FORCE             0x18 // 最小启动力, 1字节, 读写, 0~254, 0.1%
#define ADDR_EEPROM_RESERVED2                   0x19 // 无定义, 1字节, 读写
#define ADDR_EEPROM_POS_DEADZONE_POS            0x1A // 正向不灵敏区, 1字节, 初始:1, 读写, 0~16, 步
#define ADDR_EEPROM_POS_DEADZONE_NEG            0x1B // 负向不灵敏区, 1字节, 初始:1, 读写, 0~16, 步
// 0x1C ~ 0x24 无定义, 1字节, 读写
#define ADDR_EEPROM_HOLD_TORQUE                 0x25 // 保持扭矩, 1字节, 初始:20, 读写, 0~254, 1%
#define ADDR_EEPROM_PROTECT_TIME                0x26 // 保护时间, 1字节, 初始:200, 读写, 0~254, 10ms
#define ADDR_EEPROM_OVERLOAD_TORQUE             0x24 // 过载扭矩, 1字节, 初始:80, 读写, 0~254, 1%

#define ADDR_SRAM_CTRL_TORQUE_SWITCH            0x28 // 扭矩开关, 1字节, 初始:0, 读写, 0~2, 写0:关闭/自由, 1:打开, 2:阻尼
#define ADDR_SRAM_CTRL_RESERVED0                0x29 // 无定义, 1字节, 读写
#define ADDR_SRAM_CTRL_GOAL_POSITION            0x2A // 目标位置, 2字节, 初始:0, 读写, 0~1023, 步
#define ADDR_SRAM_CTRL_RUNNING_TIME             0x2C // 运行时间, 2字节, 初始:0, 读写, 0~9999/-1000~1000, 1ms/0.1%
#define ADDR_SRAM_CTRL_RUNNING_SPEED            0x2E // 运行速度, 2字节, 出厂最大, 读写, 0~1000, 步/s
#define ADDR_SRAM_CTRL_LOCK_FLAG                0x30 // 锁标志, 1字节, 初始:1, 读写, 0~1, 0:关闭写入锁, 1:打开写入锁
// 0x31 ~ 0x36 无定义, 1字节, 读写

#define ADDR_SRAM_FEEDBACK_PRESENT_POSITION     0x38 // 当前位置, 2字节, 只读, 步
#define ADDR_SRAM_FEEDBACK_PRESENT_SPEED        0x3A // 当前速度, 2字节, 只读, 步/s
#define ADDR_SRAM_FEEDBACK_PRESENT_LOAD         0x3C // 当前负载, 2字节, 只读, 0.1%
#define ADDR_SRAM_FEEDBACK_PRESENT_VOLTAGE      0x3E // 当前电压, 1字节, 只读, 0.1V
#define ADDR_SRAM_FEEDBACK_PRESENT_TEMPERATURE  0x3F // 当前温度, 1字节, 只读, °C
#define ADDR_SRAM_FEEDBACK_REGWRITE_FLAG        0x40 // 异步写标志, 1字节, 初始:0, 只读
#define ADDR_SRAM_FEEDBACK_STATUS               0x41 // 舵机状态, 1字节, 初始:0, 只读
#define ADDR_SRAM_FEEDBACK_MOVING_FLAG          0x42 // 移动标志, 1字节, 初始:0, 只读