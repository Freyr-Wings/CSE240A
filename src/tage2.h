//
// Created by msi on 2020/6/3.
//

#ifndef PREDICTOR_TAGE2_H
#define PREDICTOR_TAGE2_H

#include "gshare2.h"
#include "history.h"



struct BaseEntry {
    uint8_t saturate_bit;  // 3 bit
    uint8_t modified_bit;  // 1 bit
};

struct Base {
    int num_key_bit;
    int num_predict_bit;
    struct BaseEntry *predict_table;
};

struct Base * NewBase(int num_key_bit, int num_predict_bit) {
    struct Base *predictor = (struct Base *)malloc(sizeof(struct Base));
    predictor->num_key_bit = num_key_bit;
    predictor->num_predict_bit = num_predict_bit;
    predictor->predict_table = (struct BaseEntry *)malloc(sizeof(struct BaseEntry)*(1 << num_key_bit));
    memset(predictor->predict_table, 0, sizeof(struct BaseEntry)*(1 << num_key_bit));
    return predictor;
}

uint8_t base_predict(struct Base *predictor, uint32_t pc) {
    int key = get_part(pc, predictor->num_key_bit-1, 0);
    uint8_t prediction = get_bit(predictor->predict_table[key].saturate_bit, predictor->num_predict_bit);
    return prediction;
}

void base_train(struct Base *predictor, uint32_t pc, uint8_t outcome) {
    int key = get_part(pc, predictor->num_key_bit-1, 0);
    uint8_t prediction = predictor->predict_table[key].saturate_bit;
    if (outcome == TAKEN) {
        if (prediction <  ((1u << (uint8_t)predictor->num_predict_bit) - 1u)) {
            prediction ++;
        }
    } else {
        if (prediction > 0) {
            prediction --;
        }
    }
    predictor->predict_table[key].saturate_bit = prediction;
}

struct TaggedEntry {
    uint16_t tag_bit;  // 9 bit
    uint8_t saturate_bit;  // 3 bit
    uint8_t useful_bit;  // 2 bit
};

struct Tagged {
    int num_tag_bit;
    int num_key_bit;
    int num_predict_bit;
    int num_useful_bit;
    struct History *history;
    struct TaggedEntry *predict_table;
};

struct Tagged * NewTagged(int num_tag_bit, int num_key_bit, int num_predict_bit, int num_useful_bit, int num_history_bit) {
    struct Tagged *predictor = (struct Tagged *)malloc(sizeof(struct Tagged));
    predictor->num_tag_bit = num_tag_bit;
    predictor->num_key_bit = num_key_bit;
    predictor->num_predict_bit = num_predict_bit;
    predictor->num_useful_bit = num_useful_bit;
    predictor->history = NewHistory(num_history_bit);
    predictor->predict_table = (struct TaggedEntry *)malloc(sizeof(struct TaggedEntry)*(1 << num_key_bit));
    memset(predictor->predict_table, 0, sizeof(struct TaggedEntry)*(1 << num_key_bit));
    return predictor;
}

void tagged_update_useful(struct Tagged *predictor, uint32_t pc, int useful) {
    int key = get_part(
            fold_history(predictor->history, predictor->num_key_bit) ^ pc ^ (pc >> predictor->num_key_bit),
            predictor->num_key_bit-1, 0);
    if (useful) {
        if (predictor->predict_table[key].useful_bit <  ((1u << (uint8_t)predictor->num_useful_bit) - 1u)) {
            predictor->predict_table[key].useful_bit ++;
        }
    } else {
        if (predictor->predict_table[key].useful_bit > 0) {
            predictor->predict_table[key].useful_bit --;
        }
    }
}

void tagged_reset_key(struct Tagged *predictor, uint32_t pc, uint8_t outcome) {
    int key = get_part(
            fold_history(predictor->history, predictor->num_key_bit) ^ pc ^ (pc >> predictor->num_key_bit),
            predictor->num_key_bit-1, 0);
    predictor->predict_table[key].tag_bit = get_part(
            pc ^ fold_history(predictor->history, predictor->num_key_bit) ^ (fold_history(predictor->history, predictor->num_key_bit-1) << 1u),
            predictor->num_tag_bit-1, 0);
    if (outcome == 0) {
        predictor->predict_table[key].saturate_bit = 3u;
    } else {
        predictor->predict_table[key].saturate_bit = 4u;
    }

}

uint8_t tagged_predict(struct Tagged *predictor, uint32_t pc) {
    int key = get_part(
            fold_history(predictor->history, predictor->num_key_bit) ^ pc ^ (pc >> predictor->num_key_bit),
            predictor->num_key_bit-1, 0);
    uint16_t tag = get_part(
            pc ^ fold_history(predictor->history, predictor->num_key_bit) ^ (fold_history(predictor->history, predictor->num_key_bit-1) << 1u),
            predictor->num_tag_bit-1, 0);
    uint8_t prediction = get_bit(predictor->predict_table[key].saturate_bit, predictor->num_predict_bit);

    if (predictor->predict_table[key].tag_bit == tag) {
        prediction |= 2u;
    }

    if (predictor->predict_table[key].useful_bit == 0) {
        prediction |= 4u;
    }

    return prediction;
}

