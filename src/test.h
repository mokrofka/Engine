#pragma once
#include "lib.h"

#define test_equal(actual, expected)                    \
  if (actual != expected) {                             \
    Error("Expected %i, but got %i", expected, actual); \
  }

#define test_not_equal(actual, expected)                \
  if (actual == expected) {                             \
    Error("Expected %i != %i, but equal", expected, actual); \
  }

#define test_true(actual)              \
  if (actual != true) {                \
    Error("Expected true, but false"); \
  }

#define test_false(actual)            \
  if (actual != false) {              \
    Error("Expected false, but true") \
  }

KAPI void test();
