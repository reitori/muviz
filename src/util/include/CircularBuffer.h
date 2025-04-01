#ifndef EVENTBUFFER_H
#define EVENTBUFFER_H

#include "core/header.h"

namespace viz{
    template <typename T>
    class CircularBuffer{
        public:
            CircularBuffer() = delete;
            CircularBuffer(size_t size);

            void push(T data);
            std::unique_ptr<T> pop();
        
            bool isFull() const {return full;}
            bool isEmpty() const { return (!full && (head == tail)); }

            std::vector<T> getBufferData() const;
            std::unique_ptr<std::vector<T>> releaseData();

        private:
            bool checkFull() const;
            bool checkEmpty() const;

            bool full;
            size_t m_size;
            size_t head, tail;

            std::vector<T> buffer;
    };
}



#endif