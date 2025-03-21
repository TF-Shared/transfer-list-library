/*
 * Copyright The Transfer List Library Contributors
 *
 * SPDX-License-Identifier: MIT OR GPL-2.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>

#include "test.h"
#include "transfer_list.h"
#include "unity.h"

void *buffer = NULL;

uint8_t byte_sum(const char *ptr, size_t len)
{
	uint8_t sum;

	for (size_t i = 0; i < len; i++) {
		sum += ptr[i];
	}

	return sum;
}

void test_add()
{
	struct transfer_list_header *tl;
	struct transfer_list_entry *te;

	TEST_ASSERT(tl = transfer_list_init((void *)buffer, TL_SIZE));

	TEST_ASSERT(te = transfer_list_add(tl, test_tag, sizeof(test_data),
					   &test_data));
	TEST_ASSERT_EQUAL(0, byte_sum((char *)tl, tl->max_size));
	TEST_ASSERT(*(int *)transfer_list_entry_data(te) == test_data);

	/* Try to add a TE larger greater than allocated TL space */
	TEST_ASSERT_NULL(te = transfer_list_add(tl, 2, TL_SIZE, &test_data));
	TEST_ASSERT_EQUAL(0, byte_sum((char *)tl, tl->max_size));
	TEST_ASSERT_NULL(transfer_list_find(tl, 0x2));

	unsigned int tags[4] = { TAG_GENERIC_START, TAG_GENERIC_END,
				 TAG_NON_STANDARD_START, TAG_NON_STANDARD_END };

	for (size_t i = 0; i < 4; i++) {
		TEST_ASSERT(te = transfer_list_add(tl, tags[i],
						   sizeof(test_data),
						   &test_data));
		TEST_ASSERT_EQUAL(0, byte_sum((char *)tl, tl->max_size));
		TEST_ASSERT(te = transfer_list_find(tl, tags[i]));
		TEST_ASSERT(*(int *)transfer_list_entry_data(te) == test_data);
	}

	transfer_list_dump(tl);
	/* Add some out of bound tags. */
	TEST_ASSERT_NULL(
		transfer_list_add(tl, 1 << 24, sizeof(test_data), &test_data));

	TEST_ASSERT_NULL(
		transfer_list_add(tl, -1, sizeof(test_data), &test_data));
}

void test_add_with_align()
{
	struct transfer_list_header *tl =
		transfer_list_init(buffer, TL_MAX_SIZE);
	struct transfer_list_entry *te;

	unsigned int test_id = 1;
	const unsigned int entry_size = 0xff;
	int *data;

	TEST_ASSERT(tl->size == tl->hdr_size);

	/*
	 * When a new TE with a larger alignment requirement than already exists
	 * appears, the TE should be added and TL alignement updated.
	 */
	for (char align = 0; align < (1 << 4); align++, test_id++) {
		TEST_ASSERT(
			te = transfer_list_add_with_align(
				tl, test_id, entry_size, &test_data, align));
		TEST_ASSERT(tl->alignment >= align);
		TEST_ASSERT(te = transfer_list_find(tl, test_id));
		TEST_ASSERT(data = transfer_list_entry_data(te));
		TEST_ASSERT_FALSE((uintptr_t)data % (1 << align));
		TEST_ASSERT_EQUAL(*(int *)data, test_data);
	}
}

void test_rem()
{
	struct transfer_list_header *tl = transfer_list_init(buffer, TL_SIZE);
	struct transfer_list_entry *te;

	TEST_ASSERT_EQUAL(tl->size, tl->hdr_size);

	/* Add a test TE, check the TL has been updated with its contents. */
	TEST_ASSERT(
		transfer_list_add(tl, test_tag, tl->max_size / 8, &test_data));
	TEST_ASSERT(te = transfer_list_find(tl, test_tag));
	TEST_ASSERT(byte_sum((void *)tl, tl->size) == 0);

	/* Remove the TE and make sure it isn't present in the TL. */
	TEST_ASSERT_TRUE(transfer_list_rem(tl, te));
	TEST_ASSERT(byte_sum((void *)tl, tl->size) == 0);
	TEST_ASSERT_NULL(transfer_list_find(tl, test_tag));
}

void setUp(void)
{
	buffer = malloc(TL_MAX_SIZE);
}

void tearDown(void)
{
	free(buffer);
	buffer = NULL;
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_add);
	RUN_TEST(test_add_with_align);
	return UNITY_END();
}
