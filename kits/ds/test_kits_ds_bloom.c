/*
 * Tiku Operating System
 * http://tiku-os.org
 *
 * Authors: Ambuj Varshney <ambuj@tiku-os.org>
 *
 * test_kits_ds_bloom.c - Bloom filter data structure tests
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
#include "tikukits/ds/bloom/tiku_kits_ds_bloom.h"

#include <string.h>

/*---------------------------------------------------------------------------*/
/* TEST 1: INITIALIZATION                                                    */
/*---------------------------------------------------------------------------*/

void test_kits_ds_bloom_init(void)
{
    struct tiku_kits_ds_bloom bf;
    int rc;

    TEST_GROUP_BEGIN("Bloom Init");

    /* Valid init */
    rc = tiku_kits_ds_bloom_init(&bf, 128, 2);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "init bits=128 hashes=2 returns OK");
    TEST_ASSERT(tiku_kits_ds_bloom_count(&bf) == 0,
                "count is 0 after init");

    /* Max bits accepted */
    rc = tiku_kits_ds_bloom_init(&bf,
                                  TIKU_KITS_DS_BLOOM_MAX_BITS,
                                  TIKU_KITS_DS_BLOOM_MAX_HASHES);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "init max bits and hashes OK");

    /* Minimum valid parameters */
    rc = tiku_kits_ds_bloom_init(&bf, 1, 1);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "init bits=1 hashes=1 OK");

    /* Zero bits rejected */
    rc = tiku_kits_ds_bloom_init(&bf, 0, 2);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_PARAM,
                "zero bits rejected");

    /* Zero hashes rejected */
    rc = tiku_kits_ds_bloom_init(&bf, 128, 0);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_PARAM,
                "zero hashes rejected");

    /* Oversized bits rejected */
    rc = tiku_kits_ds_bloom_init(&bf,
                                  TIKU_KITS_DS_BLOOM_MAX_BITS + 1, 2);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_PARAM,
                "oversized bits rejected");

    /* Oversized hashes rejected */
    rc = tiku_kits_ds_bloom_init(&bf, 128,
                                  TIKU_KITS_DS_BLOOM_MAX_HASHES + 1);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_PARAM,
                "oversized hashes rejected");

    TEST_GROUP_END("Bloom Init");
}

/*---------------------------------------------------------------------------*/
/* TEST 2: ADD AND CHECK                                                     */
/*---------------------------------------------------------------------------*/

void test_kits_ds_bloom_add_check(void)
{
    struct tiku_kits_ds_bloom bf;
    int rc;

    TEST_GROUP_BEGIN("Bloom Add/Check");

    tiku_kits_ds_bloom_init(&bf, 256, 3);

    /* Add a key and verify it is present */
    rc = tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"hello",
                                 strlen("hello"));
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "add 'hello' returns OK");

    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"hello",
                                   strlen("hello"));
    TEST_ASSERT(rc == 1, "check 'hello' returns 1 (present)");

    /* Add another key and verify */
    rc = tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"world",
                                 strlen("world"));
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "add 'world' returns OK");

    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"world",
                                   strlen("world"));
    TEST_ASSERT(rc == 1, "check 'world' returns 1 (present)");

    /* Original key still present */
    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"hello",
                                   strlen("hello"));
    TEST_ASSERT(rc == 1, "check 'hello' still returns 1");

    TEST_GROUP_END("Bloom Add/Check");
}

/*---------------------------------------------------------------------------*/
/* TEST 3: CHECK ABSENT KEYS                                                 */
/*---------------------------------------------------------------------------*/

