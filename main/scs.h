#pragma once

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef uint8_t scs_size;

	void scs_port_init(void);
	void* scs_port_malloc(size_t size);
	void scs_port_free(void* ptr);
	void scs_port_send(uint8_t* pkt, scs_size pktsiz);
	scs_size scs_port_recv(uint8_t* buf, scs_size bufsiz);
	void scs_port_delay(uint32_t delay_ms);
	uint32_t scs_port_gettick(void);

	void scs_ping(uint8_t id);

	void scs_pos_set(uint8_t id, uint16_t pos);
	void scs_pos_async_set(uint8_t id, uint16_t pos);

#ifdef __cplusplus
}
#endif
