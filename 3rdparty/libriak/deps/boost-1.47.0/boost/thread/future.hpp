//  (C) Copyright 2008-10 Anthony Williams 
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_THREAD_FUTURE_HPP
#define BOOST_THREAD_FUTURE_HPP
#include <stdexcept>
#include <boost/thread/detail/move.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/mpl/if.hpp>
#include <boost/config.hpp>
#include <boost/throw_exception.hpp>
#include <algorithm>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/scoped_array.hpp>
#include <boost/utility/enable_if.hpp>
#include <list>
#include <boost/next_prior.hpp>
#include <vector>

namespace riakboost{} namespace boost = riakboost; namespace riakboost{
    class future_uninitialized:
        public std::logic_error
    {
    public:
        future_uninitialized():
            std::logic_error("Future Uninitialized")
        {}
    };
    class broken_promise:
        public std::logic_error
    {
    public:
        broken_promise():
            std::logic_error("Broken promise")
        {}
    };
    class future_already_retrieved:
        public std::logic_error
    {
    public:
        future_already_retrieved():
            std::logic_error("Future already retrieved")
        {}
    };
    class promise_already_satisfied:
        public std::logic_error
    {
    public:
        promise_already_satisfied():
            std::logic_error("Promise already satisfied")
        {}
    };

    class task_already_started:
        public std::logic_error
    {
    public:
        task_already_started():
            std::logic_error("Task already started")
        {}
    };

    class task_moved:
        public std::logic_error
    {
    public:
        task_moved():
            std::logic_error("Task moved")
        {}
    };

    namespace future_state
    {
        enum state { uninitialized, waiting, ready, moved };
    }

    namespace detail
    {
        struct future_object_base
        {
            riakboost::exception_ptr exception;
            bool done;
            riakboost::mutex mutex;
            riakboost::condition_variable waiters;
            typedef std::list<riakboost::condition_variable_any*> waiter_list;
            waiter_list external_waiters;
            riakboost::function<void()> callback;

            future_object_base():
                done(false)
            {}
            virtual ~future_object_base()
            {}

            waiter_list::iterator register_external_waiter(riakboost::condition_variable_any& cv)
            {
                riakboost::unique_lock<riakboost::mutex> lock(mutex);
                do_callback(lock);
                return external_waiters.insert(external_waiters.end(),&cv);
            }
            
            void remove_external_waiter(waiter_list::iterator it)
            {
                riakboost::lock_guard<riakboost::mutex> lock(mutex);
                external_waiters.erase(it);
            }

            void mark_finished_internal()
            {
                done=true;
                waiters.notify_all();
                for(waiter_list::const_iterator it=external_waiters.begin(),
                        end=external_waiters.end();it!=end;++it)
                {
                    (*it)->notify_all();
                }
            }

            struct relocker
            {
                riakboost::unique_lock<riakboost::mutex>& lock;
                
                relocker(riakboost::unique_lock<riakboost::mutex>& lock_):
                    lock(lock_)
                {
                    lock.unlock();
                }
                ~relocker()
                {
                    lock.lock();
                }
            private:
                relocker& operator=(relocker const&);
            };

            void do_callback(riakboost::unique_lock<riakboost::mutex>& lock)
            {
                if(callback && !done)
                {
                    riakboost::function<void()> local_callback=callback;
                    relocker relock(lock);
                    local_callback();
                }
            }
            

            void wait(bool rethrow=true)
            {
                riakboost::unique_lock<riakboost::mutex> lock(mutex);
                do_callback(lock);
                while(!done)
                {
                    waiters.wait(lock);
                }
                if(rethrow && exception)
                {
                    riakboost::rethrow_exception(exception);
                }
            }

            bool timed_wait_until(riakboost::system_time const& target_time)
            {
                riakboost::unique_lock<riakboost::mutex> lock(mutex);
                do_callback(lock);
                while(!done)
                {
                    bool const success=waiters.timed_wait(lock,target_time);
                    if(!success && !done)
                    {
                        return false;
                    }
                }
                return true;
            }
            
