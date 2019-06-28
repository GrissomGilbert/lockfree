#ifndef __SAFE_QUEUE__
#define __SAFE_QUEUE__
#include <atomic>
#include <memory>

using namespace std;

template <typename T>
class safe_queue
{
public:
    safe_queue() : _tail(new node()), queue_count(0)
    {
        _head = _tail;
    };
    ~safe_queue()
    {
        while (get_count() && dequeue());
    };

public:
    void enqueue(const T &val)
    {
        shared_ptr<node> new_node = make_shared<node>(val);
        shared_ptr<node> tail;
        bool res = false;
        do
        {
            shared_ptr<node> expected(nullptr);
            tail = atomic_load(&_tail);
            res = atomic_compare_exchange_strong(&tail->next, &expected, new_node);
            if (!res)
                atomic_compare_exchange_strong(&_tail, &tail, tail->next);
        } while (!res);
        atomic_compare_exchange_strong(&_tail, &tail, new_node);
        queue_count.fetch_add(1);
    };

    void enqueue( const T &&val )
    {
        shared_ptr<node> new_node = make_shared<node>(val);
        shared_ptr<node> tail;
        bool res = false;
        do
        {
            shared_ptr<node> expected(nullptr);
            tail = atomic_load(&_tail);
            res = atomic_compare_exchange_strong(&tail->next, &expected, new_node);
            if (!res)
                atomic_compare_exchange_strong(&_tail, &tail, tail->next);
        } while (!res);
        atomic_compare_exchange_strong(&_tail, &tail, new_node);
        queue_count.fetch_add(1);
    };

    shared_ptr<T> dequeue()
    {
        shared_ptr<T> res(nullptr);
        bool ret = false;
        do
        {
            auto old_head = atomic_load(&_head);
            if (!atomic_load(&old_head->next))
            {
                return res;
            }
            ret = atomic_compare_exchange_strong(&_head, &old_head, old_head->next);
            if (ret)
            {
                res = old_head->next->payload;
                break;
            }
        } while (true);
        queue_count.fetch_sub(1);
        return res;
    };

    template <typename FUNCTION>
    shared_ptr<T> find_if(FUNCTION func)
    {
        auto next = atomic_load(&_head->next);
        do
        {
            if (next && func(next->payload))
            {
                return next->payload;
            }
        } while (next = atomic_load(&next->next));
        return shared_ptr<T>(nullptr);
    };

    uint get_count()
    {
        return queue_count.load();
    };

private:
    struct node
    {
        shared_ptr<T> payload;
        shared_ptr<node> next;
        node() : payload(nullptr), next(nullptr){};
        node(T const &val) : payload(make_shared<T>(val)), next(nullptr){};
    };

    shared_ptr<node> _head;
    shared_ptr<node> _tail;
    atomic<uint> queue_count;
};
#endif