#pragma once
#include <stdint.h>

struct FactorialArgs {
  uint64_t begin, end, mod;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod);