#include "scs.h"
#include "scs_def.h"

#include <stdarg.h>
#include <string.h>

static scs_send_callback send= NULL;
static scs_recv_callback recv = NULL;
static scs_delay_callback delay = NULL;
static scs_gettick_callback gettick = NULL;

void scs_callback_register(
	scs_send_callback send_cb, 
	scs_recv_callback recv_cb,
	scs_delay_callback delay_cb,
	scs_gettick_callback gettick_cb
) {
	//参数校验
	if (send_cb == NULL || recv_cb == NULL || delay_cb == NULL || gettick_cb == NULL) {
		return; // Invalid callback
	}
	else {
		send = send_cb;
		recv = recv_cb;
		delay = delay_cb;
		gettick = gettick_cb;
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