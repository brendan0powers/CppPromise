#ifndef PROMISE_H
#define PROMISE_H

#include <functional>
#include <memory>

namespace CppPromise {
template <typename T>
struct lambda_traits : public lambda_traits<decltype(&T::operator())>
{

};

template <typename Class, typename Return, typename ...Args>
struct lambda_traits<Return(Class::*)(Args...) const>
{
    typedef Return returnType;

    enum {
        numArgs = sizeof...(Args)
    };

    template <size_t argNum>
    struct args
    {
        typedef typename std::tuple_element<argNum, std::tuple<Args...>>::type type;
    };
};

template <typename Fn, int argNum = 0>
using lambda_traits_arg_t = typename lambda_traits<Fn>::template args<argNum>::type;

template <typename T>
struct strip_promise
{
    using type = T;
};

template <typename T, template<typename> class O>
struct strip_promise<O <T> >
{
    using type = typename strip_promise<T>::type;
};

enum class PromiseState
{
    Unresolved,
    Resolved,
    Rejected
};

template <typename T>
struct PromiseSharedState
{
    T value;
    std::exception_ptr error;
    PromiseState state = PromiseState::Unresolved;
    std::vector<std::function<void(T)>> vecThens;
    std::vector<std::function<void(std::exception_ptr)>> vecFails;
    std::vector<std::function<void(void)>> vecAlways;

    void callResolve() {
        for(const auto & func : vecThens) {
            func(value);
        }
    }

    void callReject() {
        for(const auto & func : vecFails) {
            func(error);
        }
    }

    void calAlways() {
        for(const auto & func : vecAlways) {
            func();
        }
    }

    void copyValue(const PromiseSharedState &other) {
        value = other.value;
    }
};

template <>
struct PromiseSharedState<void>
{
    PromiseState state = PromiseState::Unresolved;
    std::exception_ptr error;
    std::vector<std::function<void(void)>> vecThens;
    std::vector<std::function<void(std::exception_ptr)>> vecFails;
    std::vector<std::function<void(void)>> vecAlways;

    void callResolve() {
        for(const auto & func : vecThens) {
            func();
        }
    }

    void callReject() {
        for(const auto & func : vecFails) {
            func(error);
        }
    }

    void calAlways() {
        for(const auto & func : vecAlways) {
            func();
        }
    }

    void copyValue(const PromiseSharedState &) {

    }
};

template <typename T>
class Promise
{
public:
    Promise<T>()
        : m_pShared(new PromiseSharedState<T>())
    {

    }

    Promise<T> &operator = (const Promise<T> &other)
    {
        m_pShared->copyValue(*other.m_pShared);
        m_pShared->error = other.m_pShared->error;
        if(other.m_pShared->state == PromiseState::Resolved)
        {
            m_pShared->state = PromiseState::Resolved;
            m_pShared->callResolve();
            m_pShared->calAlways();
        }
        if(other.m_pShared->state == PromiseState::Rejected)
        {
            m_pShared->state = PromiseState::Rejected;
            m_pShared->callReject();
            m_pShared->calAlways();
        }
        else
        {
            m_pShared = other.m_pShared;
        }
        return *this;
    }

    template <typename Fn,
              typename std::enable_if<std::is_same<void, typename lambda_traits<Fn>::returnType>::value &&
                                        (lambda_traits<Fn>::numArgs == 0), int>::type = 0>
    Promise<void> then(Fn func)
    {
        Promise<void> promise;
        auto innerFunc = [promise, func](T) mutable {
            try
            {
                func();
                promise = resolve();
            }
            catch(...)
            {
                promise = reject<typename strip_promise<typename lambda_traits<Fn>::returnType>::type>(std::current_exception());
            }
        };
        m_pShared->vecThens.push_back(innerFunc);

        try
        {
            if(m_pShared->state == PromiseState::Resolved)
                innerFunc();
        }
        catch(...)
        {
            promise = reject<typename strip_promise<typename lambda_traits<Fn>::returnType>::type>(std::current_exception());
        }

        return promise;
    }

    template <typename Fn,
              typename std::enable_if<std::is_same<void, typename lambda_traits<Fn>::returnType>::value &&
                                        (lambda_traits<Fn>::numArgs == 1), int>::type = 0>
    Promise<void> then(Fn func)
    {
        static_assert((lambda_traits<Fn>::numArgs == 0)
                      || std::is_same<T, lambda_traits<Fn>::args<0>::type>::value, "Err");

        Promise<void> promise;
        auto innerFunc = [promise, func](T arg) mutable {
            try
            {
                func(arg);
                promise = resolve();
            }
            catch(...)
            {
                promise = reject<typename strip_promise<typename lambda_traits<Fn>::returnType>::type>(std::current_exception());
            }
        };
        m_pShared->vecThens.push_back(innerFunc);

        try
        {
            if(m_pShared->state == PromiseState::Resolved)
                innerFunc(m_pShared->value);
        }
        catch(...)
        {
            promise = reject<typename strip_promise<typename lambda_traits<Fn>::returnType>::type>(std::current_exception());
        }

        return promise;
    }

    template <typename Fn, typename std::enable_if<!std::is_same<void, typename lambda_traits<Fn>::returnType>::value &&
                                                     (lambda_traits<Fn>::numArgs == 0), int>::type = 0>
    auto then(Fn func) -> typename lambda_traits<Fn>::returnType
    {
        typename lambda_traits<Fn>::returnType promise;
        auto innerFunc = [promise, func](T) mutable {
            try
            {
                promise = func();
            }
            catch(...)
            {
                promise = reject<typename strip_promise<typename lambda_traits<Fn>::returnType>::type>(std::current_exception());
            }
        };

        m_pShared->vecThens.push_back(innerFunc);

        try
        {
            if(m_pShared->state == PromiseState::Resolved)
                innerFunc();
        }
        catch(...)
        {
            promise = reject<typename strip_promise<typename lambda_traits<Fn>::returnType>::type>(std::current_exception());
        }

        return promise;
    }

