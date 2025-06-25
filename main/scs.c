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

static inline uint32_t calc_pktsiz(uint32_t param_len)
{
	// magic(2) + id(1) + pro_len(1) + code(1) + param(param_len) + checksum(1)
	return 2 + 1 + 1 + 1 + param_len + 1;
}

static inline uint8_t calc_pro_len(int32_t param_len)
{
	// pro_len = code(1) + param(param_len) + checksum(1)
	return 1 + param_len + 1;
}

static inline uint8_t calc_checksum(uint8_t* pkt, uint32_t pktsiz)
{
	uint16_t checksum = 0;
	for (uint32_t i = PKTIDX_ID; i < PKTIDX_CHECKSUM(pktsiz); i++) // 不包括最后一个字节的校验和
	{
		checksum += pkt[i];
	}
	if (checksum > 255) // 如果和大于255，则只保留低8位
	{
		checksum &= 0xFF;
	}
	checksum = ~checksum; // 按位取反
	return checksum;
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

static void unpack(uint8_t* pkt, uint32_t pktsiz ,
	uint8_t* id, uint8_t* code, uint32_t* param_len, uint8_t* param_buf)
{
	if( pkt == NULL || pktsiz < PKT_MINIMUM_SIZE) {
		return; // Invalid packet
	}
	if( pkt[PKTIDX_MAGIC1] != MAGIC1 || pkt[PKTIDX_MAGIC2] != MAGIC2) {
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