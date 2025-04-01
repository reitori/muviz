#include "CircularBuffer.h"

namespace viz{
    template <typename T>
    CircularBuffer<T>::CircularBuffer(size_t size){
        buffer.resize(size);
        head = 0;
        tail = 0;
        m_size = size;
        full = false;
    }

    template<typename T>
    void CircularBuffer<T>::push(T data){
        buffer[head] = data;

        head = (head + 1) % m_size;
        if(full)
            tail = (tail + 1) % m_size;
        
        full = (head == tail);
    }

    template<typename T>
    std::unique_ptr<T> CircularBuffer<T>::pop(){
        if(checkEmpty()){
            return nullptr;
        }

        std::unique_ptr<T> tailPointer = std::make_unique<T>(std::move(buffer[tail]));
        tail = (tail + 1) % m_size;
        full = false;
        
        return tailPointer;
    }

    template<typename T>
    std::vector<T> CircularBuffer<T>::getBufferData() const{
        std::vector<T> tempVec;
        if(isEmpty())
            return tempVec;

        int i = 0;
        tempVec.reserve(buffer.size());
        do{
            tempVec[i] = buffer[(tail + i) % m_size];
            i++;
        }while((tail + i) % m_size != head)

        return 
    }

    template<typename T>
    std::unique_ptr<std::vector<T>> CircularBuffer<T>::releaseData(){
        std::unique_ptr<std::vector<T>> movedBuffer = std::make_unique<std::vector<T>>(std::move(buffer));
        buffer.resize(m_size);
        head = 0;
        tail = 0;
        full = false;
        return movedBuffer;
    }
}