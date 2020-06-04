//
// Created by msi on 2020/6/2.
//

#ifndef PREDICTOR_HISTORY_H
#define PREDICTOR_HISTORY_H

#include "utils.h"

struct History {
    uint32_t *bits;
    uint32_t high_mark;
    uint32_t length;
    int num;
};

struct History * NewHistory(int length) {
    struct History * history = (struct History *)malloc(sizeof(struct History));
    history->length = length;
    history->num = length / 32 + (length % 32) > 0 ? 1 : 0;
    history->bits = (uint32_t *)malloc(sizeof(uint32_t) * history->num);
    history->high_mark = 1u << 31u;
    return history;
}

void add_history(struct History * history, uint8_t bit) {
    for (int i = 0; i < history->num - 1; ++i) {
        history->bits[i] <<= 1u;
        history->bits[i] |= history->bits[i+1] & history->high_mark;
    }
    history->bits[history->num - 1] <<= 1u;
    history->bits[history->num - 1] |= bit & 1u;
    history->bits[history->num - 1] = get_part(history->bits[history->num - 1], history->length % 32 - 1, 0);
}

uint32_t get_part_history(struct History * history, uint32_t start, uint32_t end) {
    int start_num = start / 32;
    int end_num = end / 32;
    uint32_t start_part = start % 32;
    uint32_t end_part = end % 32;

    if (start_num == end_num) {
        return get_part(history->bits[start_num], start_part, end_part);
    }

    uint32_t result = 0;
    result |= get_part(history->bits[end_num], 31, end_part);
    result |= get_part(history->bits[start_num], start_part, 0) << (start - end - 31 - end_part);
    return result;
}

uint32_t fold_history(struct History * history, uint32_t limit) {
    uint32_t start = limit - 1;
    uint32_t end = 0;
    uint32_t result = 0;

    while (end <= history->length) {
        result ^= get_part_history(history, start, end);
        start += limit;
        end += limit;
        if (start >= history->length) {
            start = history->length - 1;
        }
    }

    return result;
}



#endif //PREDICTOR_HISTORY_H