            void mark_exceptional_finish_internal(riakboost::exception_ptr const& e)
            {
                exception=e;
                mark_finished_internal();
            }
            void mark_exceptional_finish()
            {
                riakboost::lock_guard<riakboost::mutex> lock(mutex);
                mark_exceptional_finish_internal(riakboost::current_exception());
            }

            bool has_value()
            {
                riakboost::lock_guard<riakboost::mutex> lock(mutex);
                return done && !exception;
            }
            bool has_exception()
            {
                riakboost::lock_guard<riakboost::mutex> lock(mutex);
                return done && exception;
            }

            template<typename F,typename U>
            void set_wait_callback(F f,U* u)
            {
                callback=riakboost::bind(f,riakboost::ref(*u));
            }
            
        private:
            future_object_base(future_object_base const&);
            future_object_base& operator=(future_object_base const&);
        };

        template<typename T>
        struct future_traits
        {
            typedef riakboost::scoped_ptr<T> storage_type;
#ifndef BOOST_NO_RVALUE_REFERENCES
            typedef T const& source_reference_type;
            struct dummy;
            typedef typename riakboost::mpl::if_<riakboost::is_fundamental<T>,dummy&,T&&>::type rvalue_source_type;
            typedef typename riakboost::mpl::if_<riakboost::is_fundamental<T>,T,T&&>::type move_dest_type;
#else
            typedef T& source_reference_type;
            typedef typename riakboost::mpl::if_<riakboost::is_convertible<T&,riakboost::detail::thread_move_t<T> >,riakboost::detail::thread_move_t<T>,T const&>::type rvalue_source_type;
            typedef typename riakboost::mpl::if_<riakboost::is_convertible<T&,riakboost::detail::thread_move_t<T> >,riakboost::detail::thread_move_t<T>,T>::type move_dest_type;
#endif

            static void init(storage_type& storage,source_reference_type t)
            {
                storage.reset(new T(t));
            }
            
            static void init(storage_type& storage,rvalue_source_type t)
            {
                storage.reset(new T(static_cast<rvalue_source_type>(t)));
            }

            static void cleanup(storage_type& storage)
            {
                storage.reset();
            }
        };
        
        template<typename T>
        struct future_traits<T&>
        {
            typedef T* storage_type;
            typedef T& source_reference_type;
            struct rvalue_source_type
            {};
            typedef T& move_dest_type;

            static void init(storage_type& storage,T& t)
            {
                storage=&t;
            }

            static void cleanup(storage_type& storage)
            {
                storage=0;
            }
        };

        template<>
        struct future_traits<void>
        {
            typedef bool storage_type;
            typedef void move_dest_type;

            static void init(storage_type& storage)
            {
                storage=true;
            }

            static void cleanup(storage_type& storage)
            {
                storage=false;
            }

        };

        template<typename T>
        struct future_object:
            detail::future_object_base
        {
            typedef typename future_traits<T>::storage_type storage_type;
            typedef typename future_traits<T>::source_reference_type source_reference_type;
            typedef typename future_traits<T>::rvalue_source_type rvalue_source_type;
            typedef typename future_traits<T>::move_dest_type move_dest_type;
            
            storage_type result;

            future_object():
                result(0)
            {}

            void mark_finished_with_result_internal(source_reference_type result_)
            {
                future_traits<T>::init(result,result_);
                mark_finished_internal();
            }
            void mark_finished_with_result_internal(rvalue_source_type result_)
            {
                future_traits<T>::init(result,static_cast<rvalue_source_type>(result_));
                mark_finished_internal();
            }

            void mark_finished_with_result(source_reference_type result_)
            {
                riakboost::lock_guard<riakboost::mutex> lock(mutex);
                mark_finished_with_result_internal(result_);
            }
            void mark_finished_with_result(rvalue_source_type result_)
            {
                riakboost::lock_guard<riakboost::mutex> lock(mutex);
                mark_finished_with_result_internal(result_);
            }

            move_dest_type get()
            {
                wait();
                return static_cast<move_dest_type>(*result);
            }

            future_state::state get_state()
            {
                riakboost::lock_guard<riakboost::mutex> guard(mutex);
                if(!done)
                {
                    return future_state::waiting;
                }
                else
                {
                    return future_state::ready;
                }
            }

        private:
            future_object(future_object const&);
            future_object& operator=(future_object const&);
        };

