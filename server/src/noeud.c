#include <stdio.h>
#include <stdlib.h>


struct Node
{
    int index;
    struct Node * next;
};typedef struct Node Node;

Node* createNode(int index){
    Node* n=malloc(sizeof(Node));
    if (!n){
        perror("probleme malloc dans createNode");
        return NULL;
    }
    n->index=index;
    n->next=NULL;
    return n;
}

void addEndNode(Node *t,int index){
    Node * tmp=t;
    while(1){
        if(tmp->next==NULL){
            break;
        }
        tmp=tmp->next;
    }
    tmp->next=createNode(index);
}

Node* addHeadNode(Node *t,int index){
    Node *head=createNode(index);
    head->next=t;
    return head;
}

void freeList(Node *h){
    Node *tmp=h;
    while(h!=NULL){
        h=h->next;
        free(tmp);
        tmp=h;
    }

}


