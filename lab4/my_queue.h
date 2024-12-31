#ifndef QUEUE_H
#define QUEUE_H
#include <bits/stdc++.h>
using namespace std;
template <typename T>
struct MyNode
{
public:
    MyNode(T data)
    {
        this->data = data;
        this->next = nullptr;
        this->prev = nullptr;
    }
    MyNode *next, *prev;
    T data;
};

template <typename T>
class MyQueue
{
private:
    MyNode<T> *head;
    MyNode<T> *tail;
    T dummy_value;
    int queue_size = 0;

public:
    MyQueue(T dummy_value);
    ~MyQueue();
    void add_queue(T item);
    T pop_queue(int id);
    T pop_front_queue();
    T top_queue();
};

template <typename T>
MyQueue<T>::MyQueue(T dummy_value)
{
    head = new MyNode<T>(dummy_value);
    tail = new MyNode<T>(dummy_value);
    this->dummy_value = dummy_value;
    head->next = tail;
    tail->prev = head;
};

template <typename T>
void MyQueue<T>::add_queue(T thread)
{
    MyNode<T> *node = new MyNode(thread);
    MyNode<T> *prev_node = tail->prev;
    prev_node->next = node;
    node->prev = prev_node;
    node->next = tail;
    tail->prev = node;
    queue_size += 1;
    // cout << thread << endl;
};

template <typename T>
T MyQueue<T>::pop_front_queue()
{
    MyNode<T> *front_node = head->next;
    if (front_node == tail)
    {
        return this->dummy_value;
    }
    pop_queue(front_node->data->id);
    return front_node->data;
}

template <typename T>
T MyQueue<T>::pop_queue(int id)
{
    MyNode<T> *curr_node = head->next;
    while (curr_node != tail && curr_node->data->id != id)
    {
        curr_node = curr_node->next;
    }
    if (curr_node == tail)
    {
        return nullptr;
    }
    MyNode<T> *prev_node = curr_node->prev;
    prev_node->next = curr_node->next;
    curr_node->next->prev = prev_node;
    T thread = curr_node->data;
    free(curr_node);
    return thread;
};

template <typename T>
T MyQueue<T>::top_queue()
{
    MyNode<T> *front_node = head->next;
    if (front_node == tail)
    {
        cerr << "Queue is empty!" << endl;
        exit(-1);
    }
    return front_node->data;
}

template <typename T>
MyQueue<T>::~MyQueue()
{
    MyNode<T> *curr_node = head;
    while (curr_node != nullptr)
    {
        MyNode<T> *next_node = curr_node->next;
        delete curr_node;
        curr_node = next_node;
    }
};

#endif