        template<>
        struct future_object<void>:
            detail::future_object_base
        {
            future_object()
            {}

            void mark_finished_with_result_internal()
            {
                mark_finished_internal();
            }

            void mark_finished_with_result()
            {
                riakboost::lock_guard<riakboost::mutex> lock(mutex);
                mark_finished_with_result_internal();
            }

            void get()
            {
                wait();
            }
            
            future_state::state get_state()
            {
                riakboost::lock_guard<riakboost::mutex> guard(mutex);
                if(!done)
                {
                    return future_state::waiting;
                }
                else
                {
                    return future_state::ready;
                }
            }

        private:
            future_object(future_object const&);
            future_object& operator=(future_object const&);
        };

        class future_waiter
        {
            struct registered_waiter;
            typedef std::vector<registered_waiter>::size_type count_type;
            
            struct registered_waiter
            {
                riakboost::shared_ptr<detail::future_object_base> future;
                detail::future_object_base::waiter_list::iterator wait_iterator;
                count_type index;

                registered_waiter(riakboost::shared_ptr<detail::future_object_base> const& future_,
                                  detail::future_object_base::waiter_list::iterator wait_iterator_,
                                  count_type index_):
                    future(future_),wait_iterator(wait_iterator_),index(index_)
                {}

            };
            
            struct all_futures_lock
            {
                count_type count;
                riakboost::scoped_array<riakboost::unique_lock<riakboost::mutex> > locks;
                
                all_futures_lock(std::vector<registered_waiter>& futures):
                    count(futures.size()),locks(new riakboost::unique_lock<riakboost::mutex>[count])
                {
                    for(count_type i=0;i<count;++i)
                    {
                        locks[i]=riakboost::unique_lock<riakboost::mutex>(futures[i].future->mutex);
                    }
                }
                
                void lock()
                {
                    riakboost::lock(locks.get(),locks.get()+count);
                }
                
                void unlock()
                {
                    for(count_type i=0;i<count;++i)
                    {
                        locks[i].unlock();
                    }
                }
            };
            
            riakboost::condition_variable_any cv;
            std::vector<registered_waiter> futures;
            count_type future_count;
            
        public:
            future_waiter():
                future_count(0)
            {}
            
            template<typename F>
            void add(F& f)
            {
                if(f.future)
                {
                    futures.push_back(registered_waiter(f.future,f.future->register_external_waiter(cv),future_count));
                }
                ++future_count;
            }

            count_type wait()
            {
                all_futures_lock lk(futures);
                for(;;)
                {
                    for(count_type i=0;i<futures.size();++i)
                    {
                        if(futures[i].future->done)
                        {
                            return futures[i].index;
                        }
                    }
                    cv.wait(lk);
                }
            }
            
            ~future_waiter()
            {
                for(count_type i=0;i<futures.size();++i)
                {
                    futures[i].future->remove_external_waiter(futures[i].wait_iterator);
                }
            }
            
        };
        
    }

    template <typename R>
    class unique_future;

    template <typename R>
    class shared_future;

    template<typename T>
    struct is_future_type
    {
        BOOST_STATIC_CONSTANT(bool, value=false);
    };
    
    template<typename T>
    struct is_future_type<unique_future<T> >
    {
        BOOST_STATIC_CONSTANT(bool, value=true);
    };
    
    template<typename T>
    struct is_future_type<shared_future<T> >
    {
        BOOST_STATIC_CONSTANT(bool, value=true);
    };

