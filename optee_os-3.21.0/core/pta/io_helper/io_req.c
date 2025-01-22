/**
 * @brief io request queue
 * @author XXXXXXXX <XXXXX@XXXXXX>
 */
#include "io_helper/io_req.h"
#include <tee_api.h>
#include <stdlib.h>

void queue_init(io_req_queue * q){
    q->front=NULL;
    q->rear=NULL;
}

bool is_empty(io_req_queue * q){
    return q->front==NULL;
}

TEE_Result enqueue(io_req_queue * q, io_req_t req){
    io_req_node * new_node = (io_req_node *)malloc(sizeof(io_req_node));
    if(new_node==NULL){
        DMSG("Memory alloc failed");
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    new_node->req=req;
    new_node->next=NULL;
    if(is_empty(q)){
        q->front=new_node;
        q->rear=new_node;
    }else{
        q->rear->next=new_node;
        q->rear=new_node;
    }
    return TEE_SUCCESS;
}

TEE_Result dequeue(io_req_queue * q, io_req_t * req){
    if(is_empty(q)){
        DMSG("Queue is empty");
        return TEE_ERROR_NO_DATA;
    }
    io_req_node * tmp= q->front;
    *req=tmp->req;
    q->front=q->front->next;
    if(q->front==NULL) q->rear=NULL;
    free(tmp);
    return TEE_SUCCESS;
}

