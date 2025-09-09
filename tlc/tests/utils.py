#!/usr/bin/env python3

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import random


def random_bytes(length: int) -> bytes:
    return bytes(random.getrandbits(8) for _ in range(length))
