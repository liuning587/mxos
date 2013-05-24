/**
 ******************************************************************************
 * @file       ring.h
 * @brief      ring buf模块.
 * @details    提供环形缓冲区服务
 * @copyright
 *
 ******************************************************************************
 */
#include <intLib.h>
#include <string.h>
#include <ring.h>

/**
 ******************************************************************************
 * @brief      建立环形缓冲区
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @param[in]  *buf     : 缓冲区起始地址
 * @param[in]   len     : 缓冲区长度.单位是字节数
 * @retval     None
 *
 * @details
 *  建立环形缓冲区并初始化，使用这个函数之前，用户应该定义缓冲区内存块和
 *  缓冲区数据结构。
 ******************************************************************************
 */
void
ring_init(struct ring_buf *ring, uint8_t *buf, uint16_t len)
{
    ring->buf = buf;
    ring->max_len = len;
    ring->offset_write = 0;
    ring->offset_read = 0;
    ring->len = 0;
}

/**
 ******************************************************************************
 * @brief     检查缓冲区容量
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @retval     缓冲区容量，就是调用ring_init时使用的len参数。
 *
 * @details
 * 返回缓冲区容量
 ******************************************************************************
 */
uint16_t
ring_capacity(struct ring_buf *ring)
{
    return ring->max_len;
}

/**
 ******************************************************************************
 * @brief      返回字节池地址
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @retval     环形缓冲区的字节池地址
 *
 * @details
 * 查询缓冲区的字节池地址，这个地址是用户调用ring_init时使用的缓冲区地址。
 * @note
 *      有两种情况会产生获得缓冲区地址的需求
 *      1、要释放缓冲区内存，应用程序可以使用调用ring_init时使用的指针，如果该
 *         指针丢失，可用本函数取回该指针。
 *      2、用户想自己动手访问该缓冲区，这不是一个明智的选择，有破坏模块独立性
 *         的嫌疑，这时候，使用者应该完全明白自己在干什么!
 ******************************************************************************
 */
uint8_t*
ring_get_buf(struct ring_buf *ring)
{
    return ring->buf;
}

/**
 ******************************************************************************
 * @brief      环形缓冲区写入
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @param[in]  *buffer  : 待写入的数据指针
 * @param[in]   len     : 待写入的数据长度.单位是字节数
 * @retval     实际写入的字节数,如果缓冲区有足够的空间,=len
 *
 * @details
 * 环形缓冲区写入若干个字节,返回实际写入的数据量,并移动写指针,如果环形
 * 缓冲区没有足够的空间,按实际剩余空间写入
 ******************************************************************************
 */
uint16_t
ring_write(struct ring_buf *ring, const uint8_t *buffer, uint16_t len)
{
    uint16_t wr_len;
    uint16_t partial;

    wr_len = ring->max_len - ring->len;
    if (wr_len == 0)
    {
        return 0;
    }
    if (wr_len > len)
    {
        wr_len = len;
    }
    if ((ring->offset_write + wr_len) > ring->max_len)
    { //数据发生环绕
        partial = ring->max_len - ring->offset_write;
        memcpy(&ring->buf[ring->offset_write], buffer, partial); //写第一部分
        memcpy(ring->buf, &buffer[partial], wr_len - partial); //写第二部分
        ring->offset_write = wr_len - partial;
    }
    else
    { //不发生环绕
        memcpy(&ring->buf[ring->offset_write], buffer, wr_len);
        ring->offset_write += wr_len;
    }

    intLock();
    ring->len += wr_len;
    intUnlock();

    return wr_len;
}

/**
 ******************************************************************************
 * @brief      环形缓冲区写入(强制覆盖最老的数据)
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @param[in]  *buffer  : 待写入的数据指针
 * @param[in]   len     : 待写入的数据长度.单位是字节数
 * @retval     实际写入的字节数,如果缓冲区有足够的空间,=len
 *
 * @details
 * 环形缓冲区写入若干个字节,返回实际写入的数据量,并移动写指针,如果环形
 * 缓冲区没有足够的空间,按实际剩余空间写入
 ******************************************************************************
 */
