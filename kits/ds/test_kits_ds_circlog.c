/*
 * Tiku Operating System
 * http://tiku-os.org
 *
 * Authors: Ambuj Varshney <ambuj@tiku-os.org>
 *
 * test_kits_ds_circlog.c - Circular log data structure tests
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

#include "tests/tiku_test_harness.h"
#include "tikukits/ds/circlog/tiku_kits_ds_circlog.h"

/*---------------------------------------------------------------------------*/
/* TEST 1: INITIALIZATION                                                    */
/*---------------------------------------------------------------------------*/

void test_kits_ds_circlog_init(void)
{
    struct tiku_kits_ds_circlog log;
    int rc;

    TEST_GROUP_BEGIN("CircLog Init");

    /* Valid init */
    rc = tiku_kits_ds_circlog_init(&log, 8);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "init cap=8 returns OK");
    TEST_ASSERT(tiku_kits_ds_circlog_count(&log) == 0,
                "count is 0 after init");

    /* Max capacity accepted */
    rc = tiku_kits_ds_circlog_init(&log,
                                    TIKU_KITS_DS_CIRCLOG_MAX_ENTRIES);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "init max capacity OK");

    /* Zero capacity rejected */
    rc = tiku_kits_ds_circlog_init(&log, 0);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_PARAM,
                "zero capacity rejected");

    /* Oversized capacity rejected */
    rc = tiku_kits_ds_circlog_init(
        &log, TIKU_KITS_DS_CIRCLOG_MAX_ENTRIES + 1);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_PARAM,
                "oversized capacity rejected");

    TEST_GROUP_END("CircLog Init");
}

/*---------------------------------------------------------------------------*/
/* TEST 2: APPEND AND READ LATEST                                            */
/*---------------------------------------------------------------------------*/

void test_kits_ds_circlog_append_read(void)
{
    struct tiku_kits_ds_circlog log;
    struct tiku_kits_ds_circlog_entry entry;
    uint8_t payload[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    int rc;

    TEST_GROUP_BEGIN("CircLog Append/Read");

    tiku_kits_ds_circlog_init(&log, 8);

    /* Append a single entry */
    rc = tiku_kits_ds_circlog_append(&log, 1, 10, 1000,
                                      payload, 4);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "append returns OK");
    TEST_ASSERT(tiku_kits_ds_circlog_count(&log) == 1,
                "count is 1 after append");

    /* Read it back */
    rc = tiku_kits_ds_circlog_read_latest(&log, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "read_latest returns OK");
    TEST_ASSERT(entry.level == 1, "level is 1");
    TEST_ASSERT(entry.tag == 10, "tag is 10");
    TEST_ASSERT(entry.timestamp == 1000, "timestamp is 1000");
    TEST_ASSERT(entry.payload_len == 4, "payload_len is 4");

    /* Read from empty log */
    tiku_kits_ds_circlog_init(&log, 8);
    rc = tiku_kits_ds_circlog_read_latest(&log, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_EMPTY,
                "read_latest empty returns ERR_EMPTY");

    TEST_GROUP_END("CircLog Append/Read");
}

/*---------------------------------------------------------------------------*/
/* TEST 3: MULTIPLE ENTRIES                                                  */
/*---------------------------------------------------------------------------*/

void test_kits_ds_circlog_multiple_entries(void)
{
    struct tiku_kits_ds_circlog log;
    struct tiku_kits_ds_circlog_entry entry;
    uint8_t payload[1] = {0x00};
    int rc;

    TEST_GROUP_BEGIN("CircLog Multiple Entries");

    tiku_kits_ds_circlog_init(&log, 8);

    /* Append three entries with different timestamps */
    tiku_kits_ds_circlog_append(&log, 1, 1, 100, payload, 1);
    tiku_kits_ds_circlog_append(&log, 2, 2, 200, payload, 1);
    tiku_kits_ds_circlog_append(&log, 3, 3, 300, payload, 1);

    TEST_ASSERT(tiku_kits_ds_circlog_count(&log) == 3,
                "count is 3 after 3 appends");

    /* Latest should be the third entry */
    rc = tiku_kits_ds_circlog_read_latest(&log, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "read_latest OK");
    TEST_ASSERT(entry.timestamp == 300,
                "latest has timestamp 300");
    TEST_ASSERT(entry.level == 3, "latest has level 3");
    TEST_ASSERT(entry.tag == 3, "latest has tag 3");

    /* Index 0 = newest, index 2 = oldest */
    rc = tiku_kits_ds_circlog_read_at(&log, 0, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "read_at(0) OK");
    TEST_ASSERT(entry.timestamp == 300,
                "index 0 is newest (ts=300)");

    rc = tiku_kits_ds_circlog_read_at(&log, 2, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "read_at(2) OK");
    TEST_ASSERT(entry.timestamp == 100,
                "index 2 is oldest (ts=100)");

    TEST_GROUP_END("CircLog Multiple Entries");
}

