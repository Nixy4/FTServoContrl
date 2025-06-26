#include "scs.h"
#include "scs_def.h"

#include "elog.h"

#include <stdarg.h>
#include <string.h>

#define TAG "SCS"

void* scs_default_malloc(size_t size)
{
	return malloc(size);
}

void scs_default_free(void* ptr)
{
	free(ptr);
}

static scs_send_callback send= NULL;
static scs_recv_callback recv = NULL;
static scs_delay_callback delay = NULL;
static scs_gettick_callback gettick = NULL;
static scs_malloc_callback scs_malloc = scs_default_malloc;
static scs_free_callback scs_free = scs_default_free;

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

void scs_memory_callback_register(
	scs_malloc_callback malloc_cb,
	scs_free_callback free_cb
) {
	if (malloc_cb != NULL && free_cb != NULL) {
		scs_malloc = malloc_cb;
		scs_free = free_cb;
	} else {
		elog_e(TAG, "Memory callback registration failed: malloc or free callback is NULL");
	}
}

static inline uint32_t calc_size(uint32_t param_size)
{
	// magic(2) + id(1) + pro_len(1) + code(1) + param(param_size) + checksum(1)
	return 2 + 1 + 1 + 1 + param_size + 1;
}

static inline uint8_t calc_prolen(int32_t param_size)
{
	// pro_len = code(1) + param(param_size) + checksum(1)
	return 1 + param_size + 1;
}

static inline uint8_t calc_checksum(uint8_t* buf, uint32_t size)
{
	uint16_t checksum = 0;
	for (uint32_t i = BUFIDX_ID; i < BUFIDX_CHECKSUM(size); i++) // 不包括最后一个字节的校验和
	{
		checksum += buf[i];
	}
	if (checksum > 255) // 如果和大于255，则只保留低8位
	{
		checksum &= 0xFF;
	}
	checksum = ~checksum; // 按位取反
	return checksum;
}

static inline uint32_t calc_param_size(uint32_t size)
{
	return size - PACKET_MINIMUM_SIZE;
	// size - (magic(2) + id(1) + pro_len(1) + code(1) + checksum(1))
}

static void pack(uint8_t* pkt, uint32_t pktsiz,
	uint8_t id, uint8_t code, uint32_t param_len, ...)
{
	//Fill Header
	pkt[PKTIDX_MAGIC1] = MAGIC1;
	pkt[PKTIDX_MAGIC2] = MAGIC2;
	pkt[PKTIDX_ID] = id;
	pkt[PKTIDX_PRO_LEN] = calc_pro_len(param_len);
	pkt[PKTIDX_CODE] = code;
	//Fill Params
	if (param_len != 0) {
		va_list params;
		va_start(params, param_len);
		for (uint32_t i = 0; i < param_len; i++) {
			pkt[PKTIDX_PARAM(i)] = va_arg(params, int);
		}
	}
	//Fill Checksum
	pkt[PKTIDX_CHECKSUM(pktsiz)] = calc_checksum(pkt, pktsiz);
}

static void unpack(uint8_t* pkt, uint32_t pktsiz,
	uint8_t* id, uint8_t* code, uint32_t* param_len, uint8_t* param_buf)
{
	if (pkt == NULL || pktsiz < PKT_MINIMUM_SIZE) {
		return; // Invalid packet
	}
	if (pkt[PKTIDX_MAGIC1] != MAGIC1 || pkt[PKTIDX_MAGIC2] != MAGIC2) {
		return; // Invalid packet
	}
	if (id != NULL) {
		*id = pkt[PKTIDX_ID];
	}
	if (code != NULL) {
		*code = pkt[PKTIDX_CODE];
	}
	if (param_len != NULL) {
		*param_len = pkt[PKTIDX_PRO_LEN] - 2; // code(1) + checksum(1)
	}
}

void scs_ping(uint8_t id)
{
	// Calculate packet size for ping
	uint32_t pktsiz = calc_pktsiz(0);
	// Allocate memory for the packet
	uint8_t* pkt = (uint8_t*)malloc(pktsiz);
	if (pkt == NULL) {
		return; // Memory allocation failed
	}
	// Pack the ping packet
	pack(pkt, pktsiz, id, CODE_PING, 0);
	// Send the packet
	if (send != NULL) {
		send(pkt, pktsiz);
	}
	// Recveive the response
	memset(pkt, 0, pktsiz); // Clear the packet 
	if (recv != NULL) {
		pktsiz = recv(pkt, pktsiz);
	}
	uint8_t id_ = 0;
	uint8_t code_ = 0;
	unpack(pkt, pktsiz, &id_, &code_, NULL, NULL);
}
