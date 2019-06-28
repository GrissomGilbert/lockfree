#ifndef __SAFE_STACK__
#define __SAFE_STACK__
#include <atomic>
#include <memory>

using namespace std;

template <typename T>
class safe_stack
{
public:
  safe_stack() : _head(nullptr){};
  safe_stack &operator=(const safe_stack &other) = delete;
  ~safe_stack(){};

public:
  void push(const T &value)
  {
    shared_ptr<node> new_node(make_shared<node>(value));
    new_node->next = atomic_load(&_head);
    while (!atomic_compare_exchange_weak(&_head, &new_node->next, new_node));
  }

  shared_ptr<T> pop()
  {
    shared_ptr<T> ret_val(nullptr);
    auto old_head = atomic_load(&_head);
    while (old_head && !atomic_compare_exchange_weak(&_head, &old_head, old_head->next));
    return old_head ? old_head->payload : shared_ptr<T>(nullptr);
  }

private:
  struct node
  {
    shared_ptr<T> payload;
    shared_ptr<node> next;

    node() : payload(nullptr), next(nullptr)
    {
    }
    node(T const &data) : payload(make_shared<T>(data)), next(nullptr)
    {
    }
  };
  shared_ptr<node> _head;
};
#endif
