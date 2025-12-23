#ifndef EVENTBUFFER_H
#define EVENTBUFFER_H

#include "core/header.h"

namespace viz {
    template <typename T>
    class CircularBuffer {
    public:
        CircularBuffer() = delete;
        CircularBuffer(size_t size) {
            buffer.resize(size);
            head = 0;
            tail = 0;
            m_capacity = size;
            m_size = 0;
            full = false;
        }

        void push(const T& data) {
            buffer[head] = data;

            if (full) {
                tail = (tail + 1) % m_capacity; 
            } else {
                m_size++;
            }

            head = (head + 1) % m_capacity;
            full = (m_size == m_capacity);
        }

        void push(T&& data){
            buffer[head] = std::move(data);

            if (full) {
                tail = (tail + 1) % m_capacity; 
            } else {
                m_size++;
            }

            head = (head + 1) % m_capacity;
            full = (m_size == m_capacity);
        }

        std::unique_ptr<T> pop() {
            if (checkEmpty()) {
                return nullptr;
            }

            std::unique_ptr<T> tailPointer = std::make_unique<T>(std::move(buffer[tail]));
            tail = (tail + 1) % m_capacity;
            full = false;
            
            return tailPointer;
        }

        T& operator[](size_t index) {
            if (index >= m_size) {
                throw std::out_of_range("Circular Buffer indexed into: out of range");
            }

            return buffer[(tail + index) % m_capacity];
        }
    
        bool isFull() const { return full; }
        bool isEmpty() const { return (!full && (head == tail)); }
        
        inline size_t getSize() const { return m_size; }

        std::vector<T> getBufferData() const {
            std::vector<T> tempVec;
            tempVec.reserve(m_size);
            for (size_t i = 0; i < m_size; ++i) {
                tempVec.push_back(buffer[(tail + i) % m_capacity]);
            }
            return tempVec;
        }

        std::unique_ptr<std::vector<T>> releaseData() {
            std::unique_ptr<std::vector<T>> movedBuffer = std::make_unique<std::vector<T>>(std::move(buffer));
            buffer.resize(m_capacity);
            head = 0;
            tail = 0;
            full = false;
            return movedBuffer;
        }

    private:
        bool checkFull() const;
        bool checkEmpty() const;

        bool full;
        size_t m_capacity, m_size;
        size_t head, tail;

        std::vector<T> buffer;
    };
}

#endif