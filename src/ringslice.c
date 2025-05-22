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
#include "ringslice.h"

DBC_MODULE_NAME(RINGSLICE_MODULE)

/*!
 * Increments pointer with wrapping around
 * @param[in] curr pointer to increment
 * @param[in] absolute_offset offset to increment
 * @param[in] start start pointer of wrapping
 * @param[in] end end pointer of wrapping
 *
 * @return pointer between start (including) and end (not including)
 */
static inline uint8_t *ringslice_ptr_increment_wrap_around(uint8_t const *curr, ringslice_cnt_t absolute_offset, uint8_t const *const start, uint8_t const *const end) {
    DBC_REQUIRE(666, start <= curr && curr < end);
    uint8_t *next_unwrapped = (uint8_t *)&(curr[absolute_offset]);
    uint8_t *ret = (next_unwrapped < end) ? (next_unwrapped) : (uint8_t *)(&(start[next_unwrapped - end]));
    DBC_ENSURE(999, start <= ret && ret < end);
    return ret;
}

/*!
 * Decrements pointer with wrapping around
 * @param[in] curr pointer to decrement
 * @param[in] absolute_offset offset to decrement
 * @param[in] start start pointer of wrapping
 * @param[in] end end pointer of wrapping
 *
 * @return pointer between start (including) and end (not including)
 */
static inline uint8_t *ringslice_ptr_decrement_wrap_around(uint8_t const *curr, ringslice_cnt_t absolute_offset, uint8_t const *const start, uint8_t const *const end) {
    DBC_REQUIRE(667, start <= curr && curr < end);
    uint8_t *prev_unwrapped = (uint8_t *)&(curr[-absolute_offset]);
    uint8_t *ret = (prev_unwrapped >= start) ? (prev_unwrapped) : (uint8_t *)(&(end[prev_unwrapped - start]));
    DBC_ENSURE(998, start <= ret && ret < end);
    return ret;
}

ringslice_t ringslice_strstr(ringslice_t const *const me, char const *substr) {
    ringslice_t substr_slice = ringslice_initializer(me->buf, me->buf_size, me->first, me->first);  // initialize with empty slice
    uint8_t const *const buf_end = &(me->buf[me->buf_size]);
    uint8_t const *const buf_start = &(me->buf[0]);
    uint8_t const *first_ptr = &(me->buf[me->first]);
    uint8_t const *last_ptr = &(me->buf[me->last]);
    bool found = false;
    ringslice_cnt_t cmp_pos = 0;
    while (first_ptr != last_ptr) {
        char cmp = substr[cmp_pos];

        if (cmp == '\0') {
            found = true;
            break;
        }

        if (*first_ptr == cmp) {
            cmp_pos++;
        } else {
            first_ptr = ringslice_ptr_decrement_wrap_around(first_ptr, cmp_pos, buf_start, buf_end);
            cmp_pos = 0;
        }

        first_ptr = ringslice_ptr_increment_wrap_around(first_ptr, 1, buf_start, buf_end);
    }

    if ((first_ptr == last_ptr) && (substr[cmp_pos] == '\0')) {
        found = true;
    }

    if (found) {
        ringslice_cnt_t suffix_idx_last = (ringslice_cnt_t)(first_ptr - buf_start);
        ringslice_cnt_t suffix_idx_first = (suffix_idx_last >= cmp_pos) ? (suffix_idx_last - cmp_pos) : (me->buf_size + suffix_idx_last - cmp_pos);

        DBC_ENSURE(901, 0 <= suffix_idx_first && suffix_idx_first < substr_slice.buf_size);
        DBC_ENSURE(902, 0 <= suffix_idx_last && suffix_idx_last < substr_slice.buf_size);
        substr_slice.first = suffix_idx_first;
        substr_slice.last = suffix_idx_last;
    }

    return substr_slice;
}

int ringslice_strcmp(ringslice_t const *const me, char const *str) {
    uint8_t const *first_ptr = &(me->buf[me->first]);
    uint8_t const *last_ptr = &(me->buf[me->last]);
    uint8_t const *const buf_end = &(me->buf[me->buf_size]);
    uint8_t const *const buf_start = &(me->buf[0]);
    uint8_t const *chr = (uint8_t const *)str;

    while (first_ptr != last_ptr) {
        int diff = (int)*first_ptr - (int)*chr;
        if (diff) {
            return diff;
        }

        chr++;
        first_ptr = ringslice_ptr_increment_wrap_around(first_ptr, 1, buf_start, buf_end);
    }

    return -(int)*chr;
}

ringslice_t ringslice_subslice_with_suffix(ringslice_t const *const me, ringslice_cnt_t from_idx, char const *suffix) {
    ringslice_cnt_t const rs_len = ringslice_len(me);
    DBC_ASSERT(204, from_idx <= ringslice_len(me));

    ringslice_t resp_slice = ringslice_initializer(me->buf, me->buf_size, me->first, me->first);  // initialize with empty slice
    ringslice_t search_slice = ringslice_subslice(me, from_idx, rs_len);
    ringslice_t suffix_slice = ringslice_strstr(&search_slice, suffix);

    if (ringslice_is_empty(&suffix_slice)) {
        // resp_slice already initialized with empty slice
    } else {
        resp_slice.last = suffix_slice.last;
    }

    return resp_slice;
}