/*---------------------------------------------------------------------------*/
/* TEST 4: WRAPAROUND                                                        */
/*---------------------------------------------------------------------------*/

void test_kits_ds_circlog_wraparound(void)
{
    struct tiku_kits_ds_circlog log;
    struct tiku_kits_ds_circlog_entry entry;
    uint8_t payload[1] = {0x00};
    uint16_t i;
    int rc;

    TEST_GROUP_BEGIN("CircLog Wraparound");

    tiku_kits_ds_circlog_init(&log, 4);

    /* Fill beyond capacity: append 6 entries into cap=4 */
    for (i = 0; i < 6; i++) {
        rc = tiku_kits_ds_circlog_append(&log, (uint8_t)i, (uint8_t)i,
                                          (uint32_t)(i + 1) * 100,
                                          payload, 1);
        TEST_ASSERT(rc == TIKU_KITS_DS_OK, "append during fill OK");
    }

    /* Count should be capped at capacity */
    TEST_ASSERT(tiku_kits_ds_circlog_count(&log) == 4,
                "count capped at capacity 4");

    /* Newest should be entry 5 (ts=600) */
    rc = tiku_kits_ds_circlog_read_latest(&log, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "read_latest OK");
    TEST_ASSERT(entry.timestamp == 600,
                "newest has timestamp 600");

    /* Oldest should be entry 2 (ts=300) -- entries 0,1 overwritten */
    rc = tiku_kits_ds_circlog_read_at(&log, 3, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "read_at(3) OK");
    TEST_ASSERT(entry.timestamp == 300,
                "oldest surviving has timestamp 300");

    /* Verify entry 0 (ts=100) was overwritten */
    for (i = 0; i < 4; i++) {
        tiku_kits_ds_circlog_read_at(&log, i, &entry);
    }
    /* Entry at index 3 (oldest) should be ts=300, not ts=100 */
    tiku_kits_ds_circlog_read_at(&log, 3, &entry);
    TEST_ASSERT(entry.timestamp != 100,
                "entry with ts=100 was overwritten");

    TEST_GROUP_END("CircLog Wraparound");
}

/*---------------------------------------------------------------------------*/
/* TEST 5: READ-AT ORDER                                                     */
/*---------------------------------------------------------------------------*/

void test_kits_ds_circlog_read_at_order(void)
{
    struct tiku_kits_ds_circlog log;
    struct tiku_kits_ds_circlog_entry entry;
    uint8_t payload[1] = {0xFF};
    int rc;

    TEST_GROUP_BEGIN("CircLog Read-At Order");

    tiku_kits_ds_circlog_init(&log, 8);

    /* Append 5 entries with ascending timestamps */
    tiku_kits_ds_circlog_append(&log, 1, 1, 10, payload, 1);
    tiku_kits_ds_circlog_append(&log, 2, 2, 20, payload, 1);
    tiku_kits_ds_circlog_append(&log, 3, 3, 30, payload, 1);
    tiku_kits_ds_circlog_append(&log, 4, 4, 40, payload, 1);
    tiku_kits_ds_circlog_append(&log, 5, 5, 50, payload, 1);

    /* Index 0 = newest */
    rc = tiku_kits_ds_circlog_read_at(&log, 0, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "read_at(0) OK");
    TEST_ASSERT(entry.timestamp == 50,
                "index 0 is newest (ts=50)");

    /* Index 1 = second newest */
    rc = tiku_kits_ds_circlog_read_at(&log, 1, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "read_at(1) OK");
    TEST_ASSERT(entry.timestamp == 40,
                "index 1 is second newest (ts=40)");

    /* Index 4 = oldest */
    rc = tiku_kits_ds_circlog_read_at(&log, 4, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "read_at(4) OK");
    TEST_ASSERT(entry.timestamp == 10,
                "index N-1 is oldest (ts=10)");

    /* Out of bounds */
    rc = tiku_kits_ds_circlog_read_at(&log, 5, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_BOUNDS,
                "read_at beyond count returns ERR_BOUNDS");

    rc = tiku_kits_ds_circlog_read_at(&log, 100, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_BOUNDS,
                "read_at far beyond returns ERR_BOUNDS");

    TEST_GROUP_END("CircLog Read-At Order");
}

