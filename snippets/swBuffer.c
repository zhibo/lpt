#include <stdio.h>
#include <stdlib.h>

#define MAX_AUDIO_SIZE  4096
#define MAX_VIDEO_SIZE  4096 * 5

typedef struct _frame_t {
    unsigned int* start;
    ssize_t       size;
    dlist_t*      list;
} frame_t;

static inline void frame_init(int size)
{
    frame_t frame;
    frame.size = size;
    frame.start = calloc(0, size);
}

void swBufferInit(dlist_t* head)
{
}

void swBufferPush(dlist_t* head, frame_t* frame);
void swBufferPop(dlist_t* head, frame_t* frame);
void swBufferDestroy(dlist_t* head);

int main(int argc, char **argv)
{
    printf("hello, world!\n");
    return 0;
}