uint16_t
ring_write_force(struct ring_buf *ring, const uint8_t *buffer, uint16_t len)
{
    uint16_t wr_len = len;
    uint16_t partial;

    if (len > ring->max_len)
    {
        wr_len = len % ring->max_len;
        buffer += len - wr_len;
    }
    if ((ring->offset_write + wr_len) > ring->max_len)
    { //数据发生环绕
        partial = ring->max_len - ring->offset_write;
        memcpy(&ring->buf[ring->offset_write], buffer, partial); //写第一部分
        memcpy(ring->buf, &buffer[partial], wr_len - partial); //写第二部分
        ring->offset_write = wr_len - partial;
    }
    else
    { //不发生环绕
        memcpy(&ring->buf[ring->offset_write], buffer, wr_len);
        ring->offset_write += wr_len;
    }

    intLock();
    ring->len += wr_len;
    intUnlock();

    return wr_len;
}

/**
 ******************************************************************************
 * @brief      从环形缓冲区读
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @param[in]  *buffer  : 接收数据的缓冲区指针
 * @param[in]   len     : 待读出的数据长度.单位是字节数
 * @retval     实际读出的字节数,如果缓冲区有足够的数据,=len
 *
 * @details
 * 从环形缓冲区读出若干个字节,返回实际读出的数据量,并且移动读指针如果
 * 缓冲区内数据不足，按实际数量读取。
 ******************************************************************************
 */
uint16_t
ring_read(struct ring_buf *ring, uint8_t *buffer, uint16_t len)
{
    uint16_t wr_len;

    wr_len = (ring->len < len) ? ring->len : len;
    if ((ring->offset_read + wr_len) > ring->max_len)
    { //数据发生环绕
        uint16_t partial;
        partial = ring->max_len - ring->offset_read;
        memcpy(buffer, &ring->buf[ring->offset_read], partial); //写第一部分
        memcpy(&buffer[partial], ring->buf, wr_len - partial); //写第二部分
        ring->offset_read = wr_len - partial;
    }
    else
    { //不发生环绕
        memcpy(buffer, &ring->buf[ring->offset_read], wr_len);
        ring->offset_read += wr_len;
    }
    intLock();
    ring->len -= wr_len;
    intUnlock();

    return wr_len;
}

/**
 ******************************************************************************
 * @brief      检查缓冲区数据量
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @retval     缓冲区中的数据量
 *
 * @details    检查指定的环形缓冲区中的数据量,返回字节数.
 ******************************************************************************
 */
uint16_t
ring_check(struct ring_buf *ring)
{
    return ring->len;
}

/**
 ******************************************************************************
 * @brief      检查缓冲区是否空
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @retval     空则返回TRUE,非空返回FALSE
 *
 * @details     检查指定的环形缓冲区中是否已经空.
 ******************************************************************************
 */
bool_e
ring_if_empty(struct ring_buf *ring)
{
    return (ring->len == 0) ? TRUE : FALSE;
}

/**
 ******************************************************************************
 * @brief      检查缓冲区是否满
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @retval     满则返回TRUE,非满返回FALSE
 *
 * @details     检查指定的环形缓冲区中是否已经满.
 *
 * @note
 ******************************************************************************
 */
bool_e
ring_if_full(struct ring_buf *ring)
{
    return (ring->len == ring->max_len) ? TRUE : FALSE;
}

/**
 ******************************************************************************
 * @brief      清空环形缓冲区
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @retval     None
 *
 * @details     清除缓冲区中所有数据
 ******************************************************************************
 */
void
ring_flush(struct ring_buf *ring)
{
    ring->len = 0;
    ring->offset_write = 0;
    ring->offset_read = 0;
}

/**
 ******************************************************************************
 * @brief      释放若干数据
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @param[in]   len     : 释放的数据数量
 * @retval     实际释放的数据量
 *
 * @details 从读指针开始,释放掉指定大小的数据,相当于哑读了len个字节
 ******************************************************************************
 */
