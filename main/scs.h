#pragma once

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef uint8_t scs_size;

	//typedef void*	 (*scs_port_malloc_callback)(size_t size);
	//typedef void	 (*scs_free_callback)(void* ptr);
	//typedef void	 (*scs_scs_port_send_callback)(uint8_t* pkt, scs_size pktsiz);
	//typedef scs_size (*scs_recv_callback)(uint8_t* buf, scs_size bufsiz);
	//typedef void     (*scs_delay_callback)(uint32_t delay_ms);
	//typedef uint32_t (*scs_gettick_callback)(void);

	//void scs_callback_register(
	//	scs_port_malloc_callback malloc_cb,
	//	scs_free_callback free_cb,
	//	scs_scs_port_send_callback scs_port_send_cb, 
	//	scs_recv_callback recv_cb,
	//	scs_delay_callback delay_cb,
	//	scs_gettick_callback gettick_cb
	//);

	void scs_port_init(void);
	void* scs_port_malloc(size_t size);
	void scs_port_free(void* ptr);
	void scs_port_send(uint8_t* pkt, scs_size pktsiz);
	scs_size scs_port_recv(uint8_t* buf, scs_size bufsiz);
	void scs_port_delay(uint32_t delay_ms);
	uint32_t scs_port_gettick(void);

	void scs_ping(uint8_t id);
	void scs_set_pos(uint8_t id, uint16_t pos);

#ifdef __cplusplus
}
#endif
