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
#define INDEX_PARAM_END(pktsiz)                 (pktsiz - 2) // ���һ���ֽ���У���
#define INDEX_CHECKSUM(pktsiz)                  (pktsiz - 1)

#define ADDR_BOOTROM_SW_MAJOR                   0x00
#define ADDR_BOOTROM_SW_MINOR                   0x01
#define ADDR_BOOTROM_HW_ENDIAN                  0x02
#define ADDR_BOOTROM_HW_MAJOR                   0x03
#define ADDR_BOOTROM_HW_MINOR                   0x04

#define ADDR_EEPROM_ID                          0x05 // ���ID, 1�ֽ�, ��ʼ:1, ��д, 0~253, ��, ������Ψһ����ID��ʶ
#define ADDR_EEPROM_BAUDRATE                    0x06 // ������, 1�ֽ�, ��ʼ:0, ��д, 0~7, 0-7�ֱ��������:1000000(0),500000(1),250000(2),128000(3),115200(4),76800(5),57600(6),38400(7)
#define ADDR_EEPROM_RESERVED0                   0x07 // �޶���, 1�ֽ�, ��д
#define ADDR_EEPROM_RESP_MODE                   0x08 // Ӧ��״̬����, 1�ֽ�, ��ʼ:1, ��д, 0~1, 0:����/ PING�ⲻ����Ӧ��, 1:����ָ���Ӧ��
#define ADDR_EEPROM_MIN_ANGLE_LIMIT             0x09 // ��С�Ƕ�����, 2�ֽ�, ��ʼ:20, ��д, 0~1023, ��, С�����Ƕ�����, =0������ģʽ
#define ADDR_EEPROM_MAX_ANGLE_LIMIT             0x0B // ���Ƕ�����, 2�ֽ�, ��ʼ:1003, ��д, 1~1023, ��, ������С�Ƕ�����, =0������ģʽ
#define ADDR_EEPROM_MAX_TEMP                    0x0D // ����¶�����, 1�ֽ�, ��ʼ:70, ��д, 0~100, ��C
#define ADDR_EEPROM_MAX_VOLTAGE                 0x0E // ��������ѹ, 1�ֽ�, ��д, 0~254, 0.1V
#define ADDR_EEPROM_MIN_VOLTAGE                 0x0F // ��������ѹ, 1�ֽ�, ��ʼ:40, ��д, 0~254, 0.1V
#define ADDR_EEPROM_MAX_TORQUE                  0x10 // ���Ť��, 2�ֽ�, ��ʼ:1000, ��д, 0~1000, 0.1%, �ϵ縳ֵ��48�ŵ�ַת������
#define ADDR_EEPROM_PHASE                       0x12 // ��λ, 1�ֽ�, ��д, 0~254, ���⹦���ֽ�, ���ر����󲻿��޸�
#define ADDR_EEPROM_UNLOAD_CONDITION            0x13 // ж������, 1�ֽ�, ��д, 0~254, ��Ӧλ1Ϊ��������, 0Ϊ�ر�
#define ADDR_EEPROM_LED_ALARM                   0x14 // LED��������, 1�ֽ�, ��д, 0~254, ��Ӧλ1Ϊ�������Ʊ���, 0Ϊ�ر�
#define ADDR_EEPROM_POS_P                       0x15 // λ�û�P����ϵ��, 1�ֽ�, ��д, 0~254
#define ADDR_EEPROM_POS_D                       0x16 // λ�û�D΢��ϵ��, 1�ֽ�, ��д, 0~254
#define ADDR_EEPROM_RESERVED1                   0x17 // �޶���, 1�ֽ�, ��д
#define ADDR_EEPROM_MIN_START_FORCE             0x18 // ��С������, 1�ֽ�, ��д, 0~254, 0.1%
#define ADDR_EEPROM_RESERVED2                   0x19 // �޶���, 1�ֽ�, ��д
#define ADDR_EEPROM_POS_DEADZONE_POS            0x1A // ����������, 1�ֽ�, ��ʼ:1, ��д, 0~16, ��
#define ADDR_EEPROM_POS_DEADZONE_NEG            0x1B // ����������, 1�ֽ�, ��ʼ:1, ��д, 0~16, ��
// 0x1C ~ 0x24 �޶���, 1�ֽ�, ��д
#define ADDR_EEPROM_HOLD_TORQUE                 0x25 // ����Ť��, 1�ֽ�, ��ʼ:20, ��д, 0~254, 1%
#define ADDR_EEPROM_PROTECT_TIME                0x26 // ����ʱ��, 1�ֽ�, ��ʼ:200, ��д, 0~254, 10ms
#define ADDR_EEPROM_OVERLOAD_TORQUE             0x24 // ����Ť��, 1�ֽ�, ��ʼ:80, ��д, 0~254, 1%

#define ADDR_SRAM_CTRL_TORQUE_SWITCH            0x28 // Ť�ؿ���, 1�ֽ�, ��ʼ:0, ��д, 0~2, д0:�ر�/����, 1:��, 2:����
#define ADDR_SRAM_CTRL_RESERVED0                0x29 // �޶���, 1�ֽ�, ��д
#define ADDR_SRAM_CTRL_GOAL_POSITION            0x2A // Ŀ��λ��, 2�ֽ�, ��ʼ:0, ��д, 0~1023, ��
#define ADDR_SRAM_CTRL_RUNNING_TIME             0x2C // ����ʱ��, 2�ֽ�, ��ʼ:0, ��д, 0~9999/-1000~1000, 1ms/0.1%
#define ADDR_SRAM_CTRL_RUNNING_SPEED            0x2E // �����ٶ�, 2�ֽ�, �������, ��д, 0~1000, ��/s
#define ADDR_SRAM_CTRL_LOCK_FLAG                0x30 // ����־, 1�ֽ�, ��ʼ:1, ��д, 0~1, 0:�ر�д����, 1:��д����
// 0x31 ~ 0x36 �޶���, 1�ֽ�, ��д

#define ADDR_SRAM_FEEDBACK_PRESENT_POSITION     0x38 // ��ǰλ��, 2�ֽ�, ֻ��, ��
#define ADDR_SRAM_FEEDBACK_PRESENT_SPEED        0x3A // ��ǰ�ٶ�, 2�ֽ�, ֻ��, ��/s
#define ADDR_SRAM_FEEDBACK_PRESENT_LOAD         0x3C // ��ǰ����, 2�ֽ�, ֻ��, 0.1%
#define ADDR_SRAM_FEEDBACK_PRESENT_VOLTAGE      0x3E // ��ǰ��ѹ, 1�ֽ�, ֻ��, 0.1V
#define ADDR_SRAM_FEEDBACK_PRESENT_TEMPERATURE  0x3F // ��ǰ�¶�, 1�ֽ�, ֻ��, ��C
#define ADDR_SRAM_FEEDBACK_REGWRITE_FLAG        0x40 // �첽д��־, 1�ֽ�, ��ʼ:0, ֻ��
#define ADDR_SRAM_FEEDBACK_STATUS               0x41 // ���״̬, 1�ֽ�, ��ʼ:0, ֻ��
#define ADDR_SRAM_FEEDBACK_MOVING_FLAG          0x42 // �ƶ���־, 1�ֽ�, ��ʼ:0, ֻ��