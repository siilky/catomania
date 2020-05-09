#pragma once

#include "types.h"

void tea_encrypt(barray & data, const uint32_t key[4]);
void tea_decrypt(barray & data, const uint32_t key[4]);
