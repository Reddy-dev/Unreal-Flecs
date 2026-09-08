// Parser-only substitute for Flecs' Bake test harness.
//
// Converted test bodies are rewritten to FlecsGeneratedTest assertions, so
// libclang only needs these macros to parse the original source safely.

#pragma once

#define test_assert(...) ((void)0)
#define test_bool(...) ((void)0)
#define test_true(...) ((void)0)
#define test_false(...) ((void)0)
#define test_int(...) ((void)0)
#define test_uint(...) ((void)0)
#define test_flt(...) ((void)0)
#define test_str(...) ((void)0)
#define test_null(...) ((void)0)
#define test_not_null(...) ((void)0)
#define test_ptr(...) ((void)0)
#define test_expect_abort(...) ((void)0)
