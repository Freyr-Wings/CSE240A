//
// Created by msi on 2020/5/30.
//

#ifndef PREDICTOR_GSHARE_H
#define PREDICTOR_GSHARE_H

#include "predictor.h"
#include "utils.h"
#include "history.h"
#include <string.h>


struct GShare {
    int num_history_bit;
    int num_predict_bit;
    int num_key_bit;
    struct History *history;
    uint8_t *predict_table;
};

struct GShare * NewGShare(int num_history_bit, int num_key_bit, int num_predict_bit) {
    struct GShare *predictor = (struct GShare *)malloc(sizeof(struct GShare));
    predictor->num_history_bit = num_history_bit;
    predictor->num_key_bit = num_key_bit;
    predictor->num_predict_bit = num_predict_bit;
    predictor->history = NewHistory(num_history_bit);
    predictor->predict_table = (uint8_t *)malloc(sizeof(uint8_t)*(1 << num_key_bit));
    int num_entry = 1 << num_key_bit;
    for (int i = 0; i < num_entry; ++i) {
        predictor->predict_table[i] = 1u;
    }
//    memset(predictor->predict_table, 0, sizeof(uint8_t)*(1 << num_key_bit));
    return predictor;
}

uint8_t gshare_predict(struct GShare *predictor, uint32_t pc) {
    int key = get_part(fold_history(predictor->history, predictor->num_key_bit) ^ pc, predictor->num_key_bit-1, 0);
    uint8_t prediction = get_bit(predictor->predict_table[key], predictor->num_predict_bit);
    return prediction;
}

void gshare_train(struct GShare *predictor, uint32_t pc, uint8_t outcome) {
    int key = get_part(fold_history(predictor->history, predictor->num_key_bit) ^ pc, predictor->num_key_bit-1, 0);
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



#endif //PREDICTOR_GSHARE_H
