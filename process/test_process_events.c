/*
 * Tiku Operating System
 * http://tiku-os.org
 *
 * Authors: Ambuj Varshney <ambuj@tiku-os.org>
 *
 * test_process_events.c - Event posting test
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

#if TEST_PROCESS_EVENTS

static volatile unsigned int event_recv_count = 0;

TIKU_PROCESS(test_event_proc, "test_events");

TIKU_PROCESS_THREAD(test_event_proc, ev, data)
{
    TIKU_PROCESS_BEGIN();

    while (1) {
        TIKU_PROCESS_WAIT_EVENT();

        if (ev == TEST_EVENT_CUSTOM) {
            event_recv_count++;
            TEST_PRINTF("Event process: received custom event #%d\n",
                         event_recv_count);
        }
    }

    TIKU_PROCESS_END();
}

void test_process_events(void)
{
    unsigned int i;
    unsigned int posted;

    TEST_GROUP_BEGIN("Event Posting");

    event_recv_count = 0;

    tiku_process_init();
    tiku_process_start(&test_event_proc, NULL);

    /* Drain the INIT event */
    while (tiku_process_run()) {
        /* drain */
    }

    /* Post multiple custom events */
    for (i = 0; i < TEST_NUM_EVENTS; i++) {
        posted = tiku_process_post(&test_event_proc,
                                    TEST_EVENT_CUSTOM, NULL);
        TEST_ASSERT(posted, "Post event succeeds");
    }

    /* Run scheduler to deliver all events */
    while (tiku_process_run()) {
        /* drain */
    }

    TEST_ASSERT(event_recv_count == TEST_NUM_EVENTS, "All custom events received");

    /* Verify queue return value: post should succeed */
    posted = tiku_process_post(&test_event_proc,
                                TEST_EVENT_CUSTOM, NULL);
    TEST_ASSERT(posted, "Post returns 1 on success");

    /* Clean up */
    tiku_process_exit(&test_event_proc);
    TEST_GROUP_END("Event Posting");
}

#endif /* TEST_PROCESS_EVENTS */

#endif /* PLATFORM_MSP430 */
