#include "scs.h"
#include "scs_def.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

static scs_send_callback send = NULL;
static scs_recv_callback recv = NULL;
static scs_delay_callback delay = NULL;
static scs_gettick_callback gettick = NULL;

void scs_callback_register(
	scs_malloc_callback malloc_cb,
	scs_free_callback free_cb,
	scs_send_callback send_cb, 
	scs_recv_callback recv_cb,
	scs_delay_callback delay_cb,
	scs_gettick_callback gettick_cb
) {
	//1.处理内存回调
	if(malloc_cb != NULL && free_cb != NULL) {
		send = malloc_cb;
		recv = free_cb;
	} else {
		send = malloc;
		recv = free;
	}
	//2.处理数据传输回调
	if(send_cb != NULL) {
		send = send_cb;
	} else {
		send = NULL; // 默认不发送数据
	}
}

static inline 
uint32_t calc_pktsiz(uint32_t paramlen)
{
	// magic(2) + id(1) + pro_len(1) + code(1) + param(paramlen) + checksum(1)
	return 2 + 1 + 1 + 1 + paramlen + 1;
}

static inline 
uint8_t calc_prolen(int32_t paramlen)
{
	// pro_len = code(1) + param(paramlen) + checksum(1)
	return 1 + paramlen + 1;
}

static inline 
uint8_t calc_checksum(uint8_t* pkt, uint32_t pktsiz)
{
	uint16_t checksum = 0;
	for (uint32_t i = IDX_ID; i < IDX_CHECKSUM(pktsiz); i++) // 不包括最后一个字节的校验和
	{
		checksum += pkt[i];
	}
	if (checksum > 255) // 如果和大于255，则只保留低8位
	{
		checksum &= 0xFF;
	}
	uint8_t checksum_byte = (uint8_t)(checksum & 0xFF); // 取低8位
	checksum_byte = ~checksum_byte; // 取反
	return checksum_byte;
}

void scs_ping(uint8_t id)
{
	uint32_t pktsiz = calc_pktsiz(0); // 
}