/*
 * Tiku Operating System
 * http://tiku-os.org
 *
 * Authors: Ambuj Varshney <ambuj@tiku-os.org>
 *
 * tiku_test_harness.h - Shared test harness with machine-parseable output
 *
 * All test modules should include this header instead of defining their
 * own TEST_ASSERT / TEST_PRINT macros.
 *
 * Output format (for automated tooling):
 *   [TS:BEGIN] suite_name          -- test suite start, counters reset
 *   [TG:BEGIN] group_name          -- test group start
 *   [T:P:N] description            -- assertion N passed
 *   [T:F:N] description            -- assertion N failed
 *   [TG:END] group_name            -- test group end
 *   [TS:END] suite_name total=T pass=P fail=F  -- suite summary
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TIKU_TEST_HARNESS_H_
#define TIKU_TEST_HARNESS_H_

#include <stdio.h>

/*---------------------------------------------------------------------------*/
/* PLATFORM-AWARE PRINT                                                      */
/*---------------------------------------------------------------------------*/

#ifdef PLATFORM_MSP430
#include "tiku.h"
#define TEST_PRINT(...) TIKU_PRINTF(__VA_ARGS__)
#else
#define TEST_PRINT(...) printf(__VA_ARGS__)
#endif

/*---------------------------------------------------------------------------*/
/* TEST COUNTERS                                                             */
/*---------------------------------------------------------------------------*/

extern int tests_run;
extern int tests_passed;
extern int tests_failed;

/*---------------------------------------------------------------------------*/
/* ASSERTION MACRO                                                           */
/*---------------------------------------------------------------------------*/

/**
 * @brief Assert a condition and emit machine-parseable result
 *
 * Output: [T:P:N] msg   (pass) or [T:F:N] msg   (fail)
 * where N is the running test number.
 */
#define TEST_ASSERT(cond, msg)                                              \
    do {                                                                    \
        tests_run++;                                                        \
        if (cond) {                                                         \
            tests_passed++;                                                 \
            TEST_PRINT("[T:P:%d] %s\n", tests_run, msg);                   \
        } else {                                                            \
            tests_failed++;                                                 \
            TEST_PRINT("[T:F:%d] %s\n", tests_run, msg);                   \
        }                                                                   \
    } while (0)

/*---------------------------------------------------------------------------*/
/* SUITE / GROUP MARKERS                                                     */
/*---------------------------------------------------------------------------*/

/**
 * @brief Mark the beginning of a test suite (resets counters)
 */
#define TEST_SUITE_BEGIN(name)                                              \
    do {                                                                    \
        tests_run    = 0;                                                   \
        tests_passed = 0;                                                   \
        tests_failed = 0;                                                   \
        TEST_PRINT("[TS:BEGIN] %s\n", name);                               \
    } while (0)

/**
 * @brief Mark the end of a test suite (prints summary)
 */
#define TEST_SUITE_END(name)                                                \
    TEST_PRINT("[TS:END] %s total=%d pass=%d fail=%d\n",                   \
               name, tests_run, tests_passed, tests_failed)

/**
 * @brief Mark the beginning of a test group within a suite
 */
#define TEST_GROUP_BEGIN(name)                                              \
    TEST_PRINT("[TG:BEGIN] %s\n", name)

/**
 * @brief Mark the end of a test group
 */
#define TEST_GROUP_END(name)                                                \
    TEST_PRINT("[TG:END] %s\n", name)

#endif /* TIKU_TEST_HARNESS_H_ */
