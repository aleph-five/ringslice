/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Nikita Maltsev (aleph-five)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _RINGSLICE_H_
#define _RINGSLICE_H_

#ifdef __cplusplus
extern "C" {
#endif
/*! @file
* @brief Slices of ring buffer with string-like methods
*
* @note
* Useful for embedded applications
*/


#include <stdint.h>
#include <stdbool.h>
#include "dbc_assert.h"
#include "ringslice_config.h"

/// ringslice module name for DBC assertions
#define RINGSLICE_MODULE                                                "ringslice"

/// type for counter
typedef int32_t ringslice_cnt_t;

/// ringslice structure
typedef struct
{
    uint8_t *buf;                       ///< Pointer to zeroth element of ring buffer array
    ringslice_cnt_t buf_size;           ///< size of array
    ringslice_cnt_t first;               ///< first (index of the first element that will be processed)
    ringslice_cnt_t last;               ///< last (index of empty place after last element)
}
ringslice_t;

/*!
* Initializer for ring slice.
* @param[in] buf pointer to zeroth element of ring buffer
* @param[in] buf_size size of buffer
* @param[in] first index of first element
* @param[in] last index of empty place after last element
*
* @return ring slice instance
*
*/
RINGSLICE_INLINE ringslice_t ringslice_initializer(uint8_t *buf, ringslice_cnt_t buf_size, ringslice_cnt_t first, ringslice_cnt_t last) {
    DBC_MODULE_REQUIRE(RINGSLICE_MODULE, 1, buf);
    DBC_MODULE_REQUIRE(RINGSLICE_MODULE, 2, buf_size > 0);
    DBC_MODULE_REQUIRE(RINGSLICE_MODULE, 3, (0 <= first && first < buf_size));
    DBC_MODULE_REQUIRE(RINGSLICE_MODULE, 4, (0 <= last && last < buf_size));
    ringslice_t rs = {
        .buf = buf,
        .buf_size = buf_size,
        .first = first,
        .last = last,
    };
    return rs;
}

/*!
* Calculates the length of subslice
* @param[in] me ringslice instance
*
* @return number of bytes stored in ringslice
*
*/
RINGSLICE_INLINE ringslice_cnt_t ringslice_len(ringslice_t const * const me) {
    return (me->buf_size + me->last - me->first) % me->buf_size;
}

/*!
* Checks whether the ringslice is empty
* @param[in] me ringslice instance
*
* @return true if empty, otherwise false
*
*/
RINGSLICE_INLINE bool ringslice_is_empty(ringslice_t const * const me) {
    return (me->first == me->last);
}

/*!
* Byte at relative index n
* @param[in] me pointer to ring slice instance
* @param[in] n relative index 
*
* @return byte at position n from first index of ring slice
*
* @note n must be greater than 0 and less than length of ring slice
*
*/
RINGSLICE_INLINE uint8_t ringslice_nth_byte(ringslice_t const * const me, ringslice_cnt_t n)
{
    DBC_MODULE_REQUIRE(RINGSLICE_MODULE, 104, n < ringslice_len(me));
    ringslice_cnt_t idx = (me->first + n) % me->buf_size;
    return me->buf[idx];
}

/*!
* Subslice of ring slice.
* @param[in] me pointer to ring slice instance that is copied
* @param[in] first relative index of the first element. For example,
*    if first equals to 0, then first element of returned slice will be the first of me slice. 
* @param[in] last relative index of the last element, analogous to first
*
* @return ring slice instance
*
*/
RINGSLICE_INLINE ringslice_t ringslice_subslice(ringslice_t const * const me, ringslice_cnt_t rel_first, ringslice_cnt_t rel_last) {
    DBC_MODULE_REQUIRE(RINGSLICE_MODULE, 101, rel_first <= rel_last);
    DBC_MODULE_REQUIRE(RINGSLICE_MODULE, 102, rel_first < ringslice_len(me));
    DBC_MODULE_REQUIRE(RINGSLICE_MODULE, 103, rel_last <= ringslice_len(me));
    ringslice_t rs;
    rs.buf = me->buf;
    rs.buf_size = me->buf_size;
    rs.first = (me->first + rel_first) % me->buf_size;
    rs.last = (me->first + rel_last) % me->buf_size;
    return rs;
}

/*!
* Compares ringslice instance with string lexicographically
* @param[in] me ringslice instance which is compared with string
* @param[in] str string for compare
*
* @return 0 if are equal,
*   negative value if ringslice appears before str in lexicographical order,
*   positive value if ringslice appears after str in lexicographical order
*
*/
int ringslice_strcmp(ringslice_t const * const me, char const * str);

/*!
* Searches for substring in ringslice instance
* @param[in] me ringslice instance where substring is searched for
* @param[in] substr searched substring
*
* @return subslice of me slice containing substring, otherwise empty ringslice
*
* @note if substr is empty string, then copy of me slice will be returned
*
*/
ringslice_t ringslice_strstr(ringslice_t const * const me, char const * substr);

/*!
* Searches for subslice with suffix in ringslice instance
* @param[in] me ringslice instance where suffix is searched for
* @param[in] from_idx start index for searching; if 0, then search from beginning of the slice
* @param[in] suffix suffix that is searched for
*
* @return subslice of me slice with suffix, otherwise empty ringslice
*
* @note if suffix is empty string, then empty slice will be returned
*
*/
ringslice_t ringslice_subslice_with_suffix(ringslice_t const * const me, ringslice_cnt_t from_idx, char const * suffix);

#ifdef __cplusplus
}
#endif

#endif // _RINGSLICE_H_