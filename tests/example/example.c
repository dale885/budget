/**
 * Copyright (C) 2020 Dallas Leclerc
 */

#include <stdio.h>

#include <unity.h>

void setUp() {
	printf("setting up test\n");
}

void tearDown() {
	printf("tearing down test\n");
}

void test_simple_test() {
	printf("Running test");
}


int main() {
	UNITY_BEGIN();
	RUN_TEST(test_simple_test);

	return UNITY_END();
}

