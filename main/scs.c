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
    for (scs_size i = INDEX_ID; i < INDEX_CHECKSUM(pktsiz); i++) // ���������һ���ֽڵ�У���
    {
        checksum += pkt[i];
    }
    if (checksum > 255) // ����ʹ���255����ֻ������8λ
    {
        checksum &= 0xFF;
    }
    uint8_t checksum_byte = (uint8_t)(checksum & 0xFF); // ȡ��8λ
    checksum_byte = ~checksum_byte; // ȡ��
    return checksum_byte;
}

static
void elog_pkt(uint8_t* pkt, scs_size pktsiz, scs_size paramlen)
{
#define PRINT_PKT_FMT "%-20s : %-05d"
    elog_d(TAG, PRINT_PKT_FMT, "Packet Size", pktsiz); // ��ӡ���ݰ�����
    elog_d(TAG, PRINT_PKT_FMT, "Magic1", pkt[INDEX_MAGIC1]); // ��ӡħ���ֽ�1
    elog_d(TAG, PRINT_PKT_FMT, "Magic2", pkt[INDEX_MAGIC2]); // ��ӡħ���ֽ�2
    elog_d(TAG, PRINT_PKT_FMT, "ID", pkt[INDEX_ID]); // ��ӡID
    elog_d(TAG, PRINT_PKT_FMT, "Protocol Length", pkt[INDEX_PROLEN]); // ��ӡЭ�鳤��
    elog_d(TAG, PRINT_PKT_FMT, "Code", pkt[INDEX_CODE]); // ��ӡ������
    if (paramlen > 0) {
        for (scs_size i = 0; i < paramlen; i++) {
            elog_d(TAG, PRINT_PKT_FMT, "Param", pkt[INDEX_PARAM(i)]); // ��ӡ����
        }
    }
    elog_d(TAG, PRINT_PKT_FMT, "Checksum", pkt[INDEX_CHECKSUM(pktsiz)]);
}

static
int fill_pkt(uint8_t* pkt, scs_size pktsiz, uint8_t id, uint8_t code, const uint8_t* params, scs_size paramlen)
{
    if (pktsiz < PKT_MIN_BUFSIZ || pktsiz > PKT_MAX_BUFSIZ) {
        elog_e(TAG, "���ݰ���С���Ϸ�: %d", pktsiz);
        return -1;
    }
    if (paramlen > pktsiz - 6) { // 6 = magic(2) + id(1) + pro_len(1) + code(1) + checksum(1)
        elog_e(TAG, "�������ȳ������ݰ�����: %d > %d", paramlen, pktsiz - 6);
        return -1;
    }
    //! ������ݰ�
    pkt[INDEX_MAGIC1]     = MAGIC1;
    pkt[INDEX_MAGIC2]     = MAGIC2;
    pkt[INDEX_ID]         = id;
    pkt[INDEX_PROLEN]     = calc_prolen(paramlen);
    pkt[INDEX_CODE]       = code;
    for (scs_size i = 0; i < paramlen; i++) {
        pkt[INDEX_PARAM(i)] = params[i];
    }
    pkt[INDEX_CHECKSUM(pktsiz)] = calc_checksum(pkt, pktsiz); // ����У���
    return 0; // �ɹ�
}

void scs_ping(uint8_t id)
{
    //! ָ���׼��
    scs_size pktsiz = calc_pktsiz(0);
    uint8_t* pkt = (uint8_t*)scs_port_malloc(pktsiz);
    if (pkt == NULL) {
        elog_e(TAG, "�ڴ����ʧ��");
        return;
    }
    //! ������ݰ�
    pkt[INDEX_MAGIC1] = MAGIC1;
    pkt[INDEX_MAGIC2] = MAGIC2;
    pkt[INDEX_ID]     = id;
    pkt[INDEX_PROLEN] = calc_prolen(0); // code(1) + param(0) + checksum(1)
    pkt[INDEX_CODE]   = CODE_PING;      // ����Ping������
    // û�в��������Բ���Ҫ������
    pkt[INDEX_CHECKSUM(pktsiz)] = calc_checksum(pkt, pktsiz); // ����У���
    //! ������Ϣ
    elog_d(TAG, "����Ping���ݰ�");
    elog_pkt(pkt, pktsiz, 0); // ��ӡ���ݰ�����
    //! �������ݰ�
    scs_port_send(pkt, pktsiz);
    // ׼��������Ӧ���ݰ�
    memset(pkt, 0, pktsiz); // ������ݰ�����
    scs_port_recv(pkt, pktsiz);      // �������ݰ�
    //! ������Ϣ
    elog_d(TAG, "���յ�Ping��Ӧ���ݰ�");
    elog_pkt(pkt, pktsiz, 0);
    //! �����Ӧ���ݰ�
    if (pkt[INDEX_MAGIC1] != MAGIC1 || pkt[INDEX_MAGIC2] != MAGIC2) {
        elog_e(TAG, "���յ��İ�������Ч��SCS���ݰ�");
        goto _free;
    }
    if (pkt[INDEX_ID] != id) {
        elog_e(TAG, "���յ��İ�ID��ƥ�䣬Ԥ��: %d, ʵ��: %d", id, pkt[INDEX_ID]);
        goto _free;
    }
    if (pkt[INDEX_PROLEN] != calc_prolen(0)) {
        elog_e(TAG, "���յ��İ�Э�鳤�Ȳ�ƥ�䣬Ԥ��: %d, ʵ��: %d", calc_prolen(0), pkt[INDEX_PROLEN]);
        goto _free;
    }
    if (pkt[INDEX_CHECKSUM(pktsiz)] != calc_checksum(pkt, pktsiz)) {
        elog_e(TAG, "���յ��İ�У��Ͳ�ƥ�䣬Ԥ��: %d, ʵ��: %d", pkt[INDEX_CHECKSUM(pktsiz)], calc_checksum(pkt, pktsiz));
        goto _free;
    }
_free:
    scs_port_free(pkt); //! free
}

