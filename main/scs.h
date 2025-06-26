#pragma once

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCS_OK                        0
#define SCS_ERR_GENERIC              -1000
#define SCS_ERR_CALLBACK             -1
#define SCS_ERR_MEMORY_ALLOC         -2
#define SCS_ERR_BUF_SIZE             -3
#define SCS_ERR_MAGIC                -4
#define SCS_ERR_CHECKSUM             -5
#define SCS_ERR_PARAM_SIZE           -6
#define SCS_ERR_RESPONSE_ID          -7
#define SCS_ERR_PACKET_SIZE          -8

typedef void (*scs_send_callback)(uint8_t* buf, uint32_t size);
typedef uint32_t (*scs_recv_callback)(uint8_t* buf, uint32_t size);
typedef void (*scs_delay_callback)(uint32_t delay_ms);
typedef uint32_t(*scs_gettick_callback)(void);

void scs_callback_register(
	scs_send_callback send_cb, 
	scs_recv_callback recv_cb,
	scs_delay_callback delay_cb,
	scs_gettick_callback gettick_cb
);

typedef void* (*scs_malloc_callback)(size_t size);
typedef void (*scs_free_callback)(void* ptr);

void scs_memory_callback_register(
	scs_malloc_callback malloc_cb,
	scs_free_callback free_cb
);

typedef struct scs_header {
	uint8_t magic1;          // 魔术字节1
	uint8_t magic2;          // 魔术字节2
	uint8_t id;              // ID
	uint8_t prolen;          // 协议长度
	uint8_t code;            // 命令码
} scs_header;

typedef struct scs_packet {
	uint8_t magic1;          // 魔术字节1
	uint8_t magic2;          // 魔术字节2
	uint8_t id;              // ID
	uint8_t prolen;          // 协议长度
	uint8_t code;            // 命令码
	uint8_t param[];         // 参数数据
} scs_packet;

int scs_ping(uint8_t id);

#ifdef __cplusplus
}
#endif
