//
// Created by msi on 2020/5/31.
//

#ifndef PREDICTOR_TAGE_H
#define PREDICTOR_TAGE_H

#include "gshare2.h"
#include "history.h"

struct TAGE {
    int num_gshare;
    struct GShare2 **predictors;
    int *weights;
};

struct TAGE * NewTAGE(int num_gshare) {
    num_gshare = 4;
    struct TAGE *predictor = (struct TAGE *)malloc(sizeof(struct TAGE));
    predictor->num_gshare = num_gshare;
    predictor->predictors = (struct GShare2 **)malloc(sizeof(struct GShare2 *)*num_gshare);
//    for (int i = 0; i < num_gshare; ++i) {
//        predictor->predictors[i] = NewGShare(16 * (i + 1), 13, 2);
//    }

    predictor->predictors[0] = NewGShare2(5, 12, 2);
    predictor->predictors[1] = NewGShare2(15, 12, 2);
    predictor->predictors[2] = NewGShare2(44, 13, 2);
    predictor->predictors[3] = NewGShare2(131, 13, 2);

    predictor->weights = (int *)malloc(sizeof(int)*num_gshare);
    memset(predictor->weights, 0, sizeof(int)*num_gshare);
    return predictor;
}

//uint8_t tage_predict(struct TAGE *predictor, uint32_t pc) {
//    int taken = 0;
//    int total = 0;
//    for (int i = 0; i < predictor->num_gshare; ++i) {
//        uint8_t prediction = gshare_predict(predictor->predictors[i],pc);
//        taken += prediction * predictor->weights[i];
//        total += predictor->weights[i];
//    }
//    if ((total / 2) > taken) {
//        return NOTTAKEN;
//    }
//    return TAKEN;
//}

uint8_t tage_predict(struct TAGE *predictor, uint32_t pc) {
    int max_weight = -1;
    int max_idx = -1;
    for (int i = 0; i < predictor->num_gshare; ++i) {
        if (predictor->weights[i] > max_weight) {
            max_weight = predictor->weights[i];
            max_idx = i;
        }
    }
    uint8_t prediction = gshare2_predict(predictor->predictors[max_idx],pc);
    return prediction;
}

void tage_train(struct TAGE *predictor, uint32_t pc, uint8_t outcome) {
    for (int i = 0; i < predictor->num_gshare; ++i) {
        uint8_t prediction = gshare2_predict(predictor->predictors[i],pc);
        if (prediction == outcome) {
            if (predictor->weights[i] < 200) {
                predictor->weights[i]+=1;
//                if (predictor->weights[i] == 0) {
//                    predictor->weights[i] = 1;
//                }
            }
        } else {
            if (predictor->weights[i] > 0) {
                predictor->weights[i]*=19;
                predictor->weights[i]/=20;
            }
        }
        gshare2_train(predictor->predictors[i], pc, outcome);
    }
}



#endif //PREDICTOR_TAGE_H
