#include<iostream>
#include <pthread.h>
#include<unistd.h>
#include<stddef.h>

pthread_mutex_t global_malloc_lock;

typedef char ALIGN[16];

union header_t{
    struct{
        size_t size; //size of memory block 
        bool is_free; //check if memory block is free
        union header_t *next; //pointer to next header
    }s;
    ALIGN stub; //align the struct to 16 bytes 
}*head, *tail;

header_t *get_free_block(size_t size){
    auto *curr = head;
    while(curr){
        if (curr->s.is_free && curr->s.size>=size)
            return curr;
        curr = curr->s.next;
    }
    return NULL;
}   

void *Malloc(size_t size){
   size_t total_size;
   void *block;
   header_t *header;
   if (!size)
        return NULL;
   pthread_mutex_lock(&global_malloc_lock); //acquire lock before allocating memory to a thread
   header = get_free_block(size);
   if(header){
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock); //give up lock after allocation
        return (void*)(header + 1);
   }
   total_size = sizeof(header_t) + size;
   block= sbrk(total_size);
   if(block == (void*)-1){
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
   }
   header = (header_t*)block;
   header->s.size = size;
   header->s.is_free = 0;
   header->s.next = NULL;
   if(!head){
        head = header;
   }
   if (tail){
        tail->s.next = header;
   }
   tail = header;
   pthread_mutex_unlock(&global_malloc_lock);
   return (void*)(header + 1);
}

void Free(void* block){
    if (!block){
        return;
    }
    pthread_mutex_lock(&global_malloc_lock);
    header_t *header = (header_t*)block -1;
    auto cur_break = sbrk(0);
    if (header->s.size + (char*)block == cur_break){
        if (head == tail)
            head = tail = NULL;
        else{
            auto temp = head;
            while(temp){
                if(temp->s.next == tail){
                    temp->s.next = NULL;
                    tail = temp;
                }
                temp = temp->s.next;
            }
        }
        sbrk(0 - header->s.size + sizeof(header_t));
        pthread_mutex_unlock(&global_malloc_lock);
        std::cout<<"freed from end of the heap"<<std::endl;
        return;
    }
    std::cout<<"freed"<<std::endl;
    header->s.is_free = 1;
    pthread_mutex_unlock(&global_malloc_lock);
}
int main(){
    int *arr = (int*)Malloc(2*sizeof(int));
    arr[0] = 1;
    arr[1] = 2;
    std::cout<<arr[0]<<std::endl<<arr[1]<<std::endl;
    Free((void*)arr);
    return 0;
}

