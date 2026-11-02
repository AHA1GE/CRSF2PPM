#pragma once

#include <Arduino.h>

void ledInit();
void ledStart();
const char *ledModeName();
uint32_t ledDuty();
bool ledButtonPressed();
