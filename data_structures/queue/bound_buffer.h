#ifndef DOCUMENTS_BOUND_BUFFER_H
#define DOCUMENTS_BOUND_BUFFER_H
#include <mutex>
#include "queue.h"
#include "pthread.h"
namespace data_structures {
template<typename ValueType>
class BoundBuffer {

private:
    const int max_size_;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    Queue<ValueType> *myQueue;


public:
    int mySize = 0;
    explicit BoundBuffer(int max_size) : max_size_(max_size) {
        myQueue = new Queue<ValueType>();
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);


    }

    int size() { return myQueue->size(); }

    void addLast(const ValueType& value) {

        pthread_mutex_lock(&mutex);
            while(myQueue->size() == max_size_-1) {
            pthread_cond_wait(&cond, &mutex);
            }
              myQueue->addLast(value);
              mySize++;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
      }


    ValueType removeFirst() {
        pthread_mutex_lock(&mutex);
        while(myQueue->size() == 0) {
            pthread_cond_wait(&cond, &mutex);
        }
                ValueType value = myQueue->removeFirst();
                mySize--;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
                return value;
    }


};

} // namespace data_structures

#endif //DOCUMENTS_BOUND_BUFFER_H
