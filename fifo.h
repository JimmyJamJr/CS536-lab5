// https://blog.stratifylabs.dev/device/2013-10-02-A-FIFO-Buffer-Implementation/

typedef struct {
    char * buf;
    int head;
    int tail;
    int size;
    int filled; // for bytes filled
} fifo_t;

//This initializes the FIFO structure with the given buffer and size
void fifo_init(fifo_t * f, char * buf, int size){
    f->head = 0;
    f->tail = 0;
    f->filled = 0;
    f->size = size;
    f->buf = buf;
}

//This reads nbytes bytes from the FIFO
//The number of bytes read is returned
int fifo_read(fifo_t * f, void * buf, int nbytes){
     int i;
     char * p = (char *) buf;
     
     for(i=0; i < nbytes; i++){
          if( f->tail != f->head ){ //see if any data is available
               *p++ = f->buf[f->tail];  //grab a byte from the buffer
               f->tail++;  //increment the tail
               if( f->tail == f->size ){  //check for wrap-around
                    f->tail = 0;
               }
          } else {
               f->filled -= i;
               return i; //number of bytes read
          }
     }
     f->filled -= nbytes;
     return nbytes;
}

//This writes up to nbytes bytes to the FIFO
//If the head runs in to the tail, not all bytes are written
//The number of bytes written is returned
int fifo_write(fifo_t * f, const void * buf, int nbytes){
     int i;
     const char * p = (char *) buf;
     
     for(i=0; i < nbytes; i++){
           //first check to see if there is space in the buffer
           if( (f->head + 1 == f->tail) || ( (f->head + 1 == f->size) && (f->tail == 0)) ) {
                    f->filled += i;
                 return i; //no more room
           } else {
               f->buf[f->head] = *p++;
               f->head++;  //increment the head
               if( f->head == f->size ){  //check for wrap-around
                    f->head = 0;
               }
           }
     }
     f->filled += nbytes;
     return nbytes;
}