    template<typename Iterator>
    typename riakboost::disable_if<is_future_type<Iterator>,void>::type wait_for_all(Iterator begin,Iterator end)
    {
        for(Iterator current=begin;current!=end;++current)
        {
            current->wait();
        }
    }

    template<typename F1,typename F2>
    typename riakboost::enable_if<is_future_type<F1>,void>::type wait_for_all(F1& f1,F2& f2)
    {
        f1.wait();
        f2.wait();
    }

    template<typename F1,typename F2,typename F3>
    void wait_for_all(F1& f1,F2& f2,F3& f3)
    {
        f1.wait();
        f2.wait();
        f3.wait();
    }
    
    template<typename F1,typename F2,typename F3,typename F4>
    void wait_for_all(F1& f1,F2& f2,F3& f3,F4& f4)
    {
        f1.wait();
        f2.wait();
        f3.wait();
        f4.wait();
    }

    template<typename F1,typename F2,typename F3,typename F4,typename F5>
    void wait_for_all(F1& f1,F2& f2,F3& f3,F4& f4,F5& f5)
    {
        f1.wait();
        f2.wait();
        f3.wait();
        f4.wait();
        f5.wait();
    }

    template<typename Iterator>
    typename riakboost::disable_if<is_future_type<Iterator>,Iterator>::type wait_for_any(Iterator begin,Iterator end)
    {
        if(begin==end)
            return end;
        
        detail::future_waiter waiter;
        for(Iterator current=begin;current!=end;++current)
        {
            waiter.add(*current);
        }
        return riakboost::next(begin,waiter.wait());
    }

    template<typename F1,typename F2>
    typename riakboost::enable_if<is_future_type<F1>,unsigned>::type wait_for_any(F1& f1,F2& f2)
    {
        detail::future_waiter waiter;
        waiter.add(f1);
        waiter.add(f2);
        return waiter.wait();
    }

    template<typename F1,typename F2,typename F3>
    unsigned wait_for_any(F1& f1,F2& f2,F3& f3)
    {
        detail::future_waiter waiter;
        waiter.add(f1);
        waiter.add(f2);
        waiter.add(f3);
        return waiter.wait();
    }
    
    template<typename F1,typename F2,typename F3,typename F4>
    unsigned wait_for_any(F1& f1,F2& f2,F3& f3,F4& f4)
    {
        detail::future_waiter waiter;
        waiter.add(f1);
        waiter.add(f2);
        waiter.add(f3);
        waiter.add(f4);
        return waiter.wait();
    }

    template<typename F1,typename F2,typename F3,typename F4,typename F5>
    unsigned wait_for_any(F1& f1,F2& f2,F3& f3,F4& f4,F5& f5)
    {
        detail::future_waiter waiter;
        waiter.add(f1);
        waiter.add(f2);
        waiter.add(f3);
        waiter.add(f4);
        waiter.add(f5);
        return waiter.wait();
    }
    
    template <typename R>
    class promise;

    template <typename R>
    class packaged_task;

    template <typename R>
    class unique_future
    {
        unique_future(unique_future & rhs);// = delete;
        unique_future& operator=(unique_future& rhs);// = delete;

        typedef riakboost::shared_ptr<detail::future_object<R> > future_ptr;
        
        future_ptr future;

        friend class shared_future<R>;
        friend class promise<R>;
        friend class packaged_task<R>;
        friend class detail::future_waiter;

        typedef typename detail::future_traits<R>::move_dest_type move_dest_type;

        unique_future(future_ptr future_):
            future(future_)
        {}

    public:
        typedef future_state::state state;

        unique_future()
        {}
       
        ~unique_future()
        {}

#ifndef BOOST_NO_RVALUE_REFERENCES
        unique_future(unique_future && other)
        {
            future.swap(other.future);
        }
        unique_future& operator=(unique_future && other)
        {
            future=other.future;
            other.future.reset();
            return *this;
        }
#else
        unique_future(riakboost::detail::thread_move_t<unique_future> other):
            future(other->future)
        {
            other->future.reset();
        }

