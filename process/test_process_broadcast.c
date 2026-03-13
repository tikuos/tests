/*
 * Tiku Operating System
 * http://tiku-os.org
 *
 * Authors: Ambuj Varshney <ambuj@tiku-os.org>
 *
 * test_process_broadcast.c - Broadcast event delivery test
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

#if TEST_PROCESS_BROADCAST

static volatile unsigned int bcast_count_a = 0;
static volatile unsigned int bcast_count_b = 0;

TIKU_PROCESS(test_bcast_proc_a, "bcast_a");
TIKU_PROCESS(test_bcast_proc_b, "bcast_b");

TIKU_PROCESS_THREAD(test_bcast_proc_a, ev, data)
{
    TIKU_PROCESS_BEGIN();

    while (1) {
        TIKU_PROCESS_WAIT_EVENT();

        if (ev == TEST_EVENT_CUSTOM) {
            bcast_count_a++;
            TEST_PRINTF("Broadcast A: received event #%d\n",
                         bcast_count_a);
        }
    }

    TIKU_PROCESS_END();
}

TIKU_PROCESS_THREAD(test_bcast_proc_b, ev, data)
{
    TIKU_PROCESS_BEGIN();

    while (1) {
        TIKU_PROCESS_WAIT_EVENT();

        if (ev == TEST_EVENT_CUSTOM) {
            bcast_count_b++;
            TEST_PRINTF("Broadcast B: received event #%d\n",
                         bcast_count_b);
        }
    }

    TIKU_PROCESS_END();
}

void test_process_broadcast(void)
{
    TEST_GROUP_BEGIN("Broadcast Events");

    bcast_count_a = 0;
    bcast_count_b = 0;

    tiku_process_init();
    tiku_process_start(&test_bcast_proc_a, NULL);
    tiku_process_start(&test_bcast_proc_b, NULL);

    /* Drain INIT events for both processes */
    while (tiku_process_run()) {
        /* drain */
    }

    /* Post a broadcast event (NULL target = all processes) */
    tiku_process_post(TIKU_PROCESS_BROADCAST, TEST_EVENT_CUSTOM, NULL);

    /* Run scheduler to deliver broadcast */
    while (tiku_process_run()) {
        /* drain */
    }

    TEST_ASSERT(bcast_count_a == 1, "Process A received broadcast");

    TEST_ASSERT(bcast_count_b == 1, "Process B received broadcast");

    /* Post a second broadcast to verify repeated delivery */
    tiku_process_post(TIKU_PROCESS_BROADCAST, TEST_EVENT_CUSTOM, NULL);
    while (tiku_process_run()) {
        /* drain */
    }

    TEST_ASSERT(bcast_count_a == 2 && bcast_count_b == 2, "Both processes received second broadcast");

    /* Clean up */
    tiku_process_exit(&test_bcast_proc_a);
    tiku_process_exit(&test_bcast_proc_b);
    TEST_GROUP_END("Broadcast Events");
}

#endif /* TEST_PROCESS_BROADCAST */

#endif /* PLATFORM_MSP430 */
