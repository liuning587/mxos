/**
 ******************************************************************************
 * @file       ring.h
 * @brief      ring buf模块.
 * @details    提供环形缓冲区服务, Modify from djyos.
 * @copyright
 *
 ******************************************************************************
 */
#ifndef __RING_H__
#define __RING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

#define CN_LIMIT_UINT16     0xffff

struct ring_buf
{
    uint16_t    len;            /**< 缓冲区中的字节数 */
    uint16_t    offset_read;    /**< 缓冲区读指针,指向下一次读的位置 */
    uint16_t    offset_write;   /**< 缓冲区写指针,指向下一次写的位置 */
    uint16_t    max_len;        /**< 缓冲区最大长度,元素个数. */
    uint8_t     *buf;           /**< 缓冲区指针 */
};

extern void     ring_init(struct ring_buf *ring, uint8_t *buf, uint16_t len);
extern uint16_t ring_capacity(struct ring_buf *ring);
extern uint8_t *ring_get_buf(struct ring_buf *ring);
extern uint16_t ring_write(struct ring_buf *ring,const uint8_t *buffer, uint16_t len);
extern uint16_t ring_write_force(struct ring_buf *ring, const uint8_t *buffer, uint16_t len);
extern uint16_t ring_read(struct ring_buf *ring,uint8_t *buffer, uint16_t len);
extern uint16_t ring_check(struct ring_buf *ring);
extern bool_e   ring_if_empty(struct ring_buf *ring);
extern bool_e   ring_if_full(struct ring_buf *ring);
extern void     ring_flush(struct ring_buf *ring);
extern uint16_t ring_dumb_read(struct ring_buf *ring, uint16_t len);
extern uint16_t ring_recede_read(struct ring_buf *ring, uint16_t len);
extern uint16_t ring_skip_tail(struct ring_buf *ring, uint16_t size);
extern uint16_t ring_search_ch(struct ring_buf *ring, char_t c);
extern uint16_t ring_search_str(struct ring_buf *ring, char_t *string, uint16_t str_len);

#ifdef __cplusplus
}
#endif

#endif /* __RING_H__ */