void test_kits_ds_bloom_check_absent(void)
{
    struct tiku_kits_ds_bloom bf;
    int rc;

    TEST_GROUP_BEGIN("Bloom Check Absent");

    tiku_kits_ds_bloom_init(&bf, 256, 3);

    /* Check on empty filter returns 0 */
    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"missing",
                                   strlen("missing"));
    TEST_ASSERT(rc == 0, "check 'missing' on empty filter returns 0");

    /* Add some keys, then check distinct absent keys */
    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"alpha",
                            strlen("alpha"));
    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"bravo",
                            strlen("bravo"));

    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"charlie",
                                   strlen("charlie"));
    TEST_ASSERT(rc == 0, "check 'charlie' returns 0 (absent)");

    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"delta",
                                   strlen("delta"));
    TEST_ASSERT(rc == 0, "check 'delta' returns 0 (absent)");

    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"foxtrot",
                                   strlen("foxtrot"));
    TEST_ASSERT(rc == 0, "check 'foxtrot' returns 0 (absent)");

    TEST_GROUP_END("Bloom Check Absent");
}

/*---------------------------------------------------------------------------*/
/* TEST 4: MULTIPLE KEYS                                                     */
/*---------------------------------------------------------------------------*/

void test_kits_ds_bloom_multiple_keys(void)
{
    struct tiku_kits_ds_bloom bf;
    int rc;

    TEST_GROUP_BEGIN("Bloom Multiple Keys");

    tiku_kits_ds_bloom_init(&bf, 256, 3);

    /* Add several keys */
    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"hello",
                            strlen("hello"));
    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"world",
                            strlen("world"));
    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"test123",
                            strlen("test123"));
    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"tikuos",
                            strlen("tikuos"));
    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"bloom",
                            strlen("bloom"));

    /* Verify all are present */
    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"hello",
                                   strlen("hello"));
    TEST_ASSERT(rc == 1, "check 'hello' present");

    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"world",
                                   strlen("world"));
    TEST_ASSERT(rc == 1, "check 'world' present");

    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"test123",
                                   strlen("test123"));
    TEST_ASSERT(rc == 1, "check 'test123' present");

    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"tikuos",
                                   strlen("tikuos"));
    TEST_ASSERT(rc == 1, "check 'tikuos' present");

    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"bloom",
                                   strlen("bloom"));
    TEST_ASSERT(rc == 1, "check 'bloom' present");

    TEST_ASSERT(tiku_kits_ds_bloom_count(&bf) == 5,
                "count is 5 after adding 5 keys");

    TEST_GROUP_END("Bloom Multiple Keys");
}

/*---------------------------------------------------------------------------*/
/* TEST 5: CLEAR AND RESET                                                   */
/*---------------------------------------------------------------------------*/

void test_kits_ds_bloom_clear_reset(void)
{
    struct tiku_kits_ds_bloom bf;
    int rc;

    TEST_GROUP_BEGIN("Bloom Clear/Reset");

    tiku_kits_ds_bloom_init(&bf, 256, 3);

    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"hello",
                            strlen("hello"));
    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"world",
                            strlen("world"));

    /* Verify keys are present before clear */
    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"hello",
                                   strlen("hello"));
    TEST_ASSERT(rc == 1, "check 'hello' before clear returns 1");

    /* Clear the filter */
    rc = tiku_kits_ds_bloom_clear(&bf);
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "clear returns OK");
    TEST_ASSERT(tiku_kits_ds_bloom_count(&bf) == 0,
                "count is 0 after clear");

    /* Previously added keys no longer present */
    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"hello",
                                   strlen("hello"));
    TEST_ASSERT(rc == 0, "check 'hello' after clear returns 0");

    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"world",
                                   strlen("world"));
    TEST_ASSERT(rc == 0, "check 'world' after clear returns 0");

    /* Can add keys after clear */
    rc = tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"new_key",
                                 strlen("new_key"));
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "add after clear OK");

    rc = tiku_kits_ds_bloom_check(&bf, (const uint8_t *)"new_key",
                                   strlen("new_key"));
    TEST_ASSERT(rc == 1, "check 'new_key' after clear returns 1");

    TEST_GROUP_END("Bloom Clear/Reset");
}

/*---------------------------------------------------------------------------*/
/* TEST 6: COUNT TRACKING                                                    */
/*---------------------------------------------------------------------------*/