uint16_t
ring_dumb_read(struct ring_buf *ring, uint16_t len)
{
    uint16_t result;

    result = (ring->len < len) ? ring->len : len;
    if ((ring->offset_read + result) > ring->max_len)
    { //数据发生环绕
        ring->offset_read = result + ring->offset_read - ring->max_len;
    }
    else
    { //不发生环绕
        ring->offset_read += result;
    }
    ring->len -= result;
    return result;
}

/**
 ******************************************************************************
 * @brief      退回若干数据
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @param[in]   len     : 退回的数据数量
 * @retval     实际退回的数据量
 *
 * @details
 * 本函数与ring_dumb_read函数正好相反，把缓冲区指针退回len字节，如果退回的
 * 长度超过缓冲区的空闲长度，则取缓冲区空闲长度。相当于把缓冲区中已经读出
 * 的数据返回缓冲区，好像没有读过的样子。ring模块并不校验退回的部分是否包含
 * 原来的数据。
 ******************************************************************************
 */
uint16_t
ring_recede_read(struct ring_buf *ring, uint16_t len)
{
    uint16_t result;

    if ((ring->max_len - ring->len) > len) //空闲长度大于欲退回的长度
    {
        result = len;
    }
    else
    {
        result = ring->max_len - ring->len;
    }

    if (ring->offset_read < result)
    { //数据发生环绕
        ring->offset_read = ring->max_len - (result - ring->offset_read);
    }
    else
    { //不发生环绕
        ring->offset_read -= result;
    }
    ring->len += result;

    return result;
}

/**
 ******************************************************************************
 * @brief      取消若干数据
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @param[in]   len     : 退回的数据数量
 * @retval     实际退回的数据量
 *
 * @details  取消已经写入线性缓冲区的若干数据，就像从来没有写入一样。
 ******************************************************************************
 */
uint16_t
ring_skip_tail(struct ring_buf *ring, uint16_t size)
{
    uint16_t result;

    if (ring->len > size)
    {
        result = size;
        if (ring->offset_write < result)
        { //数据发生环绕
            ring->offset_write = ring->max_len - (result - ring->offset_write);
        }
        else
        { //不发生环绕
            ring->offset_read -= result;
        }
        ring->len -= result;
    }
    else
    {
        result = ring->len;
        ring_flush(ring);
    }

    return result;
}

/**
 ******************************************************************************
 * @brief      查找字符
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @param[in]   c       : 需查找的字符
 * @retval     c出现的位置,如果没有出现则返回 CN_LIMIT_UINT16
 *
 * @details    从ring当前读位置开始查找字符c的位置
 ******************************************************************************
 */
uint16_t
ring_search_ch(struct ring_buf *ring, char_t c)
{
    uint16_t i;
    uint8_t *buf = ring->buf;

    if (ring->offset_read > ring->offset_write)
    { //缓冲区有回绕
        for (i = ring->offset_read; i < ring->max_len; i++)
        {
            if (buf[i] == c)
            {
                return (i - ring->offset_read);
            }
        }
        for (i = 0; i < ring->offset_write; i++)
        {
            if (buf[i] == c)
            {
                return (i - ring->offset_read + ring->max_len);
            }
        }
    }
    else
    { //缓冲区没有回绕
        for (i = ring->offset_read; i < ring->offset_write; i++)
        {
            if (buf[i] == c)
            {
                return (i - ring->offset_read);
            }
        }
    }
    return CN_LIMIT_UINT16;
}

/**
 ******************************************************************************
 * @brief      查找字符序列
 * @param[in]  *ring    : 目标环形缓冲区结构指针
 * @param[in]  *string  : 需查找的字符序列
 * @param[in]   str_len : 字符序列长度
 * @retval     string出现的位置相对offset_read的偏移量,
 *             如果没有出现返回 CN_LIMIT_UINT16
 *
 * @details  从ring当前读位置开始查找字符序列的位置,字符序列不以0结束,而是指定长度
 *
 * @note 这个功能可能比较常用,所以在编写时注意了速度优化,但却使代码量大增.
 ******************************************************************************
 */
