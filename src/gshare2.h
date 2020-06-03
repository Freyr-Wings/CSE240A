//
// Created by msi on 2020/6/3.
//

#ifndef PREDICTOR_GSHARE2_H
#define PREDICTOR_GSHARE2_H

#include "predictor.h"
#include "utils.h"
#include "history.h"
#include <string.h>


struct GShare2 {
    int num_history_bit;
    int num_predict_bit;
    int num_key_bit;
    struct History *history;
    uint16_t *predict_table;
};

struct GShare2 * NewGShare2(int num_history_bit, int num_key_bit, int num_predict_bit) {
    struct GShare2 *predictor = (struct GShare2 *)malloc(sizeof(struct GShare2));
    predictor->num_history_bit = num_history_bit;
    predictor->num_key_bit = num_key_bit;
    predictor->num_predict_bit = num_predict_bit;
    predictor->history = NewHistory(num_history_bit);
    predictor->predict_table = (uint16_t *)malloc(sizeof(uint16_t)*(1 << num_key_bit));
    memset(predictor->predict_table, 0, sizeof(uint16_t)*(1 << num_key_bit));
    return predictor;
}

uint8_t gshare2_predict(struct GShare2 *predictor, uint32_t pc) {
    int key = get_part(
            fold_history(predictor->history, predictor->num_key_bit) ^ pc ^ (pc >> predictor->num_key_bit),
            predictor->num_key_bit-1, 0);
    int tag = pc ^ fold_history(predictor->history, predictor->num_key_bit) ^ (fold_history(predictor->history, predictor->num_key_bit) << 1u);
    uint8_t prediction = get_bit(predictor->predict_table[key], predictor->num_predict_bit);
    return prediction;
}

void gshare2_train(struct GShare2 *predictor, uint32_t pc, uint8_t outcome) {
    int key = get_part(
            fold_history(predictor->history, predictor->num_key_bit) ^ pc ^ (pc >> predictor->num_key_bit),
            predictor->num_key_bit-1, 0);
    uint8_t prediction = predictor->predict_table[key];
    if (outcome == TAKEN) {
        if (prediction <  ((1u << (uint8_t)predictor->num_predict_bit) - 1u)) {
            prediction ++;
        }
    } else {
        if (prediction > 0) {
            prediction --;
        }
    }
    predictor->predict_table[key] = prediction;
    add_history(predictor->history, outcome);
}

#endif //PREDICTOR_GSHARE2_H
