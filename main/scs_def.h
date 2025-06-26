#pragma once

#include <stdint.h>
#include <stdio.h>

#define RXBUFSIZ					  BUFSIZ

#define MAGIC1						  0xFF
#define MAGIC2						  0xFF

#define CODE_PING                     0x01
#define CODE_READ_DATA                0x02
#define CODE_WRITE_DATA               0x03
#define CODE_REGWRITE_DATA            0x04
#define CODE_ACTION                   0x05
#define CODE_SYCNREAD_DATA            0x82
#define CODE_SYCNWRITE_DATA           0x83
#define CODE_RESET                    0x0A
#define CODE_POSITION_CALIBRATION     0x0B
#define CODE_PARAM_RECOVER            0x06
#define CODE_PARAM_BACKUP             0x09
#define CODE_REBOOT                   0x08

#define BUFIDX_MAGIC1                 0
#define BUFIDX_MAGIC2                 1
#define BUFIDX_ID                     2
#define BUFIDX_PROLEN				  3
#define BUFIDX_CODE                   4
#define BUFIDX_PARAM_START            5
#define BUFIDX_PARAM_ADDR             5
#define BUFIDX_PARAM(x)               (BUFIDX_PARAM_START + (x))
#define BUFIDX_PARAM_END(size)        (size - 2) // 最后一个字节是校验和
#define BUFIDX_CHECKSUM(size)         (size - 1)

#define PACKET_MINIMUM_SIZE			  6 // magic(2) + id(1) + pro_len(1) + code(1) + checksum(1)