        unique_future& operator=(riakboost::detail::thread_move_t<unique_future> other)
        {
            future=other->future;
            other->future.reset();
            return *this;
        }

        operator riakboost::detail::thread_move_t<unique_future>()
        {
            return riakboost::detail::thread_move_t<unique_future>(*this);
        }
#endif

        void swap(unique_future& other)
        {
            future.swap(other.future);
        }

        // retrieving the value
        move_dest_type get()
        {
            if(!future)
            {
                riakboost::throw_exception(future_uninitialized());
            }

            return future->get();
        }
        
        // functions to check state, and wait for ready
        state get_state() const
        {
            if(!future)
            {
                return future_state::uninitialized;
            }
            return future->get_state();
        }
        

        bool is_ready() const
        {
            return get_state()==future_state::ready;
        }
        
        bool has_exception() const
        {
            return future && future->has_exception();
        }
        
        bool has_value() const
        {
            return future && future->has_value();
        }
        
        void wait() const
        {
            if(!future)
            {
                riakboost::throw_exception(future_uninitialized());
            }
            future->wait(false);
        }
        
        template<typename Duration>
        bool timed_wait(Duration const& rel_time) const
        {
            return timed_wait_until(riakboost::get_system_time()+rel_time);
        }
        
        bool timed_wait_until(riakboost::system_time const& abs_time) const
        {
            if(!future)
            {
                riakboost::throw_exception(future_uninitialized());
            }
            return future->timed_wait_until(abs_time);
        }
        
    };

    template <typename R>
    class shared_future
    {
        typedef riakboost::shared_ptr<detail::future_object<R> > future_ptr;
        
        future_ptr future;

//         shared_future(const unique_future<R>& other);
//         shared_future& operator=(const unique_future<R>& other);

        friend class detail::future_waiter;
        friend class promise<R>;
        friend class packaged_task<R>;
        
        shared_future(future_ptr future_):
            future(future_)
        {}

    public:
        shared_future(shared_future const& other):
            future(other.future)
        {}

        typedef future_state::state state;

        shared_future()
        {}

        ~shared_future()
        {}

        shared_future& operator=(shared_future const& other)
        {
            future=other.future;
            return *this;
        }
#ifndef BOOST_NO_RVALUE_REFERENCES
        shared_future(shared_future && other)
        {
            future.swap(other.future);
        }
        shared_future(unique_future<R> && other)
        {
            future.swap(other.future);
        }
        shared_future& operator=(shared_future && other)
        {
            future.swap(other.future);
            other.future.reset();
            return *this;
        }
        shared_future& operator=(unique_future<R> && other)
        {
            future.swap(other.future);
            other.future.reset();
            return *this;
        }
#else            
        shared_future(riakboost::detail::thread_move_t<shared_future> other):
            future(other->future)
        {
            other->future.reset();
        }
//         shared_future(const unique_future<R> &) = delete;
        shared_future(riakboost::detail::thread_move_t<unique_future<R> > other):
            future(other->future)
        {
            other->future.reset();
        }
        shared_future& operator=(riakboost::detail::thread_move_t<shared_future> other)
        {
            future.swap(other->future);
            other->future.reset();
            return *this;
        }
        shared_future& operator=(riakboost::detail::thread_move_t<unique_future<R> > other)
        {
            future.swap(other->future);
            other->future.reset();
            return *this;
        }

        operator riakboost::detail::thread_move_t<shared_future>()
        {
            return riakboost::detail::thread_move_t<shared_future>(*this);
        }

#endif

        void swap(shared_future& other)
        {
            future.swap(other.future);
        }

        // retrieving the value
        R get()
        {
            if(!future)
            {
                riakboost::throw_exception(future_uninitialized());
            }

            return future->get();
        }
        
