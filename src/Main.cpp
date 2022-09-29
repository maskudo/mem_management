#include<iostream>

typedef char ALIGN[16];

union header_t{
    struct{
        size_t size; //size of memory block 
        bool is_free; //check if memory block is free
        union header_t *next; //pointer to next header
    }s;
    ALIGN stub; //align the struct to 16 bytes 
}
header_t *head, *tail;

void malloc(size_t size){
   size_t total_size;
   void *block;
   header_t *header;
   if (!size)
        return NULL;
   pthread_mutex_lock(&global_malloc_lock); //acquire lock before allocation memory to a thread
   header = get_free_block(size);
   if(header){
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock); //give up lock after allocation
        return *(void)(header + 1);
   }
       
}
//check  if there's a free block
header_t *get_free_block(size_t size){
    auto *curr = head;
    while(curr){
        if (curr->s.is_free && curr->s.size>=size){
            return curr;
        curr = curr->s.next;
    }
    return NULL;
}
int main(){
    std::cout<<(void*)-1<<std::endl;
    return 0;
}

