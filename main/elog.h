#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define elog_a(tag, fmt, ...) \
	do { \
		printf("[ASSERT] [%s] " fmt "\n", tag, ##__VA_ARGS__); \
	} while (0)

#define elog_e(tag, fmt, ...) \
	do { \
		printf("[ERROR] [%s] " fmt "\n", tag, ##__VA_ARGS__); \
	} while (0)

#define elog_w(tag, fmt, ...) \
	do { \
		printf("[WARN] [%s] " fmt "\n", tag, ##__VA_ARGS__); \
	} while (0)

#define elog_d(tag, fmt, ...) \
	do { \
		printf("[DEBUG] [%s] " fmt "\n", tag, ##__VA_ARGS__); \
	} while (0)

#define elog_i(tag, fmt, ...) \
	do { \
		printf("[INFO] [%s] " fmt "\n", tag, ##__VA_ARGS__); \
	} while (0)

#define elog_v(tag, fmt, ...) \
	do { \
		printf("[VERBOSE] [%s] " fmt "\n", tag, ##__VA_ARGS__); \
	} while (0)

#ifdef __cplusplus
}
#endif