        // functions to check state, and wait for ready
        state get_state() const
        {
            if(!future)
            {
                return future_state::uninitialized;
            }
            return future->get_state();
        }
        

        bool is_ready() const
        {
            return get_state()==future_state::ready;
        }
        
        bool has_exception() const
        {
            return future && future->has_exception();
        }
        
        bool has_value() const
        {
            return future && future->has_value();
        }

        void wait() const
        {
            if(!future)
            {
                riakboost::throw_exception(future_uninitialized());
            }
            future->wait(false);
        }
        
        template<typename Duration>
        bool timed_wait(Duration const& rel_time) const
        {
            return timed_wait_until(riakboost::get_system_time()+rel_time);
        }
        
        bool timed_wait_until(riakboost::system_time const& abs_time) const
        {
            if(!future)
            {
                riakboost::throw_exception(future_uninitialized());
            }
            return future->timed_wait_until(abs_time);
        }
        
    };

    template <typename R>
    class promise
    {
        typedef riakboost::shared_ptr<detail::future_object<R> > future_ptr;
        
        future_ptr future;
        bool future_obtained;
        
        promise(promise & rhs);// = delete;
        promise & operator=(promise & rhs);// = delete;

        void lazy_init()
        {
            if(!atomic_load(&future))
            {
                future_ptr blank;
                atomic_compare_exchange(&future,&blank,future_ptr(new detail::future_object<R>));
            }
        }
        
    public:
//         template <class Allocator> explicit promise(Allocator a);

        promise():
            future(),future_obtained(false)
        {}
        
        ~promise()
        {
            if(future)
            {
                riakboost::lock_guard<riakboost::mutex> lock(future->mutex);

                if(!future->done)
                {
                    future->mark_exceptional_finish_internal(riakboost::copy_exception(broken_promise()));
                }
            }
        }

        // Assignment
#ifndef BOOST_NO_RVALUE_REFERENCES
        promise(promise && rhs):
            future_obtained(rhs.future_obtained)
        {
            future.swap(rhs.future);
            rhs.future_obtained=false;
        }
        promise & operator=(promise&& rhs)
        {
            future.swap(rhs.future);
            future_obtained=rhs.future_obtained;
            rhs.future.reset();
            rhs.future_obtained=false;
            return *this;
        }
#else
        promise(riakboost::detail::thread_move_t<promise> rhs):
            future(rhs->future),future_obtained(rhs->future_obtained)
        {
            rhs->future.reset();
            rhs->future_obtained=false;
        }
        promise & operator=(riakboost::detail::thread_move_t<promise> rhs)
        {
            future=rhs->future;
            future_obtained=rhs->future_obtained;
            rhs->future.reset();
            rhs->future_obtained=false;
            return *this;
        }

        operator riakboost::detail::thread_move_t<promise>()
        {
            return riakboost::detail::thread_move_t<promise>(*this);
        }
#endif   
        
        void swap(promise& other)
        {
            future.swap(other.future);
            std::swap(future_obtained,other.future_obtained);
        }

        // Result retrieval
        unique_future<R> get_future()
        {
            lazy_init();
            if(future_obtained)
            {
                riakboost::throw_exception(future_already_retrieved());
            }
            future_obtained=true;
            return unique_future<R>(future);
        }

        void set_value(typename detail::future_traits<R>::source_reference_type r)
        {
            lazy_init();
            riakboost::lock_guard<riakboost::mutex> lock(future->mutex);
            if(future->done)
            {
                riakboost::throw_exception(promise_already_satisfied());
            }
            future->mark_finished_with_result_internal(r);
        }

//         void set_value(R && r);
        void set_value(typename detail::future_traits<R>::rvalue_source_type r)
        {
            lazy_init();
            riakboost::lock_guard<riakboost::mutex> lock(future->mutex);
            if(future->done)
            {
                riakboost::throw_exception(promise_already_satisfied());
            }
            future->mark_finished_with_result_internal(static_cast<typename detail::future_traits<R>::rvalue_source_type>(r));
        }

