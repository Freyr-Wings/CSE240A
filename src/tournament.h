//
// Created by wk on 5/30/20.
//

#ifndef PREDICTOR_TOURNAMENT_H
#define PREDICTOR_TOURNAMENT_H

#include "predictor.h"
#include "utils.h"
#include <string.h>


struct Tournament {
    int ghistoryBits;
    int lhistoryBits;
    int pcIndexBits;
    uint32_t globalHistory;
    uint16_t *localHistory;
    uint8_t *localHistoryPredictTable;
    uint8_t *globalHistoryPredictTable;
    uint8_t *choiceHistoryPredictTable;
};

struct Tournament * new_tournament(int ghistoryBits, int lhistoryBits, int pcIndexBits) {
    struct Tournament *predictor = (struct Tournament *)malloc(sizeof(struct Tournament));
    predictor->ghistoryBits = ghistoryBits;
    predictor->lhistoryBits = lhistoryBits;
    predictor->pcIndexBits = pcIndexBits;
    predictor->globalHistory = 0;
    predictor->localHistory = (uint16_t *)malloc(sizeof(uint16_t)*(1 << pcIndexBits));
    predictor->localHistoryPredictTable = (uint8_t *)malloc(sizeof(uint8_t)*(1 << lhistoryBits));
    predictor->globalHistoryPredictTable = (uint8_t *)malloc(sizeof(uint8_t)*(1 << ghistoryBits));
    predictor->choiceHistoryPredictTable = (uint8_t *)malloc(sizeof(uint8_t)*(1 << ghistoryBits));
    memset(predictor->localHistory, 0, sizeof(uint16_t)*(1 << pcIndexBits));
    memset(predictor->localHistoryPredictTable, 1, sizeof(uint8_t)*(1 << lhistoryBits));
    memset(predictor->globalHistoryPredictTable, 1, sizeof(uint8_t)*(1 << ghistoryBits));
    memset(predictor->choiceHistoryPredictTable, 1, sizeof(uint8_t)*(1 << ghistoryBits));
    return predictor;
}

uint8_t tournament_predict(struct Tournament *predictor, uint32_t pc) {
    int lv1Key = get_part(pc, predictor->pcIndexBits-1, 0);
    int lv2Key = get_part(predictor->localHistory[lv1Key], predictor->lhistoryBits-1, 0);
    uint8_t localPrediction = get_bit(predictor->localHistoryPredictTable[lv2Key], 2);

    int globalKey = get_part(predictor->globalHistory, predictor->ghistoryBits-1, 0);
    uint8_t globalPrediction = get_bit(predictor->globalHistoryPredictTable[globalKey], 2);

    uint8_t choicePrediction = get_bit(predictor->choiceHistoryPredictTable[globalKey], 2);

    if (choicePrediction == 1) {
        return localPrediction;
    }
    return globalPrediction;
}

uint8_t update_2bit_prediction(uint8_t oldValue, uint8_t outcome, int predictBits) {
    uint8_t newValue = oldValue;
    if (outcome == TAKEN) {
        if (oldValue <  ((1u << (uint8_t)predictBits) - 1u)) {
            newValue ++;
        }
    } else {
        if (oldValue > 0) {
            newValue --;
        }
    }
    return newValue;
}

uint8_t update_choice_prediction(uint8_t p1, uint8_t p2, uint8_t oldValue, uint8_t outcome, int predictBits) {
    uint8_t p1Correct = 0;
    if (p1 == outcome) {
        p1Correct = 1;
    }

    uint8_t p2Correct = 0;
    if (p2 == outcome) {
        p2Correct = 1;
    }

    uint8_t newValue = oldValue;
    if (p1Correct - p2Correct == 1) {
        if (oldValue < ((1u << (uint8_t)predictBits) - 1u)) {
            newValue ++;
        }
    } else if (p1Correct - p2Correct == -1) {
        if (oldValue > 0) {
            newValue --;
        }
    }
    return newValue;
}

void tournament_train(struct Tournament *predictor, uint32_t pc, uint8_t outcome) {
    int lv1Key = get_part(pc, predictor->pcIndexBits-1, 0);
    int lv2Key = get_part(predictor->localHistory[lv1Key], predictor->lhistoryBits-1, 0);
    uint8_t localPrediction = get_bit(predictor->localHistoryPredictTable[lv2Key], 2);
    predictor->localHistoryPredictTable[lv2Key] = update_2bit_prediction(predictor->localHistoryPredictTable[lv2Key], outcome, 2);

    predictor->localHistory[lv1Key] = get_part((predictor->localHistory[lv1Key] << 1u) + outcome, predictor->pcIndexBits-1, 0);

    int globalKey = get_part(predictor->globalHistory, predictor->ghistoryBits-1, 0);
    uint8_t globalPrediction = get_bit(predictor->globalHistoryPredictTable[globalKey], 2);
    predictor->globalHistoryPredictTable[globalKey] = update_2bit_prediction(predictor->globalHistoryPredictTable[globalKey], outcome, 2);
    predictor->choiceHistoryPredictTable[globalKey] = update_choice_prediction(localPrediction, globalPrediction, predictor->choiceHistoryPredictTable[globalKey], outcome, 2);

    predictor->globalHistory = get_part((predictor->globalHistory << 1u) + outcome, predictor->ghistoryBits-1, 0);
}


#endif //PREDICTOR_TOURNAMENT_H
