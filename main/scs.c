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
	//1.�����ڴ�ص�
	if(malloc_cb != NULL && free_cb != NULL) {
		scs_malloc = malloc_cb;
		scs_free = free_cb;
	} else {
		scs_malloc = malloc;
		scs_free = free;
		elog_w(TAG, "ʹ�ñ�׼���ڴ����");
	}
	//2.�������ݴ���ص�
	if(send_cb == NULL || recv_cb == NULL) {
		elog_e(TAG, "δע�����ݴ���ص�����");
	} else {
		send = send_cb;
		recv = recv_cb;
	}
	//3.������ʱ�ص�
	if (delay_cb != NULL) {
		delay = delay_cb;
	} else {
		elog_w(TAG, "δע����ʱ�ص�����, ������������ʱ");
	}
	//4.����ʱ����ص�
	if( gettick_cb != NULL) {
		gettick = gettick_cb;
	} else {
		elog_w(TAG, "δע��ʱ����ص�����������������ʱ����ж�");
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
	for (uint32_t i = IDX_ID; i < IDX_CHECKSUM(pktsiz); i++) // ���������һ���ֽڵ�У���
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
void elog_pkt(uint8_t* pkt, uint32_t pktsiz, uint32_t paramlen)
{
#define PRINT_PKT_FMT "%20s : %05d"
	elog_d(TAG, PRINT_PKT_FMT, "Packet Size", pktsiz);//��ӡ���ݰ�����

	elog_d(TAG, PRINT_PKT_FMT, "Magic1", pkt[IDX_MAGIC1]);//��ӡħ���ֽ�1
	elog_d(TAG, PRINT_PKT_FMT, "Magic2", pkt[IDX_MAGIC2]);//��ӡħ���ֽ�
	elog_d(TAG, PRINT_PKT_FMT, "ID", pkt[IDX_ID]);//��ӡID
	elog_d(TAG, PRINT_PKT_FMT, "Protocol Length", pkt[IDX_PROLEN]);//��ӡЭ�鳤��
	elog_d(TAG, PRINT_PKT_FMT, "Code", pkt[IDX_CODE]);//��ӡ������
	if (paramlen > 0) {
		for(int i = 0; i < paramlen; i++) {
			elog_d(TAG, PRINT_PKT_FMT, "Param", pkt[IDX_PARAM(i)]);//��ӡ����
		}
	}
	elog_d(TAG, PRINT_PKT_FMT, "Checksum", pkt[IDX_CHECKSUM(pktsiz)]);//
}

void scs_ping(uint8_t id)
{
	uint32_t pktsiz = calc_pktsiz(0); // 
	uint8_t* pkt = (uint8_t*)scs_malloc(pktsiz);
	if (pkt == NULL) {
		elog_e(TAG, "�ڴ����ʧ��");
		return;
	}
	//������ݰ�
	pkt[IDX_MAGIC1] = MAGIC1;
	pkt[IDX_MAGIC2] = MAGIC2;
	pkt[IDX_ID] = id;
	pkt[IDX_PROLEN] = calc_prolen(0); // code(1) + param(0) + checksum(1)
	pkt[IDX_CODE] = CODE_PING; // ����Ping������
	//û�в��������Բ���Ҫ������
	pkt[IDX_CHECKSUM(pktsiz)] = calc_checksum(pkt, pktsiz); // ����У���
	elog_d(TAG, "����Ping���ݰ�");
	elog_pkt(pkt, pktsiz, 0); // ��ӡ���ݰ�����
	//�������ݰ�
	if (send != NULL) {
		send(pkt, pktsiz);
	} else {
		elog_e(TAG, "δע�ᷢ�ͻص�����");
	}
	//׼��������Ӧ���ݰ�
	memset(pkt, 0, pktsiz); // ������ݰ�����
	recv(pkt, pktsiz); // �������ݰ�
	//��ӡ���յ������ݰ�
	elog_d(TAG, "���յ�Ping��Ӧ���ݰ�");
	elog_pkt(pkt, pktsiz, 0);
	//������ݰ�
	if (pkt[IDX_MAGIC1] != MAGIC1 || pkt[IDX_MAGIC2] != MAGIC2) {
		elog_e(TAG, "���յ��İ�������Ч��SCS���ݰ�");
		goto _free;
	}
	if (pkt[IDX_ID] != id) {
		elog_e(TAG, "���յ��İ�ID��ƥ�䣬Ԥ��: %d, ʵ��: %d", id, pkt[IDX_ID]);
		goto _free;
	}
	if (pkt[IDX_CODE] != CODE_PING) {
		elog_e(TAG, "���յ��İ������벻ƥ�䣬Ԥ��: %d, ʵ��: %d", CODE_PING, pkt[IDX_CODE]);
		goto _free;
	}
	if (pkt[IDX_PROLEN] != calc_prolen(0)) {
		elog_e(TAG, "���յ��İ�Э�鳤�Ȳ�ƥ�䣬Ԥ��: %d, ʵ��: %d", calc_prolen(0), pkt[IDX_PROLEN]);
		goto _free;
	}
	if (pkt[IDX_CHECKSUM(pktsiz)] != calc_checksum(pkt, pktsiz)) {
		elog_e(TAG, "���յ��İ�У��Ͳ�ƥ�䣬Ԥ��: %d, ʵ��: %d", pkt[IDX_CHECKSUM(pktsiz)], calc_checksum(pkt, pktsiz));
		goto _free;
	}
_free:
	scs_free(pkt); // �ͷ����ݰ��ڴ�
}