/*
 * Tiku Operating System
 * http://tiku-os.org
 *
 * Authors: Ambuj Varshney <ambuj@tiku-os.org>
 *
 * test_process_lifecycle.c - Basic process lifecycle test
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

#include "test_process.h"

#ifdef PLATFORM_MSP430

#if TEST_PROCESS_LIFECYCLE

static volatile unsigned int lifecycle_phase = 0;

TIKU_PROCESS(test_lifecycle_proc, "test_lifecycle");

TIKU_PROCESS_THREAD(test_lifecycle_proc, ev, data)
{
    TIKU_PROCESS_BEGIN();

    /* Reached on first call (INIT event) */
    lifecycle_phase = 1;
    TEST_PRINTF("Lifecycle: initialized (phase 1)\n");

    TIKU_PROCESS_WAIT_EVENT();

    /* Reached on second event delivery */
    lifecycle_phase = 2;
    TEST_PRINTF("Lifecycle: continued (phase 2)\n");

    TIKU_PROCESS_END();
}

void test_process_lifecycle(void)
{
    TEST_GROUP_BEGIN("Basic Process Lifecycle");

    lifecycle_phase = 0;

    /* Initialize process system */
    tiku_process_init();
    TEST_PRINTF("Process system initialized\n");

    /* Start the process (posts INIT event) */
    tiku_process_start(&test_lifecycle_proc, NULL);
    TEST_PRINTF("Process started\n");

    /* Run scheduler to deliver INIT */
    while (tiku_process_run()) {
        /* drain */
    }

    /* Verify phase 1 reached */
    TEST_ASSERT(lifecycle_phase == 1, "Process received INIT event");

    /* Verify process is still running (yielded, not ended) */
    TEST_ASSERT(test_lifecycle_proc.is_running, "Process is running after yield");

    /* Post a continue event to advance past the yield */
    tiku_process_post(&test_lifecycle_proc, TIKU_EVENT_CONTINUE, NULL);
    while (tiku_process_run()) {
        /* drain */
    }

    /* Verify phase 2 reached and process auto-exited at PROCESS_END */
    TEST_ASSERT(lifecycle_phase == 2, "Process reached phase 2");

    TEST_ASSERT(!test_lifecycle_proc.is_running, "Process auto-exited at PROCESS_END");

    TEST_GROUP_END("Basic Process Lifecycle");
}

#endif /* TEST_PROCESS_LIFECYCLE */

#endif /* PLATFORM_MSP430 */
