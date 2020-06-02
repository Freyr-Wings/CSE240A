//
// Created by wk on 6/1/20.
//

#ifndef PREDICTOR_PERCEPTRON_H
#define PREDICTOR_PERCEPTRON_H

#include "predictor.h"
#include "utils.h"
#include <string.h>


struct Perceptron {
    int ghistoryBits;
    int pcIndexBits;
    int8_t threshold;
    uint32_t globalHistory;

    int16_t **weights;
    int16_t *bias;
};

struct Perceptron * new_perceptron(int ghistoryBits, int pcIndexBits) {
    struct Perceptron *predictor = (struct Perceptron *)malloc(sizeof(struct Perceptron));
    predictor->ghistoryBits = ghistoryBits;
    predictor->pcIndexBits = pcIndexBits;
    predictor->threshold = 50;
    predictor->globalHistory = 0;

    predictor->weights = (int16_t **)malloc(sizeof(int16_t *) * (1 << pcIndexBits));
    for (int i = 0; i < (1 << pcIndexBits); ++i) {
        predictor->weights[i] = (int16_t *) malloc(sizeof(int16_t) * ghistoryBits);
        memset(predictor->weights[i], 0, sizeof(int16_t) * ghistoryBits);
    }

    predictor->bias = (int16_t *)malloc(sizeof(int16_t) * (1 << pcIndexBits));
    memset(predictor->bias, 0, sizeof(int16_t) * (1 << pcIndexBits));

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
    int lv1Key = get_part(pc, predictor->pcIndexBits-1, 0);
    uint32_t globalHistory = get_part(predictor->globalHistory, predictor->ghistoryBits-1, 0);

    int32_t result = predictor->bias[lv1Key];
    for (int i = 0; i < pcIndexBits; ++i) {
        result += predictor->weights[lv1Key][i] * get_bit(globalHistory, i + 1);
    }
    if (result >= 0) {
        return 1;
    }
    return 0;
}

void perceptron_train(struct Perceptron *predictor, uint32_t pc, uint8_t outcome) {
    int lv1Key = get_part(pc, predictor->pcIndexBits-1, 0);
    uint32_t globalHistory = get_part(predictor->globalHistory, predictor->ghistoryBits-1, 0);

    int32_t result = predictor->bias[lv1Key];
    for (int i = 0; i < pcIndexBits; ++i) {
        result += predictor->weights[lv1Key][i] * get_bit(globalHistory, i + 1);
    }
    if (sign(result) != convert(outcome) || (-predictor->threshold < result && result < predictor->threshold)) {
        for (int i = 0; i < pcIndexBits; ++i) {
            predictor->weights[lv1Key][i] += convert(outcome) * convert(get_bit(globalHistory, i + 1));
        }
        predictor->bias[lv1Key] += convert(outcome);
    }
    predictor->globalHistory = get_part((predictor->globalHistory << 1u) + outcome, predictor->ghistoryBits-1, 0);
}

#endif //PREDICTOR_PERCEPTRON_H
