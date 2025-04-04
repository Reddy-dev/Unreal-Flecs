/**
 * @file datastructures/stack_allocator.h
 * @brief Stack allocator.
 */

#ifndef FLECS_STACK_ALLOCATOR_H
#define FLECS_STACK_ALLOCATOR_H

/** Stack allocator for quick allocation of small temporary values */

typedef struct ecs_stack_page_t {
    void *data;
    struct ecs_stack_page_t *next;
    int16_t sp;
    uint32_t id;
} ecs_stack_page_t;

typedef struct ecs_stack_cursor_t {
    struct ecs_stack_cursor_t *prev;
    struct ecs_stack_page_t *page;
    int16_t sp;
    bool is_free;
#ifdef FLECS_DEBUG
    struct ecs_stack_t *owner;
#endif
} ecs_stack_cursor_t;

typedef struct ecs_stack_t {
    ecs_stack_page_t *first;
    ecs_stack_page_t *tail_page;
    ecs_stack_cursor_t *tail_cursor;
#ifdef FLECS_DEBUG
    int32_t cursor_count;
#endif
} ecs_stack_t;

#define FLECS_STACK_PAGE_OFFSET ECS_ALIGN(ECS_SIZEOF(ecs_stack_page_t), 16)
#define FLECS_STACK_PAGE_SIZE (1024 - FLECS_STACK_PAGE_OFFSET)

FLECS_DBG_API
void flecs_stack_init(
    ecs_stack_t *stack);

FLECS_DBG_API
void flecs_stack_fini(
    ecs_stack_t *stack);

FLECS_DBG_API
void* flecs_stack_alloc(
    ecs_stack_t *stack, 
    ecs_size_t size,
    ecs_size_t align);

#define flecs_stack_alloc_t(stack, T)\
    flecs_stack_alloc(stack, ECS_SIZEOF(T), ECS_ALIGNOF(T))

#define flecs_stack_alloc_n(stack, T, count)\
    flecs_stack_alloc(stack, ECS_SIZEOF(T) * (count), ECS_ALIGNOF(T))

FLECS_DBG_API
void* flecs_stack_calloc(
    ecs_stack_t *stack, 
    ecs_size_t size,
    ecs_size_t align);

#define flecs_stack_calloc_t(stack, T)\
    flecs_stack_calloc(stack, ECS_SIZEOF(T), ECS_ALIGNOF(T))

#define flecs_stack_calloc_n(stack, T, count)\
    flecs_stack_calloc(stack, ECS_SIZEOF(T) * (count), ECS_ALIGNOF(T))

FLECS_DBG_API
void flecs_stack_free(
    void *ptr,
    ecs_size_t size);

#define flecs_stack_free_t(ptr, T)\
    flecs_stack_free(ptr, ECS_SIZEOF(T))

#define flecs_stack_free_n(ptr, T, count)\
    flecs_stack_free(ptr, ECS_SIZEOF(T) * (count))

void flecs_stack_reset(
    ecs_stack_t *stack);

FLECS_DBG_API
ecs_stack_cursor_t* flecs_stack_get_cursor(
    ecs_stack_t *stack);

FLECS_DBG_API
void flecs_stack_restore_cursor(
    ecs_stack_t *stack,
    ecs_stack_cursor_t *cursor);

#endif
