#pragma once

#include <cstdint>

void makeWaveTables();
int loadSound(const char *path, uint8_t **buf, size_t *length);