        void set_exception(riakboost::exception_ptr p)
        {
            lazy_init();
            riakboost::lock_guard<riakboost::mutex> lock(future->mutex);
            if(future->done)
            {
                riakboost::throw_exception(promise_already_satisfied());
            }
            future->mark_exceptional_finish_internal(p);
        }

        template<typename F>
        void set_wait_callback(F f)
        {
            lazy_init();
            future->set_wait_callback(f,this);
        }
        
    };

    template <>
    class promise<void>
    {
        typedef riakboost::shared_ptr<detail::future_object<void> > future_ptr;
        
        future_ptr future;
        bool future_obtained;
        
        promise(promise & rhs);// = delete;
        promise & operator=(promise & rhs);// = delete;

        void lazy_init()
        {
            if(!atomic_load(&future))
            {
                future_ptr blank;
                atomic_compare_exchange(&future,&blank,future_ptr(new detail::future_object<void>));
            }
        }
    public:
//         template <class Allocator> explicit promise(Allocator a);

        promise():
            future(),future_obtained(false)
        {}
        
        ~promise()
        {
            if(future)
            {
                riakboost::lock_guard<riakboost::mutex> lock(future->mutex);

                if(!future->done)
                {
                    future->mark_exceptional_finish_internal(riakboost::copy_exception(broken_promise()));
                }
            }
        }

        // Assignment
#ifndef BOOST_NO_RVALUE_REFERENCES
        promise(promise && rhs):
            future_obtained(rhs.future_obtained)
        {
            future.swap(rhs.future);
            rhs.future_obtained=false;
        }
        promise & operator=(promise&& rhs)
        {
            future.swap(rhs.future);
            future_obtained=rhs.future_obtained;
            rhs.future.reset();
            rhs.future_obtained=false;
            return *this;
        }
#else
        promise(riakboost::detail::thread_move_t<promise> rhs):
            future(rhs->future),future_obtained(rhs->future_obtained)
        {
            rhs->future.reset();
            rhs->future_obtained=false;
        }
        promise & operator=(riakboost::detail::thread_move_t<promise> rhs)
        {
            future=rhs->future;
            future_obtained=rhs->future_obtained;
            rhs->future.reset();
            rhs->future_obtained=false;
            return *this;
        }

        operator riakboost::detail::thread_move_t<promise>()
        {
            return riakboost::detail::thread_move_t<promise>(*this);
        }
#endif
        
        void swap(promise& other)
        {
            future.swap(other.future);
            std::swap(future_obtained,other.future_obtained);
        }

        // Result retrieval
        unique_future<void> get_future()
        {
            lazy_init();
            
            if(future_obtained)
            {
                riakboost::throw_exception(future_already_retrieved());
            }
            future_obtained=true;
            return unique_future<void>(future);
        }

        void set_value()
        {
            lazy_init();
            riakboost::lock_guard<riakboost::mutex> lock(future->mutex);
            if(future->done)
            {
                riakboost::throw_exception(promise_already_satisfied());
            }
            future->mark_finished_with_result_internal();
        }

        void set_exception(riakboost::exception_ptr p)
        {
            lazy_init();
            riakboost::lock_guard<riakboost::mutex> lock(future->mutex);
            if(future->done)
            {
                riakboost::throw_exception(promise_already_satisfied());
            }
            future->mark_exceptional_finish_internal(p);
        }

        template<typename F>
        void set_wait_callback(F f)
        {
            lazy_init();
            future->set_wait_callback(f,this);
        }
        
    };

