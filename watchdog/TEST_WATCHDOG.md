# Watchdog Timer Tests

**Suite name:** `Watchdog`
**Source directory:** `tests/watchdog/`
**Header:** `test_watchdog.h`
**Parent flag:** `TEST_WATCHDOG` (auto-derived from sub-flags)

## Overview

Tests for the TikuOS watchdog timer subsystem (`tiku_watchdog_*` API).
The watchdog provides system-level fault recovery by resetting the MCU
when software fails to "kick" the timer within its timeout window. These
tests verify basic operation, pause/resume, interval timer mode, and
intentional timeout reset.

**Hardware dependency:** All tests require MSP430 hardware (`PLATFORM_MSP430`).
Tests use ACLK (32.768 kHz) as the watchdog clock source.

---

## Tests

### 1. Basic Watchdog Operation

| Field | Value |
|-------|-------|
| **Flag** | `TEST_WDT_BASIC` |
| **Source** | `test_watchdog_basic.c` |
| **Function** | `test_watchdog_basic()` |
| **Group** | "Basic Watchdog Operation" |
| **Destructive** | No |

**What it tests:**
Configures the watchdog in watchdog mode with ACLK source and ~1 second
timeout (WDTIS__32768). Kicks the watchdog every 500 ms in a loop for
30 iterations, toggling LED1 every 10 kicks. If the system does not reset,
the watchdog kick mechanism is working correctly.

**Key parameters:**
- Timeout: ~1 s (32768 ACLK cycles)
- Kick interval: 500 ms (`TEST_WATCHDOG_DELAY_NORMAL`)
- Iterations: 30

**Assertions:**
- (Implicit) System does not reset during the 30-kick loop

---

### 2. Watchdog Pause/Resume

| Field | Value |
|-------|-------|
| **Flag** | `TEST_WDT_PAUSE_RESUME` |
| **Source** | `test_watchdog_pause_resume.c` |
| **Function** | `test_watchdog_pause_resume()` |
| **Group** | "Watchdog Pause/Resume" |
| **Destructive** | No |

**What it tests:**
Starts the watchdog with a ~250 ms timeout (WDTIS__8192). Kicks normally for
5 iterations, then calls `tiku_watchdog_pause()` and performs a long operation
(10 iterations x 300 ms = 3 seconds) without kicking. After the long operation,
calls `tiku_watchdog_resume_with_kick()` and kicks normally for 5 more
iterations. Validates that pausing prevents timeout during extended operations.

**Key parameters:**
- Timeout: ~250 ms (8192 ACLK cycles)
- Normal kick interval: 100 ms
- Paused operation interval: 300 ms (exceeds timeout)
- Paused iterations: 10

**Assertions:**
- (Implicit) System does not reset during the paused window
- (Implicit) System continues operating normally after resume

---

### 3. Interval Timer Mode

| Field | Value |
|-------|-------|
| **Flag** | `TEST_WDT_INTERVAL` |
| **Source** | `test_watchdog_interval.c` |
| **Function** | `test_watchdog_interval_timer()` |
| **Group** | "Interval Timer Mode" |
| **Destructive** | No |

**What it tests:**
Configures the watchdog as an interval timer (WDTTMSEL) instead of reset mode.
Uses ACLK with WDTIS__8192 for ~250 ms periodic interrupts. Enables the WDT
interrupt (WDTIE) and counts ISR invocations over 50 polling iterations at
100 ms each. Verifies that the watchdog generates periodic interrupts without
causing a system reset.

**Key parameters:**
- Interval: ~250 ms (8192 ACLK cycles)
- Polling iterations: 50
- Poll interval: 100 ms

**Assertions:**
- (Implicit) ISR counter increments, proving interrupts fire
- (Implicit) No system reset occurs

---

### 4. Watchdog Timeout (System Reset)

| Field | Value |
|-------|-------|
| **Flag** | `TEST_WDT_TIMEOUT` |
| **Source** | `test_watchdog_timeout.c` |
| **Function** | `test_watchdog_timeout()` |
| **Group** | "Watchdog Timeout Demo" |
| **Destructive** | **YES** - causes system reset |

**What it tests:**
Configures watchdog with ~15 ms timeout (WDTIS__512) and deliberately does
NOT kick it. The system should reset within milliseconds. This test verifies
that the watchdog actually triggers a hardware reset when not serviced.

**Key parameters:**
- Timeout: ~15 ms (512 ACLK cycles)
- 2000 ms warning delay before starting

**Assertions:**
- (Implicit) System resets (test never completes normally)

---

## Requests

<!--
  Add new test requests below. Each request should describe:
  - What behavior to test
  - Expected outcome
  - Any specific parameters or edge cases

  An LLM will read these requests and generate the corresponding C test
  code in a new file under tests/watchdog/. After implementation, move
  the request to the Tests section above with full documentation.

  Example format:

  ### Request: Watchdog clock source switching
  Test switching the watchdog clock source from ACLK to SMCLK mid-operation.
  After switching, verify the watchdog still triggers correctly with the new
  clock source's timing. Expected: no reset during switch, correct timeout
  with new source.
-->
