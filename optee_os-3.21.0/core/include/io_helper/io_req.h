/**
 * @brief io request queue
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#ifndef IO_REQ_H
#define IO_REQ_H
#include <stdint.h>
#include <tee_api_types.h>
#include <tee_api_defines.h>
#include "pta_io_helper.h"

typedef struct {
    io_req_type_t type;         // io req type, defined which io handle function to use
    uint32_t signo;             // a io req is allocated with a signal number
    TEE_Param params[3];        // other params
}io_req_t;

typedef struct io_req_node{
    io_req_t req;
    struct io_req_node *next;
}io_req_node;

typedef struct {
    io_req_node * front;
    io_req_node * rear;
}io_req_queue;

void queue_init(io_req_queue * q);
bool is_empty(io_req_queue * q);
TEE_Result enqueue(io_req_queue *q, io_req_t req);
TEE_Result dequeue(io_req_queue *q, io_req_t * req);

#endif