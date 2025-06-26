#pragma once

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void*	 (*scs_malloc_callback)(size_t size);
typedef void	 (*scs_free_callback)(void* ptr);
typedef void	 (*scs_send_callback)(uint8_t* pkt, uint32_t pktsiz);
typedef uint32_t (*scs_recv_callback)(uint8_t* buf, uint32_t bufsiz);
typedef void     (*scs_delay_callback)(uint32_t delay_ms);
typedef uint32_t (*scs_gettick_callback)(void);

void scs_callback_register(
	scs_malloc_callback malloc_cb,
	scs_free_callback free_cb,
	scs_send_callback send_cb, 
	scs_recv_callback recv_cb,
	scs_delay_callback delay_cb,
	scs_gettick_callback gettick_cb
);

void scs_ping(uint8_t id);

#ifdef __cplusplus
}
#endif
