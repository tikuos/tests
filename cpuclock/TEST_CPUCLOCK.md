# CPU Clock Tests

**Suite name:** `CPU Clock`
**Source directory:** `tests/cpuclock/`
**Header:** `test_cpuclock.h`
**Parent flag:** `TEST_CPUCLOCK`

## Overview

Tests for the TikuOS CPU clock subsystem. Currently limited to verifying
clock output on a GPIO pin for oscilloscope measurement. This is a
hardware-verification test rather than an assertion-based test.

**Hardware dependency:** Requires MSP430 hardware (`PLATFORM_MSP430`)
and an oscilloscope or logic analyzer connected to P3.4.

---

## Tests

### 1. Clock Output on P3.4

| Field | Value |
|-------|-------|
| **Flag** | `TEST_CPUCLOCK` |
| **Source** | `test_cpuclock_basic.c` |
| **Function** | `test_cpuclock_basic()` |
| **Group** | (none - no TEST_GROUP markers) |
| **Destructive** | No |

**What it tests:**
Configures GPIO pin P3.4 as a clock output by setting the direction register
(P3DIR) and special function select registers (P3SEL0, P3SEL1). This allows
an oscilloscope to measure the actual CPU clock frequency and verify it
matches the expected configuration (typically 8 MHz MCLK from DCO).

**Key parameters:**
- Output pin: P3.4
- Expected frequency: CPU MCLK (configured elsewhere in boot)

**Assertions:**
- (None - manual oscilloscope verification required)

---

## Requests

<!--
  Add new test requests below. Each request should describe:
  - What behavior to test
  - Expected outcome
  - Any specific parameters or edge cases

  Example format:

  ### Request: Verify DCO frequency after tiku_cpu_clock_set()
  Call tiku_cpu_clock_set() with each supported frequency (1, 4, 8, 16 MHz),
  output MCLK on P3.4, and use a timer capture to count cycles over a known
  ACLK interval. Assert that measured frequency is within 1% of target.
-->