void test_kits_ds_bloom_count_tracking(void)
{
    struct tiku_kits_ds_bloom bf;

    TEST_GROUP_BEGIN("Bloom Count Tracking");

    tiku_kits_ds_bloom_init(&bf, 256, 3);

    TEST_ASSERT(tiku_kits_ds_bloom_count(&bf) == 0,
                "initial count is 0");

    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"hello",
                            strlen("hello"));
    TEST_ASSERT(tiku_kits_ds_bloom_count(&bf) == 1,
                "count is 1 after first add");

    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"world",
                            strlen("world"));
    TEST_ASSERT(tiku_kits_ds_bloom_count(&bf) == 2,
                "count is 2 after second add");

    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"test123",
                            strlen("test123"));
    TEST_ASSERT(tiku_kits_ds_bloom_count(&bf) == 3,
                "count is 3 after third add");

    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"tikuos",
                            strlen("tikuos"));
    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"bloom",
                            strlen("bloom"));
    TEST_ASSERT(tiku_kits_ds_bloom_count(&bf) == 5,
                "count is 5 after five adds");

    /* Count resets after clear */
    tiku_kits_ds_bloom_clear(&bf);
    TEST_ASSERT(tiku_kits_ds_bloom_count(&bf) == 0,
                "count is 0 after clear");

    /* Count increments again after re-adding */
    tiku_kits_ds_bloom_add(&bf, (const uint8_t *)"new",
                            strlen("new"));
    TEST_ASSERT(tiku_kits_ds_bloom_count(&bf) == 1,
                "count is 1 after add post-clear");

    TEST_GROUP_END("Bloom Count Tracking");
}

/*---------------------------------------------------------------------------*/
/* TEST 7: DIFFERENT HASH COUNTS                                             */
/*---------------------------------------------------------------------------*/

void test_kits_ds_bloom_different_hashes(void)
{
    struct tiku_kits_ds_bloom bf1;
    struct tiku_kits_ds_bloom bf3;
    int rc;

    TEST_GROUP_BEGIN("Bloom Different Hashes");

    /* Filter with 1 hash function */
    tiku_kits_ds_bloom_init(&bf1, 256, 1);
    tiku_kits_ds_bloom_add(&bf1, (const uint8_t *)"hello",
                            strlen("hello"));

    rc = tiku_kits_ds_bloom_check(&bf1, (const uint8_t *)"hello",
                                   strlen("hello"));
    TEST_ASSERT(rc == 1,
                "1-hash: check 'hello' returns 1 (present)");

    rc = tiku_kits_ds_bloom_check(&bf1, (const uint8_t *)"missing",
                                   strlen("missing"));
    TEST_ASSERT(rc == 0,
                "1-hash: check 'missing' returns 0 (absent)");

    /* Filter with 3 hash functions */
    tiku_kits_ds_bloom_init(&bf3, 256, 3);
    tiku_kits_ds_bloom_add(&bf3, (const uint8_t *)"hello",
                            strlen("hello"));

    rc = tiku_kits_ds_bloom_check(&bf3, (const uint8_t *)"hello",
                                   strlen("hello"));
    TEST_ASSERT(rc == 1,
                "3-hash: check 'hello' returns 1 (present)");

    rc = tiku_kits_ds_bloom_check(&bf3, (const uint8_t *)"missing",
                                   strlen("missing"));
    TEST_ASSERT(rc == 0,
                "3-hash: check 'missing' returns 0 (absent)");

    /* Both filters detect added key */
    tiku_kits_ds_bloom_add(&bf1, (const uint8_t *)"test123",
                            strlen("test123"));
    tiku_kits_ds_bloom_add(&bf3, (const uint8_t *)"test123",
                            strlen("test123"));

    rc = tiku_kits_ds_bloom_check(&bf1, (const uint8_t *)"test123",
                                   strlen("test123"));
    TEST_ASSERT(rc == 1,
                "1-hash: check 'test123' returns 1");

    rc = tiku_kits_ds_bloom_check(&bf3, (const uint8_t *)"test123",
                                   strlen("test123"));
    TEST_ASSERT(rc == 1,
                "3-hash: check 'test123' returns 1");

    TEST_GROUP_END("Bloom Different Hashes");
}

