#!/bin/sh

for off in 0 1; do ./check_align uint16_t $off; done
for off in 0 1 2 3; do ./check_align uint32_t $off; done
for off in 0 1 2 3 4 5 6 7; do ./check_align uint64_t $off; done
for off in 0 1 2 3 4 5 6 7; do ./check_align double $off; done
