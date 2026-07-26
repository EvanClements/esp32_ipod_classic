#include <stdlib.h>

#include "unity.h"
#include "track_utils.h"

void setUp(void) {}
void tearDown(void) {}

static void test_format_sub_hour_as_m_ss(void)
{
    char buf[16];

    TEST_ASSERT_TRUE(track_utils_format_time(0, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_STRING("0:00", buf);

    TEST_ASSERT_TRUE(track_utils_format_time(9, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_STRING("0:09", buf);

    TEST_ASSERT_TRUE(track_utils_format_time(65, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_STRING("1:05", buf);

    TEST_ASSERT_TRUE(track_utils_format_time(3599, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_STRING("59:59", buf);
}

static void test_format_hour_plus_as_h_mm_ss(void)
{
    char buf[16];

    TEST_ASSERT_TRUE(track_utils_format_time(3600, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_STRING("1:00:00", buf);

    TEST_ASSERT_TRUE(track_utils_format_time(3661, buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_STRING("1:01:01", buf);
}

static void test_rejects_invalid_input(void)
{
    char buf[16];

    /* Negative durations are not valid. */
    TEST_ASSERT_FALSE(track_utils_format_time(-1, buf, sizeof(buf)));

    /* Buffer too small to hold the formatted result. */
    TEST_ASSERT_FALSE(track_utils_format_time(60, buf, 2));

    /* NULL destination. */
    TEST_ASSERT_FALSE(track_utils_format_time(60, NULL, sizeof(buf)));
}

void app_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_format_sub_hour_as_m_ss);
    RUN_TEST(test_format_hour_plus_as_h_mm_ss);
    RUN_TEST(test_rejects_invalid_input);
    /* Exit with the failure count so CI fails when a test fails. */
    exit(UNITY_END());
}
