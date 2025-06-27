#include "scs.h"
#include "scs_def.h"

#include "elog.h"
#define TAG "scs"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

static inline
scs_size calc_pktsiz(scs_size paramlen)
{
    // magic(2) + id(1) + pro_len(1) + code(1) + param(paramlen) + checksum(1)
    return 2 + 1 + 1 + 1 + paramlen + 1;
}

static inline
scs_size calc_prolen(scs_size paramlen)
{
    // pro_len = code(1) + param(paramlen) + checksum(1)
    return 1 + paramlen + 1;
}

static inline
uint8_t calc_checksum(uint8_t* pkt, scs_size pktsiz)
{
    uint16_t checksum = 0;
    for (scs_size i = INDEX_ID; i < INDEX_CHECKSUM(pktsiz); i++) // 不包括最后一个字节的校验和
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
void elog_pkt(uint8_t* pkt, scs_size pktsiz, scs_size paramlen)
{
#define PRINT_PKT_FMT "%-20s : %-05d"
    elog_d(TAG, PRINT_PKT_FMT, "Packet Size", pktsiz); // 打印数据包长度
    elog_d(TAG, PRINT_PKT_FMT, "Magic1", pkt[INDEX_MAGIC1]); // 打印魔术字节1
    elog_d(TAG, PRINT_PKT_FMT, "Magic2", pkt[INDEX_MAGIC2]); // 打印魔术字节2
    elog_d(TAG, PRINT_PKT_FMT, "ID", pkt[INDEX_ID]); // 打印ID
    elog_d(TAG, PRINT_PKT_FMT, "Protocol Length", pkt[INDEX_PROLEN]); // 打印协议长度
    elog_d(TAG, PRINT_PKT_FMT, "Code", pkt[INDEX_CODE]); // 打印命令码
    if (paramlen > 0) {
        for (scs_size i = 0; i < paramlen; i++) {
            elog_d(TAG, PRINT_PKT_FMT, "Param", pkt[INDEX_PARAM(i)]); // 打印参数
        }
    }
    elog_d(TAG, PRINT_PKT_FMT, "Checksum", pkt[INDEX_CHECKSUM(pktsiz)]);
}

static
int fill_pkt(uint8_t* pkt, scs_size pktsiz, uint8_t id, uint8_t code, const uint8_t* params, scs_size paramlen)
{
    if (pktsiz < PKT_MIN_BUFSIZ || pktsiz > PKT_MAX_BUFSIZ) {
        elog_e(TAG, "数据包大小不合法: %d", pktsiz);
        return -1;
    }
    if (paramlen > pktsiz - 6) { // 6 = magic(2) + id(1) + pro_len(1) + code(1) + checksum(1)
        elog_e(TAG, "参数长度超过数据包限制: %d > %d", paramlen, pktsiz - 6);
        return -1;
    }
    //! 填充数据包
    pkt[INDEX_MAGIC1]     = MAGIC1;
    pkt[INDEX_MAGIC2]     = MAGIC2;
    pkt[INDEX_ID]         = id;
    pkt[INDEX_PROLEN]     = calc_prolen(paramlen);
    pkt[INDEX_CODE]       = code;
    for (scs_size i = 0; i < paramlen; i++) {
        pkt[INDEX_PARAM(i)] = params[i];
    }
    pkt[INDEX_CHECKSUM(pktsiz)] = calc_checksum(pkt, pktsiz); // 计算校验和
    return 0; // 成功
}

void scs_ping(uint8_t id)
{
    //! 指令常量准备
    scs_size pktsiz = calc_pktsiz(0);
    uint8_t* pkt = (uint8_t*)scs_port_malloc(pktsiz);
    if (pkt == NULL) {
        elog_e(TAG, "内存分配失败");
        return;
    }
    //! 填充数据包
    pkt[INDEX_MAGIC1] = MAGIC1;
    pkt[INDEX_MAGIC2] = MAGIC2;
    pkt[INDEX_ID]     = id;
    pkt[INDEX_PROLEN] = calc_prolen(0); // code(1) + param(0) + checksum(1)
    pkt[INDEX_CODE]   = CODE_PING;      // 设置Ping命令码
    // 没有参数，所以不需要填充参数
    pkt[INDEX_CHECKSUM(pktsiz)] = calc_checksum(pkt, pktsiz); // 计算校验和
    //! 调试信息
    elog_d(TAG, "发送Ping数据包");
    elog_pkt(pkt, pktsiz, 0); // 打印数据包内容
    //! 发送数据包
    scs_port_send(pkt, pktsiz);
    // 准备接收响应数据包
    memset(pkt, 0, pktsiz); // 清空数据包缓冲
    scs_port_recv(pkt, pktsiz);      // 接收数据包
    //! 调试信息
    elog_d(TAG, "接收到Ping响应数据包");
    elog_pkt(pkt, pktsiz, 0);
    //! 检查响应数据包
    if (pkt[INDEX_MAGIC1] != MAGIC1 || pkt[INDEX_MAGIC2] != MAGIC2) {
        elog_e(TAG, "接收到的包不是有效的SCS数据包");
        goto _free;
    }
    if (pkt[INDEX_ID] != id) {
        elog_e(TAG, "接收到的包ID不匹配，预期: %d, 实际: %d", id, pkt[INDEX_ID]);
        goto _free;
    }
    if (pkt[INDEX_PROLEN] != calc_prolen(0)) {
        elog_e(TAG, "接收到的包协议长度不匹配，预期: %d, 实际: %d", calc_prolen(0), pkt[INDEX_PROLEN]);
        goto _free;
    }
    if (pkt[INDEX_CHECKSUM(pktsiz)] != calc_checksum(pkt, pktsiz)) {
        elog_e(TAG, "接收到的包校验和不匹配，预期: %d, 实际: %d", pkt[INDEX_CHECKSUM(pktsiz)], calc_checksum(pkt, pktsiz));
        goto _free;
    }
_free:
    scs_port_free(pkt); //! free
}

void scs_pos_set(uint8_t id, uint16_t pos)
{
    //! 指令常量准备
	const scs_size paramlen = 2;
    const scs_size pktsiz   = calc_pktsiz(paramlen);
    uint8_t* const pkt      = (uint8_t*)scs_port_malloc(pktsiz);//! alloc
    if (pkt == NULL) {
        elog_e(TAG, "内存分配失败");
        return;
    }
    //! 填充数据包
    pkt[INDEX_MAGIC1]     = MAGIC1;
    pkt[INDEX_MAGIC2]     = MAGIC2;
    pkt[INDEX_ID]         = id;
    pkt[INDEX_PROLEN]     = calc_prolen(paramlen);
    pkt[INDEX_CODE]       = CODE_WRITE_DATA; 
    pkt[INDEX_PARAM_ADDR] = ADDR_SRAM_CTRL_GOAL_POSITION;
    pkt[INDEX_PARAM(1)]   = (pos >> 8) & 0xFF; // 高字节
    pkt[INDEX_PARAM(2)]   = pos & 0xFF;        // 低字节
    pkt[INDEX_CHECKSUM(pktsiz)] = calc_checksum(pkt, pktsiz); // 计算校验和
    //! 调试信息
    elog_d(TAG, "发送设置位置数据包");
    elog_pkt(pkt, pktsiz, paramlen); // 打印数据包内容
    //! 发送数据包
    scs_port_send(pkt, pktsiz);
    scs_port_free(pkt); //! free
}

void scs_pos_async_set(uint8_t id, uint16_t pos)
{
    //! 指令常量准备
    const scs_size paramlen = 2;
    const scs_size pktsiz = calc_pktsiz(paramlen);
    uint8_t* const pkt = (uint8_t*)scs_port_malloc(pktsiz); //! alloc
    if (pkt == NULL) {
        elog_e(TAG, "内存分配失败");
        return;
    }
    //! 填充数据包
    pkt[INDEX_MAGIC1]     = MAGIC1;
    pkt[INDEX_MAGIC2]     = MAGIC2;
    pkt[INDEX_ID]         = id;
    pkt[INDEX_PROLEN]     = calc_prolen(paramlen);
    pkt[INDEX_CODE]       = CODE_REGWRITE_DATA;
    pkt[INDEX_PARAM_ADDR] = ADDR_SRAM_CTRL_GOAL_POSITION;
    pkt[INDEX_PARAM(1)]   = (pos >> 8) & 0xFF; // 高字节
    pkt[INDEX_PARAM(2)]   = pos & 0xFF;        // 低字节
    pkt[INDEX_CHECKSUM(pktsiz)] = calc_checksum(pkt, pktsiz);
    //! 调试信息
    elog_d(TAG, "发送异步设置位置数据包");
    elog_pkt(pkt, pktsiz, paramlen); // 打印数据包内容
    //! 发送数据包
    scs_port_send(pkt, pktsiz);
    scs_port_free(pkt); //! free
}

void scs_pos_async_execute(uint8_t id)
{
    //! 指令常量准备
	const scs_size paramlen = 0; // 异步执行不需要参数
    const scs_size pktsiz = calc_pktsiz(paramlen);
    uint8_t* const pkt = (uint8_t*)scs_port_malloc(pktsiz); //! alloc
    if (pkt == NULL) {
        elog_e(TAG, "内存分配失败");
        return;
    }
    //! 填充数据包
    pkt[INDEX_MAGIC1]     = MAGIC1;
    pkt[INDEX_MAGIC2]     = MAGIC2;
    pkt[INDEX_ID]         = id;
    pkt[INDEX_PROLEN]     = calc_prolen(paramlen);
    pkt[INDEX_CODE]       = CODE_ACTION; // 设置异步执行命令
    pkt[INDEX_CHECKSUM(pktsiz)] = calc_checksum(pkt, pktsiz);
    //! 调试信息
    elog_d(TAG, "发送异步执行数据包");
    elog_pkt(pkt, pktsiz, 0); // 打印数据包内容
    //! 发送数据包
    scs_port_send(pkt, pktsiz);
    scs_port_free(pkt); //! free
}