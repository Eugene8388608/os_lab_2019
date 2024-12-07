#include <stdint.h>
#include "utils.h"

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  return (__uint128_t) a * b % mod;
}