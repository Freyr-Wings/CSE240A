//
// Created by msi on 2020/5/30.
//

#ifndef PREDICTOR_GSHARE_H
#define PREDICTOR_GSHARE_H

#include "predictor.h"
#include "utils.h"
#include <string.h>


struct GShare {
    int num_history_bit;
    int num_predict_bit;
    uint32_t history;
    uint8_t *predict_table;
};

struct GShare * NewGShare(int num_history_bit, int num_predict_bit) {
    struct GShare *predictor = (struct GShare *)malloc(sizeof(struct GShare));
    predictor->num_history_bit = num_history_bit;
    predictor->num_predict_bit = num_predict_bit;
    predictor->history = 0;
    predictor->predict_table = (uint8_t *)malloc(sizeof(uint8_t)*(1 << num_history_bit));
    memset(predictor->predict_table, 0, sizeof(uint8_t)*(1 << num_history_bit));
    return predictor;
}

uint8_t predict(struct GShare *predictor, uint32_t pc) {
    int key = get_part(predictor->history ^ pc, predictor->num_history_bit-1, 0);
    uint8_t prediction = get_bit(predictor->predict_table[key], predictor->num_predict_bit);
    return prediction;
}

void train(struct GShare *predictor, uint32_t pc, uint8_t outcome) {
    int key = get_part(predictor->history ^ pc, predictor->num_history_bit-1, 0);
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
    predictor->history = get_part((predictor->history << 1u) + outcome, predictor->num_history_bit-1, 0);
}



#endif //PREDICTOR_GSHARE_H
