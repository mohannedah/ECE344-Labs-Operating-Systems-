#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H
#include <bits/stdc++.h>
template <typename T>
class DynamicArray
{
    int capacity;
    T *arr;

public:
    DynamicArray();
    int size = 0;
    void push_array(T thread);
    T get(int idx);
    T *get_begin_pointer();
};

using namespace std;

template <typename T>
DynamicArray<T>::DynamicArray()
{
    capacity = 2;
    arr = (T *)malloc(sizeof(T) * capacity);
}

template <typename T>
T DynamicArray<T>::get(int idx)
{
    if (idx >= size)
    {
        cerr << "Index out of bounds" << endl;
        exit(-1);
    }
    return arr[idx];
}

template <typename T>
void DynamicArray<T>::push_array(T thread)
{
    size += 1;
    if (size > capacity)
    {
        capacity *= 2;
        arr = (T *)realloc(arr, sizeof(T) * capacity);
    }
    arr[size - 1] = thread;
}

template <typename T>
T *DynamicArray<T>::get_begin_pointer()
{
    return arr;
}

#endif