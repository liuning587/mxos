/**
 ******************************************************************************
 * @file       ring.h
 * @brief      ring bufģ��.
 * @details    �ṩ���λ���������, Modify from djyos.
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

#pragma pack(push, 1)

struct ring_buf
{
    uint16_t    len;            /**< �������е��ֽ��� */
    uint16_t    offset_read;    /**< ��������ָ��,ָ����һ�ζ���λ�� */
    uint16_t    offset_write;   /**< ������дָ��,ָ����һ��д��λ�� */
    uint16_t    max_len;        /**< ��������󳤶�,Ԫ�ظ���. */
    uint8_t     *buf;           /**< ������ָ�� */
};

#pragma pack(pop)

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