/*---------------------------------------------------------------------------*/
/* TEST 6: SEQUENCE NUMBERS                                                  */
/*---------------------------------------------------------------------------*/

void test_kits_ds_circlog_sequence(void)
{
    struct tiku_kits_ds_circlog log;
    uint8_t payload[1] = {0x00};
    uint32_t seq1, seq2, seq3;

    TEST_GROUP_BEGIN("CircLog Sequence");

    tiku_kits_ds_circlog_init(&log, 8);

    /* Sequence starts at a baseline */
    seq1 = tiku_kits_ds_circlog_sequence(&log);

    /* Append and check monotonic increase */
    tiku_kits_ds_circlog_append(&log, 1, 1, 100, payload, 1);
    seq2 = tiku_kits_ds_circlog_sequence(&log);
    TEST_ASSERT(seq2 > seq1,
                "sequence increases after first append");

    tiku_kits_ds_circlog_append(&log, 2, 2, 200, payload, 1);
    seq3 = tiku_kits_ds_circlog_sequence(&log);
    TEST_ASSERT(seq3 > seq2,
                "sequence increases after second append");

    /* Sequence is strictly monotonic */
    TEST_ASSERT(seq3 > seq1,
                "sequence is strictly monotonic");

    /* Sequence continues after wraparound */
    tiku_kits_ds_circlog_init(&log, 2);
    tiku_kits_ds_circlog_append(&log, 1, 1, 100, payload, 1);
    seq1 = tiku_kits_ds_circlog_sequence(&log);
    tiku_kits_ds_circlog_append(&log, 2, 2, 200, payload, 1);
    seq2 = tiku_kits_ds_circlog_sequence(&log);
    /* This append overwrites the first */
    tiku_kits_ds_circlog_append(&log, 3, 3, 300, payload, 1);
    seq3 = tiku_kits_ds_circlog_sequence(&log);
    TEST_ASSERT(seq3 > seq2 && seq2 > seq1,
                "sequence stays monotonic through wraparound");

    TEST_GROUP_END("CircLog Sequence");
}

/*---------------------------------------------------------------------------*/
/* TEST 7: CLEAR AND RESET                                                   */
/*---------------------------------------------------------------------------*/

void test_kits_ds_circlog_clear_reset(void)
{
    struct tiku_kits_ds_circlog log;
    struct tiku_kits_ds_circlog_entry entry;
    uint8_t payload[2] = {0x11, 0x22};
    int rc;

    TEST_GROUP_BEGIN("CircLog Clear/Reset");

    tiku_kits_ds_circlog_init(&log, 8);
    tiku_kits_ds_circlog_append(&log, 1, 1, 100, payload, 2);
    tiku_kits_ds_circlog_append(&log, 2, 2, 200, payload, 2);
    tiku_kits_ds_circlog_append(&log, 3, 3, 300, payload, 2);

    TEST_ASSERT(tiku_kits_ds_circlog_count(&log) == 3,
                "count is 3 before clear");

    /* Clear the log */
    rc = tiku_kits_ds_circlog_clear(&log);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "clear returns OK");
    TEST_ASSERT(tiku_kits_ds_circlog_count(&log) == 0,
                "count is 0 after clear");

    /* Read from cleared log fails */
    rc = tiku_kits_ds_circlog_read_latest(&log, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_EMPTY,
                "read_latest after clear returns ERR_EMPTY");

    /* Can append after clear */
    rc = tiku_kits_ds_circlog_append(&log, 5, 5, 500, payload, 2);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "append after clear OK");
    TEST_ASSERT(tiku_kits_ds_circlog_count(&log) == 1,
                "count is 1 after post-clear append");

    rc = tiku_kits_ds_circlog_read_latest(&log, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK,
                "read_latest after post-clear append OK");
    TEST_ASSERT(entry.timestamp == 500,
                "post-clear entry has timestamp 500");

    TEST_GROUP_END("CircLog Clear/Reset");
}

/*---------------------------------------------------------------------------*/
/* TEST 8: PAYLOAD DATA INTEGRITY                                            */
/*---------------------------------------------------------------------------*/

