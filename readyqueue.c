
#include "readyqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// The ready_queue is implemented using a Linked List where each pcb is a node

void ready_queue_init(pcb *head, ready_queue *queue, char *policy){
    queue->policy=strdup(policy);
    queue->head=head;
    queue->first=1;// flag to run the bash script first when using exec #
}
// function helper for RR preemption
void ready_queue_switch(ready_queue *queue){
    pcb *temp = queue->head;
    pcb *cur = queue->head;
    queue->head=cur->next;
    while(cur->next!=NULL){
        cur=cur->next;
    }
    cur->next=temp;
    temp->next=NULL;
}
//Enqueue holds most of the logic for handling different scheduling policies. The idea is to keep things sorted at insertion
void ready_queue_enqueue(pcb *new_block, ready_queue *queue){
 
    pcb *temp = queue->head;
    
    if(temp==NULL){// first element
      
        queue->head=new_block;
        queue->head->next=NULL;
        return;
    }
    if(strcmp(queue->policy,"FCFS")==0||strcmp(queue->policy,"RR")==0||strcmp("RR30",queue->policy)==0){ // Enqueue is AddLast for FCFS-like policies
    while(temp->next!=NULL){
        temp=temp->next;
    }
    temp->next=new_block;}
    else if(strcmp(queue->policy,"SJF")==0){ //SJF and AGING emulate the 'insert' part of insertion sort.
        if(new_block->length<temp->length){
            queue->head=new_block;
            new_block->next=temp;
        }
        pcb *prev;
       
        while(temp!=NULL&&temp->length<=new_block->length){
        
            prev = temp;
            temp= temp->next;
        }
        prev->next=new_block;
        new_block->next=temp;
    }
    else if(strcmp(queue->policy,"AGING")==0){
        pcb *prev=temp;
        if(new_block->length_score<temp->length_score){
            queue->head=new_block;
            new_block->next=temp;
        }
       
        while(temp!=NULL&&temp->length_score<=new_block->length_score){
            prev = temp;
            temp= temp->next;
        }
        prev->next=new_block;
        new_block->next=temp;
    }

}
// Pop returns the head regardless of the policy, since Enqueue ensures the highest priority pcb is at the top.
pcb *ready_queue_pop(ready_queue *queue, int instructions){
    if(queue->first){ // 'Cheat' the queue to run the bash script first when using exec.
        if(queue->head->pid!=0){
        pcb *cur = queue->head->next;
        pcb *prev= queue->head;
        while(cur!=NULL){
            if(cur->pid==0){
                prev->next=cur->next;
                cur->next=queue->head;
                queue->head=cur;
                break;
            }
            prev=cur;
            cur=cur->next;
        }}
        queue->first=0;
    }
    pcb *temp = queue->head;
    if(strcmp("FCFS",queue->policy)==0||strcmp("SJF",queue->policy)==0||temp->currentIndex+instructions-1>=temp->startIndex+temp->length-1){ //If Scheduler will run the pcb to termination, take out of the queue(certain for non-preemptive policies)
    queue->head=temp->next;
    temp->next=NULL;}
    else if(strcmp("RR",queue->policy)==0||strcmp("RR30",queue->policy)==0) { //Preemption for Round Robin.
        if(queue->head->next!=NULL){
            ready_queue_switch(queue);}}
    return temp;
}

int ready_queue_isEmpty(ready_queue *queue){
    if(queue->head){
        return 0;
    }
    free(queue->policy); //free space used to store policy when queue has been emptied
    return 1;
}

//Handles updates and preemption for AGING policy.
void ready_queue_age(ready_queue *queue){
    if(!queue->head){return;}
    pcb *cur = queue->head->next;
    if(cur==NULL)
        return;
    while(cur!=NULL){
        if (cur->length_score>0)
             cur->length_score--;
        cur=cur->next;
    }
    if(queue->head->next->length_score<queue->head->length_score){ //preemption, emulates the 'insert' part of insertion sort.
        pcb *temp = queue->head;
        queue->head=temp->next;
        temp->next=NULL;
        cur = queue->head->next;
        pcb *prev=queue->head;
        while(cur!=NULL &&cur->length_score<temp->length_score){
            prev=cur;
            cur=cur->next;
        }
        prev->next=temp;
        temp->next=cur;
       

    }
}



