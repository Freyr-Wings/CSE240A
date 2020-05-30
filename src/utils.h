//
// Created by Binlu on 2020/5/30.
//

#ifndef PREDICTOR_UTILS_H
#define PREDICTOR_UTILS_H

uint32_t gen_mask(uint32_t start, uint32_t end) {
    uint32_t mask = (1u << (start - end + 1)) - 1;
    return mask << end;
}

uint32_t get_part(uint32_t addr, uint32_t start, uint32_t end) {
    uint32_t mask = gen_mask(start, end);
    return (addr & mask) >> end;
}

uint32_t get_bit(uint32_t addr, uint32_t num) {
    return (addr >> (num-1)) & 1u;
}


#endif //PREDICTOR_UTILS_H