/*---------------------------------------------------------------------------*/
/* TEST 8: BINARY (NON-STRING) KEYS                                          */
/*---------------------------------------------------------------------------*/

void test_kits_ds_bloom_binary_keys(void)
{
    struct tiku_kits_ds_bloom bf;
    int rc;
    uint8_t key_a[] = { 0x00, 0xFF, 0x80, 0x01 };
    uint8_t key_b[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    uint8_t key_c[] = { 0x01, 0x02, 0x03 };
    uint8_t key_absent[] = { 0xAA, 0xBB, 0xCC, 0xDD };

    TEST_GROUP_BEGIN("Bloom Binary Keys");

    tiku_kits_ds_bloom_init(&bf, 256, 3);

    /* Add binary keys */
    rc = tiku_kits_ds_bloom_add(&bf, key_a, sizeof(key_a));
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "add binary key_a OK");

    rc = tiku_kits_ds_bloom_add(&bf, key_b, sizeof(key_b));
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "add binary key_b OK");

    rc = tiku_kits_ds_bloom_add(&bf, key_c, sizeof(key_c));
    TEST_ASSERT(rc == TIKU_KITS_DS_OK, "add binary key_c OK");

    /* Check binary keys are present */
    rc = tiku_kits_ds_bloom_check(&bf, key_a, sizeof(key_a));
    TEST_ASSERT(rc == 1, "check binary key_a returns 1");

    rc = tiku_kits_ds_bloom_check(&bf, key_b, sizeof(key_b));
    TEST_ASSERT(rc == 1, "check binary key_b returns 1");

    rc = tiku_kits_ds_bloom_check(&bf, key_c, sizeof(key_c));
    TEST_ASSERT(rc == 1, "check binary key_c returns 1");

    /* Check absent binary key */
    rc = tiku_kits_ds_bloom_check(&bf, key_absent,
                                   sizeof(key_absent));
    TEST_ASSERT(rc == 0, "check absent binary key returns 0");

    TEST_ASSERT(tiku_kits_ds_bloom_count(&bf) == 3,
                "count is 3 after adding 3 binary keys");

    TEST_GROUP_END("Bloom Binary Keys");
}

/*---------------------------------------------------------------------------*/
/* TEST 9: NULL POINTER INPUTS                                               */
/*---------------------------------------------------------------------------*/

void test_kits_ds_bloom_null_inputs(void)
{
    struct tiku_kits_ds_bloom bf;
    int rc;

    TEST_GROUP_BEGIN("Bloom NULL Inputs");

    /* init with NULL bloom */
    rc = tiku_kits_ds_bloom_init(NULL, 128, 2);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "init NULL bloom rejected");

    /* add with NULL bloom */
    rc = tiku_kits_ds_bloom_add(NULL, (const uint8_t *)"hello",
                                 strlen("hello"));
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "add NULL bloom rejected");

    /* add with NULL key */
    tiku_kits_ds_bloom_init(&bf, 128, 2);
    rc = tiku_kits_ds_bloom_add(&bf, NULL, 5);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "add NULL key rejected");

    /* check with NULL bloom */
    rc = tiku_kits_ds_bloom_check(NULL, (const uint8_t *)"hello",
                                   strlen("hello"));
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "check NULL bloom rejected");

    /* check with NULL key */
    rc = tiku_kits_ds_bloom_check(&bf, NULL, 5);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "check NULL key rejected");

    /* clear with NULL */
    rc = tiku_kits_ds_bloom_clear(NULL);
    TEST_ASSERT(rc == TIKU_KITS_DS_ERR_NULL,
                "clear NULL rejected");

    /* count with NULL returns 0 */
    TEST_ASSERT(tiku_kits_ds_bloom_count(NULL) == 0,
                "count NULL returns 0");

    TEST_GROUP_END("Bloom NULL Inputs");
}