void scs_pos_set(uint8_t id, uint16_t pos)
{
    //! ָ���׼��
	const scs_size paramlen = 2;
    const scs_size pktsiz   = calc_pktsiz(paramlen);
    uint8_t* const pkt      = (uint8_t*)scs_port_malloc(pktsiz);//! alloc
    if (pkt == NULL) {
        elog_e(TAG, "�ڴ����ʧ��");
        return;
    }
    //! ������ݰ�
    pkt[INDEX_MAGIC1]     = MAGIC1;
    pkt[INDEX_MAGIC2]     = MAGIC2;
    pkt[INDEX_ID]         = id;
    pkt[INDEX_PROLEN]     = calc_prolen(paramlen);
    pkt[INDEX_CODE]       = CODE_WRITE_DATA; 
    pkt[INDEX_PARAM_ADDR] = ADDR_SRAM_CTRL_GOAL_POSITION;
    pkt[INDEX_PARAM(1)]   = (pos >> 8) & 0xFF; // ���ֽ�
    pkt[INDEX_PARAM(2)]   = pos & 0xFF;        // ���ֽ�
    pkt[INDEX_CHECKSUM(pktsiz)] = calc_checksum(pkt, pktsiz); // ����У���
    //! ������Ϣ
    elog_d(TAG, "��������λ�����ݰ�");
    elog_pkt(pkt, pktsiz, paramlen); // ��ӡ���ݰ�����
    //! �������ݰ�
    scs_port_send(pkt, pktsiz);
    scs_port_free(pkt); //! free
}

void scs_pos_async_set(uint8_t id, uint16_t pos)
{
    //! ָ���׼��
    const scs_size paramlen = 2;
    const scs_size pktsiz = calc_pktsiz(paramlen);
    uint8_t* const pkt = (uint8_t*)scs_port_malloc(pktsiz); //! alloc
    if (pkt == NULL) {
        elog_e(TAG, "�ڴ����ʧ��");
        return;
    }
    //! ������ݰ�
    pkt[INDEX_MAGIC1]     = MAGIC1;
    pkt[INDEX_MAGIC2]     = MAGIC2;
    pkt[INDEX_ID]         = id;
    pkt[INDEX_PROLEN]     = calc_prolen(paramlen);
    pkt[INDEX_CODE]       = CODE_REGWRITE_DATA;
    pkt[INDEX_PARAM_ADDR] = ADDR_SRAM_CTRL_GOAL_POSITION;
    pkt[INDEX_PARAM(1)]   = (pos >> 8) & 0xFF; // ���ֽ�
    pkt[INDEX_PARAM(2)]   = pos & 0xFF;        // ���ֽ�
    pkt[INDEX_CHECKSUM(pktsiz)] = calc_checksum(pkt, pktsiz);
    //! ������Ϣ
    elog_d(TAG, "�����첽����λ�����ݰ�");
    elog_pkt(pkt, pktsiz, paramlen); // ��ӡ���ݰ�����
    //! �������ݰ�
    scs_port_send(pkt, pktsiz);
    scs_port_free(pkt); //! free
}

void scs_pos_async_execute(uint8_t id)
{
    //! ָ���׼��
	const scs_size paramlen = 0; // �첽ִ�в���Ҫ����
    const scs_size pktsiz = calc_pktsiz(paramlen);
    uint8_t* const pkt = (uint8_t*)scs_port_malloc(pktsiz); //! alloc
    if (pkt == NULL) {
        elog_e(TAG, "�ڴ����ʧ��");
        return;
    }
    //! ������ݰ�
    pkt[INDEX_MAGIC1]     = MAGIC1;
    pkt[INDEX_MAGIC2]     = MAGIC2;
    pkt[INDEX_ID]         = id;
    pkt[INDEX_PROLEN]     = calc_prolen(paramlen);
    pkt[INDEX_CODE]       = CODE_ACTION; // �����첽ִ������
    pkt[INDEX_CHECKSUM(pktsiz)] = calc_checksum(pkt, pktsiz);
    //! ������Ϣ
    elog_d(TAG, "�����첽ִ�����ݰ�");
    elog_pkt(pkt, pktsiz, 0); // ��ӡ���ݰ�����
    //! �������ݰ�
    scs_port_send(pkt, pktsiz);
    scs_port_free(pkt); //! free
}