    template <typename Fn, typename std::enable_if<!std::is_same<void, typename lambda_traits<Fn>::returnType>::value &&
                                                     (lambda_traits<Fn>::numArgs == 1), int>::type = 0>
    auto then(Fn func) -> typename lambda_traits<Fn>::returnType
    {
        static_assert((lambda_traits<Fn>::numArgs == 0)
                      || std::is_same<T, lambda_traits<Fn>::args<0>::type>::value, "Err");

        typename lambda_traits<Fn>::returnType promise;
        auto innerFunc = [promise, func](T arg) mutable {
            try
            {
                promise = func(arg);
            }
            catch(...)
            {
                promise = reject<typename strip_promise<typename lambda_traits<Fn>::returnType>::type>(std::current_exception());
            }
        };

        m_pShared->vecThens.push_back(innerFunc);

        try
        {
            if(m_pShared->state == PromiseState::Resolved)
                innerFunc(m_pShared->value);
        }
        catch(...)
        {
            promise = reject<typename strip_promise<typename lambda_traits<Fn>::returnType>::type>(std::current_exception());
        }

        return promise;
    }

    template <typename Fn>
    Promise<T> failAny(Fn func)
    {
        Promise<T> promise;

        auto innerFunc = [promise, func](std::exception_ptr) mutable {
            try
            {
                func();
            }
            catch(...)
            {
                promise = reject<T>(std::current_exception());
            }
        };

        m_pShared->vecFails.push_back(innerFunc);

        if(m_pShared->state == PromiseState::Rejected)
            innerFunc(m_pShared->error);

        return promise;
    }

    template <typename Fn, typename std::enable_if<!std::is_same<typename std::exception_ptr, lambda_traits_arg_t<Fn>>::value,
                                                   int>::type = 0>
    Promise<T> fail(Fn func)
    {
        Promise<T> promise;

        auto innerFunc = [promise, func](std::exception_ptr err) mutable {
            try
            {
                std::rethrow_exception(err);
            }
            catch(lambda_traits<Fn>::args<0>::type value)
            {
                try
                {
                    func(value);
                }
                catch(...)
                {
                    promise = reject<T>(std::current_exception());
                }
            }
            catch(...)
            {
                promise = reject<T>(std::current_exception());
            }
        };

        m_pShared->vecFails.push_back(innerFunc);

        if(m_pShared->state == PromiseState::Rejected)
            innerFunc(m_pShared->error);

        return promise;
    }

    template <typename Fn, typename std::enable_if<std::is_same<typename std::exception_ptr, lambda_traits_arg_t<Fn>>::value,
                                                   int>::type = 0>
    Promise<T> fail(Fn func)
    {
        Promise<T> promise;

        auto innerFunc = [promise, func](std::exception_ptr err) mutable {
            try
            {
                try
                {
                    func(err);
                }
                catch(...)
                {
                    promise = reject<T>(std::current_exception());
                }
            }
            catch(...)
            {
                promise = reject<T>(std::current_exception());
            }
        };

        m_pShared->vecFails.push_back(innerFunc);

        if(m_pShared->state == PromiseState::Rejected)
            innerFunc(m_pShared->error);

        return promise;
    }

    template <typename Fn>
    Promise<T> always(Fn func)
    {
        Promise<T> promise;

        auto innerFunc = [promise, func]() mutable -> void {
            func();
        };

        m_pShared->vecAlways.push_back(innerFunc);

        if(m_pShared->state != PromiseState::Unresolved)
            innerFunc();

        return promise;
    }

protected:
    std::shared_ptr<PromiseSharedState<T>> m_pShared;
};

template <typename T>
class PromiseResolved : public Promise<T>
{
public:
    PromiseResolved<T>(const T & value)
    {
        m_pShared->state = PromiseState::Resolved;
        m_pShared->value = value;
    }
};

template <>
class PromiseResolved<void> : public Promise<void>
{
public:
    PromiseResolved<void>()
    {
        m_pShared->state = PromiseState::Resolved;
    }
};

template <typename T>
class PromiseRejected : public Promise<T>
{
public:
    PromiseRejected<T>(std::exception_ptr err)
    {
        m_pShared->state = PromiseState::Rejected;
        m_pShared->error = err;
    }
};

template <>
class PromiseRejected<void> : public Promise<void>
{
public:
    PromiseRejected<void>(std::exception_ptr err)
    {
        m_pShared->state = PromiseState::Rejected;
        m_pShared->error = err;
    }
};


inline Promise<void> resolve() {
    return PromiseResolved<void>();
}

template <typename T>
Promise<T> resolve(T arg) {
    return PromiseResolved<T>(arg);
}

template <typename T>
Promise<T> reject(std::exception_ptr err) {
    return PromiseRejected<T>(err);
}

template <typename T, typename E,
          typename std::enable_if<!std::is_same<std::exception_ptr, E>::value,
                                  int>::type = 0>
Promise<T> reject(E && err) {
    return PromiseRejected<T>(std::make_exception_ptr(err));
}

template <typename T, typename E,
          typename std::enable_if<std::is_same<std::exception_ptr, E>::value,
                                  int>::type = 0>
Promise<T> reject(E && err) {
    return PromiseRejected<T>(err);
}


}

#endif // PROMISE_H