    namespace detail
    {
        template<typename R>
        struct task_base:
            detail::future_object<R>
        {
            bool started;

            task_base():
                started(false)
            {}

            void run()
            {
                {
                    riakboost::lock_guard<riakboost::mutex> lk(this->mutex);
                    if(started)
                    {
                        riakboost::throw_exception(task_already_started());
                    }
                    started=true;
                }
                do_run();
            }

            void owner_destroyed()
            {
                riakboost::lock_guard<riakboost::mutex> lk(this->mutex);
                if(!started)
                {
                    started=true;
                    this->mark_exceptional_finish_internal(riakboost::copy_exception(riakboost::broken_promise()));
                }
            }
            
            
            virtual void do_run()=0;
        };
        
        
        template<typename R,typename F>
        struct task_object:
            task_base<R>
        {
            F f;
            task_object(F const& f_):
                f(f_)
            {}
            task_object(riakboost::detail::thread_move_t<F> f_):
                f(f_)
            {}
            
            void do_run()
            {
                try
                {
                    this->mark_finished_with_result(f());
                }
                catch(...)
                {
                    this->mark_exceptional_finish();
                }
            }
        };

        template<typename F>
        struct task_object<void,F>:
            task_base<void>
        {
            F f;
            task_object(F const& f_):
                f(f_)
            {}
            task_object(riakboost::detail::thread_move_t<F> f_):
                f(f_)
            {}
            
            void do_run()
            {
                try
                {
                    f();
                    this->mark_finished_with_result();
                }
                catch(...)
                {
                    this->mark_exceptional_finish();
                }
            }
        };

    }
    

    template<typename R>
    class packaged_task
    {
        riakboost::shared_ptr<detail::task_base<R> > task;
        bool future_obtained;

        packaged_task(packaged_task&);// = delete;
        packaged_task& operator=(packaged_task&);// = delete;
        
    public:
        packaged_task():
            future_obtained(false)
        {}
        
        // construction and destruction
        template <class F>
        explicit packaged_task(F const& f):
            task(new detail::task_object<R,F>(f)),future_obtained(false)
        {}
        explicit packaged_task(R(*f)()):
            task(new detail::task_object<R,R(*)()>(f)),future_obtained(false)
        {}
        
        template <class F>
        explicit packaged_task(riakboost::detail::thread_move_t<F> f):
            task(new detail::task_object<R,F>(f)),future_obtained(false)
        {}

//         template <class F, class Allocator>
//         explicit packaged_task(F const& f, Allocator a);
//         template <class F, class Allocator>
//         explicit packaged_task(F&& f, Allocator a);


        ~packaged_task()
        {
            if(task)
            {
                task->owner_destroyed();
            }
        }

        // assignment
#ifndef BOOST_NO_RVALUE_REFERENCES
        packaged_task(packaged_task&& other):
            future_obtained(other.future_obtained)
        {
            task.swap(other.task);
            other.future_obtained=false;
        }
        packaged_task& operator=(packaged_task&& other)
        {
            packaged_task temp(static_cast<packaged_task&&>(other));
            swap(temp);
            return *this;
        }
#else
        packaged_task(riakboost::detail::thread_move_t<packaged_task> other):
            future_obtained(other->future_obtained)
        {
            task.swap(other->task);
            other->future_obtained=false;
        }
        packaged_task& operator=(riakboost::detail::thread_move_t<packaged_task> other)
        {
            packaged_task temp(other);
            swap(temp);
            return *this;
        }
        operator riakboost::detail::thread_move_t<packaged_task>()
        {
            return riakboost::detail::thread_move_t<packaged_task>(*this);
        }
#endif

        void swap(packaged_task& other)
        {
            task.swap(other.task);
            std::swap(future_obtained,other.future_obtained);
        }

        // result retrieval
        unique_future<R> get_future()
        {
            if(!task)
            {
                riakboost::throw_exception(task_moved());
            }
            else if(!future_obtained)
            {
                future_obtained=true;
                return unique_future<R>(task);
            }
            else
            {
                riakboost::throw_exception(future_already_retrieved());
            }
        }
        

        // execution
        void operator()()
        {
            if(!task)
            {
                riakboost::throw_exception(task_moved());
            }
            task->run();
        }

        template<typename F>
        void set_wait_callback(F f)
        {
            task->set_wait_callback(f,this);
        }
        
    };

}


#endif
