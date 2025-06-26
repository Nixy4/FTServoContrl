#include "scs.h"
#include "scs_def.h"

#include "elog.h"
#define TAG "scs"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

static scs_malloc_callback scs_malloc = NULL;
static scs_free_callback scs_free = NULL;
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
		scs_malloc = malloc_cb;
		scs_free = free_cb;
	} else {
		scs_malloc = malloc;
		scs_free = free;
		elog_w(TAG, "使用标准库内存管理");
	}
	//2.处理数据传输回调
	if(send_cb == NULL || recv_cb == NULL) {
		elog_e(TAG, "未注册数据传输回调函数");
	} else {
		send = send_cb;
		recv = recv_cb;
	}
	//3.处理延时回调
	if (delay_cb != NULL) {
		delay = delay_cb;
	} else {
		elog_w(TAG, "未注册延时回调函数, 将跳过所有延时");
	}
	//4.处理时间戳回调
	if( gettick_cb != NULL) {
		gettick = gettick_cb;
	} else {
		elog_w(TAG, "未注册时间戳回调函数，将跳过所有时间戳判断");
	}
}

static inline 
uint32_t calc_pktsiz(uint32_t paramlen)
{
	// magic(2) + id(1) + pro_len(1) + code(1) + param(paramlen) + checksum(1)
	return 2 + 1 + 1 + 1 + paramlen + 1;
}

static inline 
uint32_t calc_prolen(int32_t paramlen)
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

static
void elog_pkt(uint8_t* pkt, uint32_t pktsiz, uint32_t paramlen)
{
#define PRINT_PKT_FMT "%20s : %05d"
	elog_d(TAG, PRINT_PKT_FMT, "Packet Size", pktsiz);//打印数据包长度

	elog_d(TAG, PRINT_PKT_FMT, "Magic1", pkt[IDX_MAGIC1]);//打印魔术字节1
	elog_d(TAG, PRINT_PKT_FMT, "Magic2", pkt[IDX_MAGIC2]);//打印魔术字节
	elog_d(TAG, PRINT_PKT_FMT, "ID", pkt[IDX_ID]);//打印ID
	elog_d(TAG, PRINT_PKT_FMT, "Protocol Length", pkt[IDX_PROLEN]);//打印协议长度
	elog_d(TAG, PRINT_PKT_FMT, "Code", pkt[IDX_CODE]);//打印命令码
	if (paramlen > 0) {
		for(int i = 0; i < paramlen; i++) {
			elog_d(TAG, PRINT_PKT_FMT, "Param", pkt[IDX_PARAM(i)]);//打印参数
		}
	}
	elog_d(TAG, PRINT_PKT_FMT, "Checksum", pkt[IDX_CHECKSUM(pktsiz)]);//
}

void scs_ping(uint8_t id)
{
	uint32_t pktsiz = calc_pktsiz(0); // 
	uint8_t* pkt = (uint8_t*)scs_malloc(pktsiz);
	if (pkt == NULL) {
		elog_e(TAG, "内存分配失败");
		return;
	}
	//填充数据包
	pkt[IDX_MAGIC1] = MAGIC1;
	pkt[IDX_MAGIC2] = MAGIC2;
	pkt[IDX_ID] = id;
	pkt[IDX_PROLEN] = calc_prolen(0); // code(1) + param(0) + checksum(1)
	pkt[IDX_CODE] = CODE_PING; // 设置Ping命令码
	//没有参数，所以不需要填充参数
	pkt[IDX_CHECKSUM(pktsiz)] = calc_checksum(pkt, pktsiz); // 计算校验和
	elog_d(TAG, "发送Ping数据包");
	elog_pkt(pkt, pktsiz, 0); // 打印数据包内容
	//发送数据包
	if (send != NULL) {
		send(pkt, pktsiz);
	} else {
		elog_e(TAG, "未注册发送回调函数");
	}
	//准备接收响应数据包
	memset(pkt, 0, pktsiz); // 清空数据包缓冲
	recv(pkt, pktsiz); // 接收数据包
	//打印接收到的数据包
	elog_d(TAG, "接收到Ping响应数据包");
	elog_pkt(pkt, pktsiz, 0);
	//检查数据包
	if (pkt[IDX_MAGIC1] != MAGIC1 || pkt[IDX_MAGIC2] != MAGIC2) {
		elog_e(TAG, "接收到的包不是有效的SCS数据包");
		goto _free;
	}
	if (pkt[IDX_ID] != id) {
		elog_e(TAG, "接收到的包ID不匹配，预期: %d, 实际: %d", id, pkt[IDX_ID]);
		goto _free;
	}
	if (pkt[IDX_CODE] != CODE_PING) {
		elog_e(TAG, "接收到的包命令码不匹配，预期: %d, 实际: %d", CODE_PING, pkt[IDX_CODE]);
		goto _free;
	}
	if (pkt[IDX_PROLEN] != calc_prolen(0)) {
		elog_e(TAG, "接收到的包协议长度不匹配，预期: %d, 实际: %d", calc_prolen(0), pkt[IDX_PROLEN]);
		goto _free;
	}
	if (pkt[IDX_CHECKSUM(pktsiz)] != calc_checksum(pkt, pktsiz)) {
		elog_e(TAG, "接收到的包校验和不匹配，预期: %d, 实际: %d", pkt[IDX_CHECKSUM(pktsiz)], calc_checksum(pkt, pktsiz));
		goto _free;
	}
_free:
	scs_free(pkt); // 释放数据包内存
}