void test_kits_ds_circlog_payload(void)
{
    struct tiku_kits_ds_circlog log;
    struct tiku_kits_ds_circlog_entry entry;
    uint8_t payload_full[TIKU_KITS_DS_CIRCLOG_PAYLOAD_SIZE];
    uint8_t payload_small[3] = {0xDE, 0xAD, 0xBE};
    uint16_t i;
    int rc;
    int match;

    TEST_GROUP_BEGIN("CircLog Payload");

    tiku_kits_ds_circlog_init(&log, 8);

    /* Fill payload with known pattern */
    for (i = 0; i < TIKU_KITS_DS_CIRCLOG_PAYLOAD_SIZE; i++) {
        payload_full[i] = (uint8_t)(i + 1);
    }

    /* Append with max payload */
    rc = tiku_kits_ds_circlog_append(&log, 1, 1, 100,
                                      payload_full,
                                      TIKU_KITS_DS_CIRCLOG_PAYLOAD_SIZE);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "append max payload OK");

    /* Read back and verify each byte */
    tiku_kits_ds_circlog_read_latest(&log, &entry);
    TEST_ASSERT(entry.payload_len == TIKU_KITS_DS_CIRCLOG_PAYLOAD_SIZE,
                "payload_len matches max size");

    match = 1;
    for (i = 0; i < TIKU_KITS_DS_CIRCLOG_PAYLOAD_SIZE; i++) {
        if (entry.payload[i] != (uint8_t)(i + 1)) {
            match = 0;
            break;
        }
    }
    TEST_ASSERT(match == 1, "max payload data matches byte-by-byte");

    /* Append with smaller payload */
    rc = tiku_kits_ds_circlog_append(&log, 2, 2, 200,
                                      payload_small, 3);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "append small payload OK");

    tiku_kits_ds_circlog_read_latest(&log, &entry);
    TEST_ASSERT(entry.payload_len == 3,
                "small payload_len is 3");
    TEST_ASSERT(entry.payload[0] == 0xDE, "payload[0] is 0xDE");
    TEST_ASSERT(entry.payload[1] == 0xAD, "payload[1] is 0xAD");
    TEST_ASSERT(entry.payload[2] == 0xBE, "payload[2] is 0xBE");

    /* Zero-length payload */
    rc = tiku_kits_ds_circlog_append(&log, 3, 3, 300, NULL, 0);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "append zero payload OK");
    tiku_kits_ds_circlog_read_latest(&log, &entry);
    TEST_ASSERT(entry.payload_len == 0,
                "zero payload_len is 0");

    /* Oversized payload rejected */
    rc = tiku_kits_ds_circlog_append(&log, 4, 4, 400,
                                      payload_full,
                                      TIKU_KITS_DS_CIRCLOG_PAYLOAD_SIZE + 1);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_PARAM,
                "oversized payload rejected");

    TEST_GROUP_END("CircLog Payload");
}

/*---------------------------------------------------------------------------*/
/* TEST 9: NULL POINTER INPUTS                                               */
/*---------------------------------------------------------------------------*/

void test_kits_ds_circlog_null_inputs(void)
{
    struct tiku_kits_ds_circlog log;
    struct tiku_kits_ds_circlog_entry entry;
    uint8_t payload[2] = {0x01, 0x02};
    int rc;

    TEST_GROUP_BEGIN("CircLog NULL Inputs");

    /* init with NULL */
    rc = tiku_kits_ds_circlog_init(NULL, 8);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "init NULL rejected");

    /* append with NULL log */
    rc = tiku_kits_ds_circlog_append(NULL, 1, 1, 100, payload, 2);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "append NULL log rejected");

    /* read_latest with NULL log */
    rc = tiku_kits_ds_circlog_read_latest(NULL, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "read_latest NULL log rejected");

    /* read_latest with NULL entry_out */
    tiku_kits_ds_circlog_init(&log, 8);
    tiku_kits_ds_circlog_append(&log, 1, 1, 100, payload, 2);
    rc = tiku_kits_ds_circlog_read_latest(&log, NULL);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "read_latest NULL entry_out rejected");

    /* read_at with NULL log */
    rc = tiku_kits_ds_circlog_read_at(NULL, 0, &entry);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "read_at NULL log rejected");

    /* read_at with NULL entry_out */
    rc = tiku_kits_ds_circlog_read_at(&log, 0, NULL);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "read_at NULL entry_out rejected");

    /* clear with NULL */
    rc = tiku_kits_ds_circlog_clear(NULL);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "clear NULL rejected");

    /* count with NULL returns 0 */
    TEST_ASSERT(tiku_kits_ds_circlog_count(NULL) == 0,
                "count NULL returns 0");

    /* sequence with NULL returns 0 */
    TEST_ASSERT(tiku_kits_ds_circlog_sequence(NULL) == 0,
                "sequence NULL returns 0");

    TEST_GROUP_END("CircLog NULL Inputs");
}
