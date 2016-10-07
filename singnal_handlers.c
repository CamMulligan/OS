#include <sys/signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

struct bgList {
    pid_t pid;
    struct bgList* next;
    struct bgList* prev;
    
};

pid_t removeBgNode(pid_t pid, struct bgList *head){
    struct bgList* temp = head;
    while (temp != NULL){
        if (temp->pid == pid){
            temp->prev->next=temp->next;
            temp->next->prev = temp->prev;
            return temp->pid;
        } else {
            temp = temp->next;
        }
    }
    return(-1);
} 

int addBgNode(struct bgList *node, struct bgList *head){
    struct bgList* temp = head;
    if (head == NULL){
        head = node;
        return(1);
    } else {
        while (temp->next != NULL){ //get to last node
            temp = temp->next;
        }
        temp->next = node;
        node->prev = temp;
    }
}

struct bgList* newBgNode(pid_t pid){
    struct bgList *new = (struct bgList*) malloc (sizeof(struct bgList));
    new->pid = pid;
    return(new);
}




