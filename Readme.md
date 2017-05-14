CppPromise is an experimental implementation of the JavaScript [A+](https://promisesaplus.com/) Promise standard in C++. It is designed to make asynchornous code easier to write and maintain. There are several other implementations of this concept already ([cpp-promise](https://github.com/xhawk18/promise-cpp), [q](https://github.com/grantila/q), [PoolQueue](https://github.com/rhashimoto/poolqueue)). CppPromise is mostly an excuse for me to improve my tempalte meta-programming skills. However, it may have some features that make it useful to others.

* __Header only__
* __Works on early C++11 compilers__ - (VS2013 in particular)
* __Type-safe__ - Promise objects are strongly typed, and most mistakes should be caught at compile time.
* __Framework agnostic__ - Does not have a bundled event system or timers. Feel free to use any framewok that makes sense (Boost.Asio, libuv, Qt, etc...)

CppPromise is __extermly experimental__ at the moment. There is currently no documentation, almost no comments, and no tests.

Here is a code example
```C++
    //Return an already resolved promise containing an int
    Promise<int> testReturnInt()
    {
        return resolve(123);
    }

    //Return an already resolved promise containing a float
    Promise<float> testReturnFloat()
    {
        return resolve(1.23f);
    }

    testReturnInt().then([](int iTest) {
       REQUIRE(iTest == 123);
       return testReturnFloat();
    }).then([](float fTest) {
        REQUIRE(fTest == Approx(1.23));
        testReturnInt();
    }).fail([](std::exception &e) {
        std::cout << "Exception was thrown: " << e.what() << std::endl;
    });
```