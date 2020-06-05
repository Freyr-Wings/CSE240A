//
// Created by wk on 6/1/20.
//

#ifndef PREDICTOR_PERCEPTRON_H
#define PREDICTOR_PERCEPTRON_H

#include "predictor.h"
#include "utils.h"
#include <string.h>
#define w_size int8_t


struct Perceptron {
    int ghistoryBits;
    int pcIndexBits;
    int16_t threshold;
    uint32_t globalHistory;

    w_size **weights;
    w_size *bias;
};

struct Perceptron * new_perceptron() {
    struct Perceptron *predictor = (struct Perceptron *)malloc(sizeof(struct Perceptron));
    predictor->ghistoryBits = 26;
    predictor->pcIndexBits = 8;
    predictor->threshold = 10;
    predictor->globalHistory = 0;

    predictor->weights = (w_size **)malloc(sizeof(w_size *) * (1 << predictor->pcIndexBits));
    for (int i = 0; i < (1 << predictor->pcIndexBits); ++i) {
        predictor->weights[i] = (w_size *) malloc(sizeof(w_size) * predictor->ghistoryBits);
        memset(predictor->weights[i], 0, sizeof(w_size) * predictor->ghistoryBits);
    }

    predictor->bias = (w_size *)malloc(sizeof(w_size) * (1 << predictor->pcIndexBits));
    memset(predictor->bias, 0, sizeof(w_size) * (1 << predictor->pcIndexBits));

//    int size = (1 << predictor->pcIndexBits) * (predictor->ghistoryBits + 1) * 8 + 32;
//    printf("%d\n", size);
    return predictor;
}

int16_t convert(uint32_t x) {
    if (x == 0) {
        return -1;
    }
    return 1;
}

int16_t sign(int32_t x) {
    if (x > 0) {
        return 1;
    } else if (x == 0) {
        return 0;
    }
    return -1;
}

uint8_t perceptron_predict(struct Perceptron *predictor, uint32_t pc) {
    uint32_t globalHistory = get_part(predictor->globalHistory, predictor->ghistoryBits-1, 0);
//    int lv1Key = fold(pc, predictor->pcIndexBits);
//    int lv1Key = get_part(pc, predictor->pcIndexBits-1, 0);
    int lv1Key = get_part(pc ^ predictor->globalHistory, predictor->pcIndexBits-1, 0);
//    int lv1Key = fold(pc ^ predictor->globalHistory, predictor->pcIndexBits);

    int32_t result = predictor->bias[lv1Key];
    for (int i = 0; i < predictor->ghistoryBits; ++i) {
        result += predictor->weights[lv1Key][i] * get_bit(globalHistory, i + 1);
    }
    if (result >= 0) {
        return 1;
    }
    return 0;
}

w_size add(int16_t a, int16_t b) {
    return a + b;
}

void perceptron_train(struct Perceptron *predictor, uint32_t pc, uint8_t outcome) {
    uint32_t globalHistory = get_part(predictor->globalHistory, predictor->ghistoryBits-1, 0);
//    int lv1Key = fold(pc, predictor->pcIndexBits);
//    int lv1Key = get_part(pc, predictor->pcIndexBits-1, 0);
    int lv1Key = get_part(pc ^ predictor->globalHistory, predictor->pcIndexBits-1, 0);
//    int lv1Key = fold(pc ^ predictor->globalHistory, predictor->pcIndexBits);

    int32_t result = predictor->bias[lv1Key];
    for (int i = 0; i < predictor->ghistoryBits; ++i) {
        result += predictor->weights[lv1Key][i] * get_bit(globalHistory, i + 1);
    }
    if (sign(result) != convert(outcome) || (-predictor->threshold < result && result < predictor->threshold)) {
        for (int i = 0; i < predictor->ghistoryBits; ++i) {
            predictor->weights[lv1Key][i] = add(convert(outcome) * convert(get_bit(globalHistory, i + 1)), predictor->weights[lv1Key][i]);
        }
        predictor->bias[lv1Key] = add(convert(outcome), predictor->bias[lv1Key]);
    }
    predictor->globalHistory = get_part((predictor->globalHistory << 1u) + outcome, predictor->ghistoryBits-1, 0);
}

#endif //PREDICTOR_PERCEPTRON_H