uint16_t
ring_search_str(struct ring_buf *ring, char_t *string, uint16_t str_len)
{
    uint16_t i, j;
    bool_e next;
    uint8_t *buf;
    uint16_t end, start;

    if (ring->len < str_len)
    {
        return CN_LIMIT_UINT16;
    }
    buf = ring->buf;
    if (ring->offset_read <= ring->offset_write)
    { //缓冲区没有回绕
        for (i = ring->offset_read; i <= ring->offset_write - str_len; i++)
        {
            next = FALSE;
            for (j = 0; j < str_len; j++)
            {
                if (buf[i + j] != string[j])
                {
                    next = TRUE;
                }
            }
            if (next == FALSE)
            {
                return (i - ring->offset_read);
            }
        }
    }
    else
    { //缓冲区有回绕
      //先处理不环绕部分,不包括缓冲区首末结合部分
        for (i = ring->offset_read; i <= (ring->max_len - str_len); i++)
        {
            next = FALSE;
            for (j = 0; j < str_len; j++)
            {
                if (buf[i + j] != string[j])
                {
                    next = TRUE; //如果发现不等,则表示要进行下一个循环
                }
            }
            if (next == FALSE)
            { //表示上一个循环中已经发现匹配的串,
                return (i - ring->offset_read);
            }
        }
        //再处理缓冲区首末结合部分以及回绕部分
        if (ring->offset_write >= str_len)
        { //缓冲区中回绕部分的长度超过查找长度,末端和首端分别查找
          //先查找缓冲区末端部分
            for (; i < ring->max_len; i++)
            {
                next = FALSE;
                //string分成两部分,end个字符在缓冲区末端,start个字符在缓冲区首
                end = ring->max_len - i;
                start = str_len - end;
                for (j = 0; j < end; j++)
                { //先比较缓冲区末端部分
                    if (buf[i + j] != string[j])
                    {
                        next = TRUE;
                    }
                }
                if (next == FALSE)
                { //缓冲区末端部分全部匹配,需要看首端是否也匹配
                    for (j = 0; j < start; j++)
                    {
                        if (buf[j] != string[start + j])
                        {
                            next = TRUE;
                        }
                    }
                }
                if (next == FALSE)
                { //表示上一个循环中已经发现匹配的串,
                    return (i - ring->offset_read);
                }
            }
            //再查找首端部分
            for (i = 0; i < (ring->offset_write - str_len); i++)
            {
                next = FALSE;
                for (j = 0; j < str_len; j++)
                {
                    if (buf[i + j] != string[j])
                    {
                        next = TRUE; //如果发现不等,则表示要进行下一个循环
                    }
                }
                if (next == FALSE)
                { //表示上一个循环中已经发现匹配的串,
                    return (i - ring->offset_read);
                }
            }
        }
        else
        { //缓冲区中回绕部分长度没有超过str_len,只需要查找末端即可.
            next = FALSE;
            for (; i < (ring->offset_read + ring->len - str_len); i++)
            {
                uint16_t end, start;
                next = FALSE;
                //string分成两部分,end个字符在缓冲区末端,start个字符在缓冲区首
                end = ring->max_len - i;
                start = str_len - end;
                for (j = 0; j < end; j++)
                { //先比较缓冲区末端部分
                    if (buf[i + j] != string[j])
                    {
                        next = TRUE;
                    }
                }
                if (next == FALSE)
                { //缓冲区末端部分全部匹配,需要看首端是否也匹配
                    for (j = 0; j < start; j++)
                    {
                        if (buf[j] != string[start + j])
                        {
                            next = TRUE;
                        }
                    }
                }
                if (next == FALSE)
                { //表示上一个循环中已经发现匹配的串,
                    return (i - ring->offset_read);
                }
            }
        }
    }
    return CN_LIMIT_UINT16;
}
