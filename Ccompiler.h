//
// Created by dxy on 2020/7/23.
//

#ifndef CCOMPILER_CCOMPILER_H
#define CCOMPILER_CCOMPILER_H

#define kBlockSize 4096

enum LexType {
    ID, KEYWORD, OP, SEP, CONST, UNUSED
};

typedef struct {
    enum LexType type;
    char *name;
} LexUnit;

typedef struct {
    int line;
    int column;
} Loc;

Loc g_loc;

/**
 * Get struct address from its member address.
 *
 * @param ptr address of the member
 * @param type struct name
 * @param member member name in the struct
 */
#define GET_STRUCT(ptr, type, member) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/// @typedef ListNode a circular linked List
typedef struct ListNode {
    struct ListNode *next;
} ListNode;

/**
 * @typedef Queue 'head' points to the head in the circular linked List and
 * 'tail' points to the node before 'head'.
 */
typedef struct {
    ListNode *head;
    ListNode *tail;
} Queue;

static inline ListNode *List() {
    ListNode *head = malloc(sizeof(ListNode));
    head->next = head;
    return head;
}

static inline void ListAdd(ListNode *prev, ListNode *new_node) {
    ListNode *next = prev->next;
    prev->next = new_node;
    new_node->next = next;
}

static inline ListNode *ListRemove(ListNode *prev) {
    ListNode *del = prev->next;
    prev->next = del->next;
    del->next = NULL;
    return del;
}

static inline int IsListEmpty(ListNode *head) {
    if (head->next == head) {
        return 1;
    }
    return 0;
}

static inline void DestructList(ListNode *head) {
    free(head);
}

static inline Queue *queue() {
    ListNode *head = List();
    Queue *q = malloc(sizeof(Queue));
    q->head = head;
    q->tail = head;
    return q;
}

static inline void Enqueue(Queue *queue, ListNode *new_node) {
    ListAdd(queue->tail, new_node);
    queue->tail = new_node;
}

static inline ListNode *Dequeue(Queue *queue) {
    if (IsListEmpty(queue->head) == 1) {
        // empty queue
        return NULL;
    }

    if (queue->head->next == queue->tail) {
        queue->tail = queue->head;
    }
    return ListRemove(queue->head);
}

static inline ListNode *QueueGetHead(Queue *queue) {
    if (IsListEmpty(queue->head) == 1) {
        return NULL;
    }
    return queue->head->next;
}

static inline void DestructQueue(Queue *queue) {
    DestructList(queue->head);
    free(queue);
}

/// @typedef ErrInfo record detailed error info
typedef struct {
    char *error;
    Loc errLoc;
    ListNode *next;
} ErrInfo;

Queue *err_queue;

void InitLex(int fd);

LexUnit *AnalyseLexical(int fd);

#endif //CCOMPILER_CCOMPILER_H