void tagged_train(struct Tagged *predictor, uint32_t pc, uint8_t outcome, uint8_t prediction, int diff) {
    int key = get_part(
            fold_history(predictor->history, predictor->num_key_bit) ^ pc ^ (pc >> predictor->num_key_bit),
            predictor->num_key_bit-1, 0);

    if (diff) {
        if (prediction == outcome) {
            if (predictor->predict_table[key].useful_bit <  ((1u << (uint8_t)predictor->num_useful_bit) - 1u)) {
                predictor->predict_table[key].useful_bit ++;
            }
        } else {
            if (predictor->predict_table[key].useful_bit > 0) {
                predictor->predict_table[key].useful_bit --;
            }
        }
    }

    if (outcome == TAKEN) {
        if (predictor->predict_table[key].saturate_bit <  ((1u << (uint8_t)predictor->num_predict_bit) - 1u)) {
            predictor->predict_table[key].saturate_bit ++;
        }
    } else {
        if (predictor->predict_table[key].saturate_bit > 0) {
            predictor->predict_table[key].saturate_bit --;
        }
    }
}


struct TAGE {
    int num_gshare;
    struct Base *base_predictor;
    struct Tagged **tagged_predictors;
};

struct TAGE * NewTAGE() {
    srand(233);
    int num_gshare = 4;
    struct TAGE *predictor = (struct TAGE *)malloc(sizeof(struct TAGE));
    predictor->num_gshare = num_gshare;
    predictor->base_predictor = NewBase(12, 3);
    predictor->tagged_predictors = (struct Tagged **)malloc(sizeof(struct Tagged *)*num_gshare);

    predictor->tagged_predictors[0] = NewTagged(9, 10, 3, 2, 5);
    predictor->tagged_predictors[1] = NewTagged(9, 10, 3, 2, 15);
    predictor->tagged_predictors[2] = NewTagged(9, 10, 3, 2, 44);
    predictor->tagged_predictors[3] = NewTagged(9, 10, 3, 2, 133);

    return predictor;
}

int choose_next(int len) {
    int limit = 1;
    for (int i = 0; i < len-1; ++i) {
        limit <<= 1;
    }
    int count = len-1;
    int choose = rand() % limit;
    while (choose > 0) {
        count --;
        choose >>= 1;
    }

    return count;
}

uint8_t tage_predict(struct TAGE *predictor, uint32_t pc) {
    uint8_t base_prediction = base_predict(predictor->base_predictor, pc);
    for (int i = 3; i >= 0; --i) {
        uint8_t predictions = tagged_predict(predictor->tagged_predictors[i], pc);
        if (predictions & 2u) {
            return predictions & 1u;
        }
    }
    return base_prediction;
}

void tage_train(struct TAGE *predictor, uint32_t pc, uint8_t outcome) {
    uint8_t base_prediction = base_predict(predictor->base_predictor, pc);
    uint8_t tagged_predictions[4] = {0};
    int pri_idx = -1, alt_idx = -1;
    for (int i = 3; i >= 0; --i) {
        tagged_predictions[i] = tagged_predict(predictor->tagged_predictors[i], pc);
        if (tagged_predictions[i] & 2u) {
            if (pri_idx < 0) {
                pri_idx = i;
                continue;
            }
            if (alt_idx < 0) {
                alt_idx = i;
                break;
            }
        }
    }

    int diff = 0;
    if (pri_idx >= 0) {
        if (alt_idx >= 0) {
            if ((tagged_predictions[pri_idx] & 1u) != (tagged_predictions[alt_idx] & 1u)) {
                diff = 1;
            }
        } else {
            if (tagged_predictions[pri_idx] != base_prediction) {
                diff = 1;
            }
        }

        if (tagged_predictions[pri_idx] == outcome) {
            tagged_train(predictor->tagged_predictors[pri_idx], pc, outcome, tagged_predictions[pri_idx], diff);
        }
    }

    if ((pri_idx >= 0 && (tagged_predictions[pri_idx] & 1u) != outcome) || (pri_idx < 0 && base_prediction != outcome)) {
        if (pri_idx < 3) {
            int choice = 0;
            for (int i = pri_idx + 1; i < 4; ++i) {
                if (tagged_predictions[i] & 4u) {
                    choice ++;
                }
            }

            if (choice == 0) {
                for (int i = pri_idx + 1; i < 4; ++i) {
                    tagged_update_useful(predictor->tagged_predictors[i], pc, 0);
                }
            } else {
                int next = choose_next(choice);
                int next_idx = pri_idx;
                for (int i = pri_idx + 1; i < 4; ++i) {
                    if (tagged_predictions[i] & 4u) {
                        if (next == 0) {
                            next_idx = i;
                            break;
                        }
                        next --;
                    }
                }
                tagged_reset_key(predictor->tagged_predictors[next_idx], pc, outcome);
            }
        }
    }
    base_train(predictor->base_predictor, pc, outcome);
}

#endif //PREDICTOR_TAGE2_H
