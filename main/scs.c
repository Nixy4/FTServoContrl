#include "scs.h"
#include "scs_def.h"

#include "elog.h"

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

#define TAG "SCS"

int scs_packet_send(uint8_t id, uint8_t code, uint32_t param_size, ...)
{
	uint32_t size = calc_size(param_size);
	uint8_t* buf = (uint8_t*)malloc(size);
	if (buf == NULL) {
		elog_e(TAG, "Memory allocation failed for packet buffer");
		return SCS_ERR_MEMORY_ALLOC; // 内存分配失败
	}
	scs_packet* packet = (scs_packet*)buf;
	packet->magic1 = MAGIC1;
	packet->magic2 = MAGIC2;
	packet->id = id;
	packet->prolen = calc_prolen(param_size);
	packet->code = code;
	if( param_size > 0) {
		va_list args;
		va_start(args, param_size);
		for (uint32_t i = 0; i < param_size; i++) {
			packet->param[i] = (uint8_t)va_arg(args, uint8_t);
		}
		va_end(args);
	}
	buf[BUFIDX_CHECKSUM(size)] = calc_checksum(buf, size);

	if (send != NULL) {
		send(buf, size);
		free(buf);
		elog_d(TAG, "Packet sent: ID=%d, Code=%d, Size=%d", id, code, size);
		return SCS_OK;
	} else {
		free(buf);
		elog_e(TAG, "Send callback not registered");
		return SCS_ERR_CALLBACK; //回调未注册
	}
}

uint32_t scs_packet_recv(uint8_t* buf, uint32_t size)
{
	if( buf == NULL || size < PACKET_MINIMUM_SIZE) {
		elog_e(TAG, "Invalid buffer or size for receiving packet");
		return SCS_ERR_BUF_SIZE; // 无效的缓冲区或大小
	}
	return recv(buf, size);
}

int scs_packet_parse(uint8_t* buf, uint32_t rx_size, 
	scs_header* header, uint8_t* param_buf, uint32_t* param_size)
{
	if (buf == NULL || rx_size < PACKET_MINIMUM_SIZE || header == NULL) {
		elog_e(TAG, "Invalid parameters for packet parsing");
		return SCS_ERR_GENERIC; // 无效的参数
	}
	
	if (buf[BUFIDX_MAGIC1] != MAGIC1 || buf[BUFIDX_MAGIC2] != MAGIC2) {
		elog_e(TAG, "Magic bytes mismatch: %02X %02X", buf[BUFIDX_MAGIC1], buf[BUFIDX_MAGIC2]);
		return SCS_ERR_MAGIC; // 魔术字节不匹配
	}

	uint8_t checksum = calc_checksum(buf, rx_size);
	if (checksum != buf[BUFIDX_CHECKSUM(rx_size)]) {
		elog_e(TAG, "Checksum mismatch: calculated %02X, received %02X", checksum, buf[BUFIDX_CHECKSUM(rx_size)]);
		return SCS_ERR_CHECKSUM; // 校验和不匹配
	}

	header->magic1 = buf[BUFIDX_MAGIC1];
	header->magic2 = buf[BUFIDX_MAGIC2];
	header->id = buf[BUFIDX_ID];
	header->prolen = buf[BUFIDX_PROLEN];
	header->code = buf[BUFIDX_CODE];

	uint32_t psize = calc_param_size(rx_size);
	if (psize > 0) {
		if (param_buf == NULL || param_size == NULL) {
			elog_e(TAG, "Invalid parameter buffer or size pointer");
			return SCS_ERR_GENERIC; // 无效的参数缓冲区或大小指针
		}
		if (psize > rx_size - PACKET_MINIMUM_SIZE) {
			elog_e(TAG, "Parameter size exceeds received size: expected %d, received %d", psize, rx_size - PACKET_MINIMUM_SIZE);
			return SCS_ERR_PARAM_SIZE; // 参数大小超过实际接收的大小
		}
		*param_size = psize;
		memcpy(param_buf, &buf[BUFIDX_PARAM_START], *param_size);
	}
	return SCS_OK;
}

int scs_ping(uint8_t id)
{
	//发送PING命令
	int err = scs_packet_send(id, CODE_PING, 0);
	if (err < 0) {
		return err; // 发送失败
	}
	//接收响应
	uint8_t buf[RXBUFSIZ] = { 0 };
	uint32_t rx_size = scs_packet_recv(buf, RXBUFSIZ);
	if (rx_size < PACKET_MINIMUM_SIZE) {
		return SCS_ERR_PACKET_SIZE; // 接收数据包大小小于最小要求
	}
	scs_header header;
	err = scs_packet_parse(buf, rx_size, &header, NULL, NULL);
	// 检查解析结果
	if (err < 0) {
		return err; // 解析失败
	}
	if (header.id != id) {
		return SCS_ERR_RESPONSE_ID; // 响应ID不匹配
	}
	return SCS_OK; // 成功
}
