#include "../objectPool.h"
#include <future>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// BEGIN: ignore the warnings listed below when compiled with clang from here
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma clang diagnostic ignored "-Wpadded"

using namespace ::testing;
////////////////////////////////////////////////////////////////////////////////
// Tests
TEST (objectFactory, test_1)
{
  class A
  {
    mutable int _x {0};
    mutable int _y {0};
    mutable int _z {0};

   public:
    explicit
    A() noexcept
    {
      std::cout << "default constructor A(): ";
      display_object();
    }

    explicit
    A(const int& x) noexcept
    :
    _x(x)
    {
      std::cout << "constructor-1 A(x): ";
      display_object();
    }

    explicit
    A(const int& x, const int& y) noexcept
    :
    _x(x),
    _y(y)
    {
      std::cout << "constructor-2 A(x, y): ";
      display_object();
    }

    explicit
    A(const int& x, const int& y, const int& z) noexcept
    :
    _x(x),
    _y(y),
    _z(z)
    {
      std::cout << "constructor-3 A(x, y, z): ";
      display_object();
    }

    int get_x() const noexcept
    {
      return _x;
    }
    int get_y() const noexcept
    {
      return _y;
    }
    int get_z() const noexcept
    {
      return _z;
    }

    void display_object (const std::string& prompt = "") const noexcept
    {
      std::cout << prompt
                << "(x, y, z): ("
                << get_x()
                << ", "
                << get_y()
                << ", "
                << get_z()
                << ")"
                << '\n';
    }

    ~A()
    {
      std::cout << "Destroyed object: ";
      display_object();
    }
  };  // class A

  using Object = std::unique_ptr<A>;
  
  object_factory::objectFactoryFun<A> objectFactoryFun {};

  objectFactoryFun = object_factory::createObjectFactoryFun<A>();
  Object o0 = objectFactoryFun();

  {
    objectFactoryFun = object_factory::createObjectFactoryFun<A, const int>(123);
  }
  Object o1 = objectFactoryFun();

  objectFactoryFun = object_factory::createObjectFactoryFun<A,const int>(456);
  Object o2 = objectFactoryFun();

  objectFactoryFun = object_factory::createObjectFactoryFun<A,const int, const int>(11, 22);
  Object o3 = objectFactoryFun();

  objectFactoryFun = object_factory::createObjectFactoryFun<A,const int, const int, const int>(11, 22, 33);
  Object o4 = objectFactoryFun();

  ASSERT_EQ(o0.get()->get_x(), 0);
  ASSERT_EQ(o0.get()->get_y(), 0);
  ASSERT_EQ(o0.get()->get_z(), 0);
  ASSERT_EQ(o1.get()->get_x(), 123);
  ASSERT_EQ(o1.get()->get_y(), 0);
  ASSERT_EQ(o1.get()->get_z(), 0);
  ASSERT_EQ(o2.get()->get_x(), 456);
  ASSERT_EQ(o2.get()->get_y(), 0);
  ASSERT_EQ(o2.get()->get_z(), 0);
  ASSERT_EQ(o3.get()->get_x(), 11);
  ASSERT_EQ(o3.get()->get_y(), 22);
  ASSERT_EQ(o3.get()->get_z(), 0);
  ASSERT_EQ(o4.get()->get_x(), 11);
  ASSERT_EQ(o4.get()->get_y(), 22);
  ASSERT_EQ(o4.get()->get_z(), 33);

  {
    objectFactoryFun = object_factory::createObjectFactoryFun<A,
                                                              const int,
                                                              const int,
                                                              const int>(99, 88, 77);

    std::vector<Object> v {};
    for (int i {1}; i <= 5; ++i)
    {
      // create an object that can be used in this scope only
      Object o = objectFactoryFun();
      // store the object in the vector v using move semantics;
      // the lifetime of the object is extended over this scope
      v.push_back(std::move(o));
    }
    // all elements in the vector have the same attribute values
    for (auto&& item : v)
    {
      ASSERT_EQ(item.get()->get_x(), 99);
      ASSERT_EQ(item.get()->get_y(), 88);
      ASSERT_EQ(item.get()->get_z(), 77);
    }
  }  // the objects in the vector v are destroyed, going out of scope
}

TEST(objectPool, test_1)
{
  EXPECT_THROW(object_pool::objectPool<bool> aPool(0), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<bool> aPool(-1), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<int8_t> aPool(0), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<int8_t> aPool(-1), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<int16_t> aPool(0), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<int16_t> aPool(-1), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<int32_t> aPool(0), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<int32_t> aPool(-1), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<int64_t> aPool(0), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<int64_t> aPool(-1), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<uint8_t> aPool(0), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<uint8_t> aPool(-1), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<uint16_t> aPool(0), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<uint16_t> aPool(-1), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<uint32_t> aPool(0), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<uint32_t> aPool(-1), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<uint64_t> aPool(0), std::invalid_argument);
  EXPECT_THROW(object_pool::objectPool<uint64_t> aPool(-1), std::invalid_argument);
}

TEST(objectPool, test_1A)
{
  EXPECT_NO_THROW(object_pool::objectPool<bool> aPool(10, 50));
  EXPECT_NO_THROW(object_pool::objectPool<int8_t> aPool(20, 2'000));
  EXPECT_NO_THROW(object_pool::objectPool<int16_t> aPool(30, 3'000));
  EXPECT_NO_THROW(object_pool::objectPool<int32_t> aPool(40));
  EXPECT_NO_THROW(object_pool::objectPool<int64_t> aPool(50, 6'000));
  EXPECT_NO_THROW(object_pool::objectPool<uint8_t> aPool(60));
  EXPECT_NO_THROW(object_pool::objectPool<uint16_t> aPool(70, 7'000));
  EXPECT_NO_THROW(object_pool::objectPool<uint32_t> aPool(80));
  EXPECT_NO_THROW(object_pool::objectPool<uint64_t> aPool(90, 9'000));
}

TEST(objectPool, test_2)
{
  class CA
  {
    int m_x {};

  public:
    CA() : m_x(8) {}

    ~CA()
    {
      std::clog << "CA dtor called: m_x is " << m_x << '\n';
    }
  };

  using CAop = object_pool::objectPool<CA>;
  EXPECT_THROW(CAop CAPool(0), std::invalid_argument);
  EXPECT_THROW(CAop CAPool(-1), std::invalid_argument);
  EXPECT_THROW(CAop CAPool(-1000), std::invalid_argument);
  EXPECT_THROW(CAop CAPool(10, -10'000), std::invalid_argument);
  EXPECT_THROW(CAop CAPool(-10, 10'000), std::invalid_argument);
  EXPECT_THROW(CAop CAPool(-10, -10'000), std::invalid_argument);
  EXPECT_THROW(CAop CAPool(100, 10), std::invalid_argument);
  EXPECT_NO_THROW(CAop CAPool(10, 10'000));
}

TEST(objectPool, test_3)
{
  static const int poolSize {1'000};
  static int ctorCalls {0};
  static int dtorCalls {0};

  class CA
  {
    int m_id {};

  public:

    CA()
    :
    m_id(++ctorCalls)
    {
      if ( m_id < poolSize )
      {
        std::clog << "Constructed object " << m_id << "\r";
        return;
      }
      std::clog << "Constructed object " << m_id << '\n';
    }

    ~CA()
    {
      ++dtorCalls;
      if ( m_id < poolSize )
      {
        std::clog << "Destroyed object   " << m_id << "\r";
        return;
      }
      std::clog << "Destroyed object   " << m_id << '\n';
    }
  };

  using CAop = object_pool::objectPool<CA>;

  EXPECT_NO_THROW(CAop CAPool(poolSize));
  ASSERT_EQ(poolSize + 1, ctorCalls);
  ASSERT_EQ(poolSize + 1, dtorCalls);
}

TEST(objectPool, test_4)
{
  // Let's create a pool of int's
  using int_op = object_pool::objectPool<int>;

  const auto poolSize {1};
  const auto hardMaxObjectsLimit {1};

  // the pool has 1 object, and 1 as the hard max objects limit
  int_op iPool(poolSize, hardMaxObjectsLimit);

  // check the intial conditions
  ASSERT_EQ(poolSize, iPool.getFreeListSize());
  ASSERT_EQ(poolSize, iPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, iPool.checkObjectsOverflow());

  {
    // let's get an object from the pool
    // C++17
    auto [o, of] = iPool.acquireObject();
    // Equivalent to:
    // int_op::Object o;
    // bool of{};
    // std::tie(o, of) = iPool.acquireObject();

    // check the pool of free elements is empty now
    ASSERT_EQ(0, iPool.getFreeListSize());  
    ASSERT_EQ(poolSize, iPool.getNumberOfObjectsCreated());
    ASSERT_EQ(false, iPool.checkObjectsOverflow());
    ASSERT_EQ(of, iPool.checkObjectsOverflow());

    // reference to the value
    auto& vr = *(o.get());
    // copying the value in v is allowed
    auto v = *(o.get());

    ASSERT_EQ(0, *(o.get()));
    ASSERT_EQ(0, vr);
    ASSERT_EQ(0, v);

    // change the referenced value
    vr = 123;
    // check it's changed
    ASSERT_EQ(123, *(o.get()));
    ASSERT_EQ(123, vr);
    // check it's not changed here
    ASSERT_EQ(0, v);
  }  // Object o and bool of go out of scope

  // The Object o has been restored in the pool
  // Check the free list pool is not empty now
  ASSERT_EQ(poolSize, iPool.getFreeListSize());  
  ASSERT_EQ(poolSize, iPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, iPool.checkObjectsOverflow());

  // pool destroyed here when aPool goes out of scope
  std::clog << "Object pool being destroyed now... total objects in the pool: "
            << iPool.getNumberOfObjectsCreated()
            << '\n';
}

TEST(objectPool, test_5)
{
  const int initDefaultValue {2};

  class CA
  {
    mutable int m_x {};

  public:
    // By default the attribute is set to initDefaultValue
    explicit CA() : m_x(initDefaultValue) {}

    CA(const CA& rhs)
    {
      m_x = rhs.m_x;
    }
    CA& operator=(const CA& rhs)
    {
      m_x = rhs.m_x;
      return *this;
    }

    ~CA()
    {
      std::clog << "CA dtor called - m_x set to " << m_x << '\n';
    }

    int get_x () const noexcept
    {
      return m_x;
    }

    void set_x(const int& x) const noexcept
    {
      m_x = x;
    }
  };  // class CA

  // Let's create a pool of CA's objects
  using ca_op = object_pool::objectPool<CA>;

  const auto poolSize {1};
  const auto hardMaxObjectsLimit {1};

  // the pool has 1 object, and 1 as the hard max objects limit
  ca_op caPool(poolSize,hardMaxObjectsLimit);

  // check the intial conditions
  ASSERT_EQ(poolSize, caPool.getFreeListSize());
  ASSERT_EQ(poolSize, caPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, caPool.checkObjectsOverflow());

  {
    // let's get an object from the pool
    // C++17
    auto [o1, of1] = caPool.acquireObject();
    // Equivalent to:
    // ca_op::Object o1;
    // bool of1{};
    // std::tie(o1, of1) = caPool.acquireObject();

    // check the pool is empty now
    ASSERT_EQ(0, caPool.getFreeListSize());  
    ASSERT_EQ(poolSize, caPool.getNumberOfObjectsCreated());
    ASSERT_EQ(false, caPool.checkObjectsOverflow());
    ASSERT_EQ(of1, caPool.checkObjectsOverflow());

    // let's try to get another object from the pool
    // since the pool is empty a new pool of size 1 is allocated
    auto [o2, of2] = caPool.acquireObject();

    // check the free list pool is still empty
    ASSERT_EQ(0, caPool.getFreeListSize());
    // and a new object was created
    ASSERT_EQ(2, caPool.getNumberOfObjectsCreated());
    // and we are in an overflow condition since the hard max objects limit is 1
    ASSERT_EQ(true, caPool.checkObjectsOverflow());
    ASSERT_EQ(of2, caPool.checkObjectsOverflow());
    
    // reference to the value of o1
    auto& vr1 = *(o1.get());
    // copying the value in v1 is allowed
    auto v1 = *(o1.get());

    ASSERT_EQ(initDefaultValue, o1.get()->get_x() );
    ASSERT_EQ(initDefaultValue, vr1.get_x() );

    vr1.set_x(345);

    ASSERT_EQ(345, o1.get()->get_x() );
    ASSERT_EQ(345, vr1.get_x() );
  }  // Objects o1, o2 and bool's of1, of2 go out of scope

  // The Objects o1, o2 have been restored in the pool
  // Check the free list pool is not empty now
  ASSERT_EQ(2, caPool.getFreeListSize());  
  ASSERT_EQ(2, caPool.getNumberOfObjectsCreated());
  ASSERT_EQ(true, caPool.checkObjectsOverflow());

  // pool destroyed here when aPool goes out of scope
  std::clog << "Object pool being destroyed now... total objects in the pool: "
            << caPool.getNumberOfObjectsCreated()
            << '\n';
}

TEST(objectPool, test_5A)
{
  const int initDefaultValue {2};

  class CA
  {
    mutable int m_x {};

  public:
    // By default the attribute is set to initDefaultValue
    explicit CA() : m_x(initDefaultValue) {}

    CA(const CA& rhs)
    {
      m_x = rhs.m_x;
    }

    CA& operator=(const CA& rhs)
    {
      m_x = rhs.m_x;
      return *this;
    }

    ~CA()
    {
      std::clog << "CA dtor called - m_x set to " << m_x << '\n';
    }
  };  // class CA

  // Let's create a pool of CA's objects
  using ca_op = object_pool::objectPool<CA>;

  const auto poolSize {1};
  const auto hardMaxObjectsLimit {1};

  // the pool has 1 object, and 1 as the hard max objects limit
  ca_op caPool(poolSize,hardMaxObjectsLimit);

  // check the intial conditions
  ASSERT_EQ(poolSize, caPool.getFreeListSize());
  ASSERT_EQ(poolSize, caPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, caPool.checkObjectsOverflow());

  {
    // let's try to get an object from the pool but use std::ignore instead of
    // an object variable.
    // In this case no object is returned by acquireObject(), though an object
    // is removed from the free list pool and immediately restored back by the
    // custom dtor
    bool of1 {};
    std::tie(std::ignore, of1) = caPool.acquireObject();

    // check the free list pool status: it must not have been changed
    ASSERT_EQ(poolSize, caPool.getFreeListSize());  
    ASSERT_EQ(poolSize, caPool.getNumberOfObjectsCreated());
    ASSERT_EQ(false, caPool.checkObjectsOverflow());
    ASSERT_EQ(of1, caPool.checkObjectsOverflow());
  }

  // Check the free list pool again
  ASSERT_EQ(poolSize, caPool.getFreeListSize());  
  ASSERT_EQ(poolSize, caPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, caPool.checkObjectsOverflow());

  // pool destroyed here when aPool goes out of scope
  std::clog << "Object pool being destroyed now... total objects in the pool: "
            << caPool.getNumberOfObjectsCreated()
            << '\n';
}

TEST(objectPool, test_6)
{
  const int initDefaultValue {7};

  class CA
  {
    mutable int m_x {};

  public:
    // By default the attribute is set to initDefaultValue
    explicit CA() : m_x(initDefaultValue) {}

    CA(const CA& rhs)
    {
      m_x = rhs.m_x;
    }

    CA& operator=(const CA& rhs)
    {
      m_x = rhs.m_x;
      return *this;
    }

    ~CA()
    {
      std::clog << "CA dtor called - m_x set to " << m_x << '\n';
    }
  };  // class CA

  // Let's create a pool of CA's objects
  using ca_op = object_pool::objectPool<CA>;

  const auto poolSize {1};
  const auto hardMaxObjectsLimit {100};

  // the pool has 1 object, and 100 as the hard max objects limit
  ca_op caPool(poolSize,hardMaxObjectsLimit);

  // check the initial conditions
  ASSERT_EQ(poolSize, caPool.getFreeListSize());
  ASSERT_EQ(poolSize, caPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, caPool.checkObjectsOverflow());

  const int objectsToAcquire {10};

  {
    std::vector<ca_op::Object> v {};

    // Acquire objectsToAcquire objects from the pool and push back them in vector v
    // Since the object pool has been created with 1 object, objectsToAcquire-1
    // more objects will be created
    for (int i {1}; i <= objectsToAcquire; ++i)
    {
      auto [o1, of1] = caPool.acquireObject();
      v.push_back(o1);
    }
  }  // Vector v goes out of scope and all objectsToAcquire objects are restored
     // back in the pool

  // The Objects have been restored in the pool
  // Check the free list pool is not empty now
  ASSERT_EQ(objectsToAcquire, caPool.getFreeListSize());  
  ASSERT_EQ(objectsToAcquire, caPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, caPool.checkObjectsOverflow());

  // pool destroyed here when aPool goes out of scope
  std::clog << "Object pool being destroyed now... total objects in the pool: "
            << caPool.getNumberOfObjectsCreated()
            << '\n';
}

TEST(objectPool, test_7)
{
  const int initDefaultValue {88};

  class CA
  {
    mutable int m_x {};

  public:
    // By default the attribute is set to initDefaultValue
    explicit CA() : m_x(initDefaultValue) {}

    CA(const CA& rhs)
    {
      m_x = rhs.m_x;
    }

    CA& operator=(const CA& rhs)
    {
      m_x = rhs.m_x;
      return *this;
    }

    ~CA()
    {
      std::clog << "CA dtor called - m_x set to " << m_x << '\n';
    }

    void set_x(const int x) const noexcept
    {
      m_x = x;
    }
  };  // class CA

  // Let's create a pool of CA's objects
  using ca_op = object_pool::objectPool<CA>;

  const auto poolSize {2};
  const auto hardMaxObjectsLimit {1'000};

  // the pool has 1 object, and 1000 as the hard max objects limit
  ca_op caPool(poolSize,hardMaxObjectsLimit);

  // check the initial conditions
  ASSERT_EQ(poolSize, caPool.getFreeListSize());
  ASSERT_EQ(poolSize, caPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, caPool.checkObjectsOverflow());

  const int objectsToAcquire {2};

  {
    std::vector<ca_op::Object> v {};

    // Acquire 1500 objects from the pool and push back them in vector v
    // Since the object pool has been created with 1 object, 1499 more objects will
    // be created
    for (int i {1}; i <= objectsToAcquire; ++i)
    {
      auto [o1, of1] = caPool.acquireObject();
      // Just change the attribute value to the current index value
      o1.get()->set_x(i);
      // Push the object in the vector
      v.push_back(o1);
    }
  }  // Vector v goes out of scope and all 1500 objects are restored back in the pool

  // The Objects have been restored in the pool
  // Check the free list pool is not empty now
  ASSERT_EQ(objectsToAcquire, caPool.getFreeListSize());  
  ASSERT_EQ(objectsToAcquire, caPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, caPool.checkObjectsOverflow());

  // pool destroyed here when aPool goes out of scope
  std::clog << "Object pool being destroyed now... total objects in the pool: "
            << caPool.getNumberOfObjectsCreated()
            << '\n';
}

TEST (objectPoolWithCreator, test_1)
{
  class A
  {
    mutable std::string _s{};
    mutable int _x{};
    mutable int _y{};
    mutable int _z{};

   public:
    explicit
    A() noexcept
    {
      std::cout << "default constructor A(): ";
      display_object();
    }

    explicit
    A(const std::string& s,
      const int& x,
      const int& y,
      const int& z) noexcept
    :
    _s(s),
    _x(x),
    _y(y),
    _z(z)
    {
      std::cout << "constructor-1 A(s, x, y, z): ";
      display_object();
    }

    std::string get_s() const noexcept
    {
      return _s;
    }
    int get_x() const noexcept
    {
      return _x;
    }
    int get_y() const noexcept
    {
      return _y;
    }
    int get_z() const noexcept
    {
      return _z;
    }

    void set_x(const int& x) const noexcept
    {
      _x = x;
    }

    void display_object (const std::string& prompt = "") const noexcept
    {
      std::cout << prompt
                << "(s, x, y, z): ("
                << get_s()
                << ", "
                << get_x()
                << ", "
                << get_y()
                << ", "
                << get_z()
                << ")"
                << '\n';
    }

    A& operator=(const A& rhs)
            {
              _s = rhs._s;
              _x = rhs._x;
              _y = rhs._y;
              _z = rhs._z;
              return *this;
            }

    ~A()
    {
      std::cout << "Destroyed object: ";
      display_object();
    }
  };  // class A

  auto S {"Object Name"};
  auto X {34};
  const auto Y {56};
  auto Z {78};

  object_factory::objectFactoryFun<A> objectFactoryFun {};
  objectFactoryFun = object_factory::createObjectFactoryFun<A,
                                                            const decltype(S),
                                                            const decltype(X),
                                                            const decltype(Y),
                                                            const decltype(Z)>
                          (std::forward<const decltype(S)>(S),
                           std::forward<const decltype(X)>(X),
                           std::forward<const decltype(Y)>(Y),
                           std::forward<const decltype(Z)>(Z));

  // Let's create a pool of A's objects
  using a_op = object_pool::objectPool<A>;

  const auto poolSize {2};
  const auto hardMaxObjectsLimit {10};

  // the pool has 2 object, and 10 as the hard max objects limit
  // the lambda that invokes the ctor registered is objectFactoryFun()
  a_op aPool(objectFactoryFun, poolSize, hardMaxObjectsLimit);

  // check the initial conditions
  ASSERT_EQ(poolSize, aPool.getFreeListSize());
  ASSERT_EQ(poolSize, aPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, aPool.checkObjectsOverflow());

  // get 2 objects from the pool
  auto [o1, of1] = aPool.acquireObject();
  auto [o2, of2] = aPool.acquireObject();

  // check pool conditions
  ASSERT_EQ(poolSize - 2, aPool.getFreeListSize());
  ASSERT_EQ(poolSize, aPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, aPool.checkObjectsOverflow());
  
  auto& vr1 = *o1.get();

  // check the object's attribute values set by the non-default ctor invoked by
  // objectFactoryFun()
  ASSERT_EQ(S, vr1.get_s());
  ASSERT_EQ(X, vr1.get_x());
  ASSERT_EQ(Y, vr1.get_y());
  ASSERT_EQ(Z, vr1.get_z());

  const int objectsToAcquire {50};
  {
    // create a vector of objects
    std::vector<decltype(o1)> v {};
    {
      // acquire objectsToAcquire objects from the pool; since the object pool is
      // empty then objectsToAcquire more objects are created
      for (int i {1}; i <= objectsToAcquire; ++i)
      {
        auto [o, f] = aPool.acquireObject();
        // Just change the attribute value to the current index value
        o.get()->set_x(i);
        v.push_back(o);
      }

      // check the current conditions
      ASSERT_EQ(objectsToAcquire, v.size());
      ASSERT_EQ(0, aPool.getFreeListSize());
      ASSERT_EQ(objectsToAcquire + poolSize, aPool.getNumberOfObjectsCreated());
      ASSERT_EQ(true, aPool.checkObjectsOverflow());
    }
  }  // the vector is freed, then all objects are restored in the object pool

  // check the current conditions
  ASSERT_EQ(objectsToAcquire, aPool.getFreeListSize());
  ASSERT_EQ(objectsToAcquire + poolSize, aPool.getNumberOfObjectsCreated());
  ASSERT_EQ(true, aPool.checkObjectsOverflow());

  // pool destroyed here when aPool goes out of scope
  std::clog << "Object pool being destroyed now... total objects in the pool: "
            << aPool.getNumberOfObjectsCreated()
            << '\n';
}

TEST (objectPoolWithCreator, test_2)
{
  using aCtorTuple = std::tuple<std::string, int, int, int>;
  
  class A
  {
    mutable std::string _s{};
    mutable int _x{};
    mutable int _y{};
    mutable int _z{};

   public:
    explicit
    A() noexcept
    {
      std::cout << "default constructor A(): ";
      display_object();
    }

    explicit
    A(const aCtorTuple& t) noexcept
    :
    _s(std::get<0>(t)),
    _x(std::get<1>(t)),
    _y(std::get<2>(t)),
    _z(std::get<3>(t))
    {
      std::cout << "constructor-1 A({s, x, y, z}): ";
      display_object();
    }

    std::string get_s() const noexcept
    {
      return _s;
    }
    int get_x() const noexcept
    {
      return _x;
    }
    int get_y() const noexcept
    {
      return _y;
    }
    int get_z() const noexcept
    {
      return _z;
    }

    void set_x(const int& x) const noexcept
    {
      _x = x;
    }

    void display_object (const std::string& prompt = "") const noexcept
    {
      std::cout << prompt
                << "(s, x, y, z): ("
                << get_s()
                << ", "
                << get_x()
                << ", "
                << get_y()
                << ", "
                << get_z()
                << ")"
                << '\n';
    }

    A(const A& rhs)
    {
      _s = rhs._s;
      _x = rhs._x;
      _y = rhs._y;
      _z = rhs._z;
    }

    A& operator=(const A& rhs)
    {
      _s = rhs._s;
      _x = rhs._x;
      _y = rhs._y;
      _z = rhs._z;
      return *this;
    }

    ~A()
    {
      std::cout << "Destroyed object: ";
      display_object();
    }
  };  // class A

  // Pack the values for the ctor into a tuple and pass it to the ctor
  auto S {"Object Name"};
  auto X {12};
  const auto Y {34};
  auto Z {56};
  aCtorTuple t {S, X, Y, Z};
  object_factory::objectFactoryFun<A> objectFactoryFun {};

  objectFactoryFun = object_factory::createObjectFactoryFun<A, const decltype(t)>
                            (std::forward<const decltype(t)>(t));

  // Let's create a pool of A's objects
  using a_op = object_pool::objectPool<A>;

  const auto poolSize {2};
  const auto hardMaxObjectsLimit {10};

  // the pool has 2 object, and 10 as the hard max objects limit
  // the lambda that invokes the ctor registered is objectFactoryFun()
  a_op aPool(objectFactoryFun, poolSize, hardMaxObjectsLimit);

  // check the initial conditions
  ASSERT_EQ(poolSize, aPool.getFreeListSize());
  ASSERT_EQ(poolSize, aPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, aPool.checkObjectsOverflow());

  // get 2 objects from the pool
  auto [o1, of1] = aPool.acquireObject();
  auto [o2, of2] = aPool.acquireObject();

  // check pool conditions
  ASSERT_EQ(poolSize - 2, aPool.getFreeListSize());
  ASSERT_EQ(poolSize, aPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, aPool.checkObjectsOverflow());

  auto& vr1 = *o1.get();

  // check the object's attribute values set by the non-default ctor invoked by
  // objectFactoryFun()
  ASSERT_EQ(S, vr1.get_s());
  ASSERT_EQ(X, vr1.get_x());
  ASSERT_EQ(Y, vr1.get_y());
  ASSERT_EQ(Z, vr1.get_z());

  const int objectsToAcquire {50};
  {
    // create a vector of objects
    std::vector<decltype(o1)> v {};
    {
      // acquire objectsToAcquire objects from the pool; since the object pool is
      // empty then objectsToAcquire more objects are created
      for (int i {1}; i <= objectsToAcquire; ++i)
      {
        auto [o, f] = aPool.acquireObject();
        // Just change the attribute value to the current index value
        o.get()->set_x(i);
        v.push_back(o);
      }

      // check the current conditions
      ASSERT_EQ(objectsToAcquire, v.size());
      ASSERT_EQ(0, aPool.getFreeListSize());
      ASSERT_EQ(objectsToAcquire + poolSize, aPool.getNumberOfObjectsCreated());
      ASSERT_EQ(true, aPool.checkObjectsOverflow());
    }
  }  // the vector is freed, then all objects are restored in the object pool

  // check the current conditions
  ASSERT_EQ(objectsToAcquire, aPool.getFreeListSize());
  ASSERT_EQ(objectsToAcquire + poolSize, aPool.getNumberOfObjectsCreated());
  ASSERT_EQ(true, aPool.checkObjectsOverflow());

  // pool destroyed here when aPool goes out of scope
  std::clog << "Object pool being destroyed now... total objects in the pool: "
            << aPool.getNumberOfObjectsCreated()
            << '\n';
}

////////////////////////////////////////////////////////////////////////////////
//// Multithreaded test

using bCtorTuple = std::tuple<std::string, int>;

class B
{
  mutable std::string _s{};
  mutable int _k{};
  mutable int _reuseCounter{};

 public:
  explicit
  B() noexcept
  {
    std::cout << "default constructor B(): ";
    display_object();
  }

  explicit
  B(const bCtorTuple& t) noexcept
  :
  _s(std::get<0>(t)),
  _k(std::get<1>(t))
  {
    std::cout << "constructor-1 B({s, k, rc}): ";
    display_object();
  }

  B(const B& rhs)
  {
    _s = rhs._s;
    _k = rhs._k;
    _reuseCounter = rhs._reuseCounter;
  }

  B& operator=(const B& rhs)
  {
    _s = rhs._s;
    _k = rhs._k;
    _reuseCounter = rhs._reuseCounter;
    return *this;
  }

  ~B()
  {
    std::cout << "Destroyed object: ";
    display_object();
  }

  std::string get_s() const noexcept
  {
    return _s;
  }
  int get_k() const noexcept
  {
    return _k;
  }
  int get_reuseCounter() const noexcept
  {
    return _reuseCounter;
  }

  void append_s(const std::string& s) const noexcept
  {
    _s = _s + " " + s;
  }
  void set_s(const std::string& s) const noexcept
  {
    _s = s;
  }
  void set_k(const int& k) const noexcept
  {
    _k = k;
  }
  void updateReuseCounter() const noexcept
  {
    ++_reuseCounter;
  }

  void display_object (const std::string& prompt = "") const noexcept
  {
    std::cout << prompt
              << "(s, k, rc): ("
              << get_s()
              << ", "
              << get_k()
              << ", "
              << get_reuseCounter()
              << ")"
              << '\n';
  }
};  // class B

static void allow (const int64_t& d = 0) noexcept
{
  std::this_thread::yield();
  if ( 0 == d )
  {
    return;
  }
  std::this_thread::sleep_for(std::chrono::nanoseconds(d));
}

// Let's create a pool of B's objects
using b_op = object_pool::objectPool<B>;

static void threadBody(b_op& aPool,
                       const std::string& func,
                       const int K,
                       const int64_t& d) noexcept
{
   std::clog << "[" << func << "] "
            << "STARTED"
            << '\n';

  // get 1 object from the pool
  auto [o1, of1] = aPool.acquireObject();
  auto& vr1 = *o1.get();
  vr1.append_s(func);
  vr1.set_k(K);
  vr1.updateReuseCounter();

  const int objectsToAcquire {50};
  for (unsigned int loops {1}; loops <= 10; ++loops)
  {
    // create a vector of objects
    std::vector<decltype(o1)> v {};
    {
      // acquire objectsToAcquire objects from the pool;
      for (int i {1}; i <= objectsToAcquire; ++i)
      {
        auto [o, f] = aPool.acquireObject();
        // append the function name to the string attribute value
        o.get()->append_s(func);
        o.get()->set_k(K);
        o.get()->updateReuseCounter();
        v.push_back(o);
        
        allow(d);
      }

      // check the current conditions
      ASSERT_EQ(objectsToAcquire, v.size());
      ASSERT_EQ(false, aPool.checkObjectsOverflow());
    }
  }  // the vector is freed, then all objects are restored in the object pool

  // we cannot say anything about the size of the free list here, for sure we
  // can say something about the number of objects created
  ASSERT_TRUE(aPool.getNumberOfObjectsCreated() >= (objectsToAcquire + 2));

  std::clog << "[" << func << "] "
            << "TERMINATED"
            << '\n'; 
}  // threadBody

static void clientThread_1 (b_op& aPool)
{
  threadBody(aPool, __func__, 1, 1'700'000);
}  // clientThread_1

static void clientThread_2 (b_op& aPool)
{
  threadBody(aPool, __func__, 2, 3'700'000);
}  //clientThread_2

static void clientThread_3 (b_op& aPool)
{
  threadBody(aPool, __func__, 3, 5'700'000);
}  //clientThread_3

static void clientThread_4 (b_op& aPool)
{
  threadBody(aPool, __func__, 4, 7'700'000);
}  //clientThread_4

static void clientThread_5 (b_op& aPool)
{
  threadBody(aPool, __func__, 5, 9'700'000);
}  //clientThread_5

TEST (objectPoolWithCreator, multiThreadedTest_1)
{
  // Pack the values for the ctor into a tuple and pass it to the ctor
  auto S {"-init-"};
  auto K {-1};
  bCtorTuple t {S, K};
  object_factory::objectFactoryFun<B> objectFactoryFun {};

  objectFactoryFun = object_factory::createObjectFactoryFun<B, const decltype(t)>
                            (std::forward<const decltype(t)>(t));

  const auto poolSize {2};
  const auto hardMaxObjectsLimit {1'000};

  // the pool has 2 object, and 1'000 as the hard max objects limit
  // the lambda that invokes the ctor registered is objectFactoryFun()
  b_op bPool(objectFactoryFun, poolSize, hardMaxObjectsLimit);

  // reset objects when returned to the pool (this is the default)

  // check the initial conditions
  ASSERT_EQ(poolSize, bPool.getFreeListSize());
  ASSERT_EQ(poolSize, bPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, bPool.checkObjectsOverflow());
  ASSERT_EQ(true, bPool.getResetObjectsFlag());

////////////////////////////////////////////////////////////////////////////////
//// start the threads

  // tasks launched asynchronously
  std::future<void> ct1 = std::async(std::launch::async,
                                     clientThread_1,
                                     std::ref(bPool));
  std::future<void> ct2 = std::async(std::launch::async,
                                     clientThread_2,
                                     std::ref(bPool));
  std::future<void> ct3 = std::async(std::launch::async,
                                     clientThread_3,
                                     std::ref(bPool));
  std::future<void> ct4 = std::async(std::launch::async,
                                     clientThread_4,
                                     std::ref(bPool));
  std::future<void> ct5 = std::async(std::launch::async,
                                     clientThread_5,
                                     std::ref(bPool));

  // wait for all threads to be finished and process any exception
  try
  {
    ct1.get();
    ct2.get();
    ct3.get();
    ct4.get();
    ct5.get();
  }
  catch( const std::exception& e )
  {
    std::clog << "EXCEPTION: "
              << e.what()
              << '\n';
  }
////////////////////////////////////////////////////////////////////////////////

  // check the current conditions
  std::clog << "Objects in the free list after the threads' termination: "
            << bPool.getFreeListSize()
            << '\n';
  ASSERT_TRUE(bPool.getNumberOfObjectsCreated() >= 102);
  ASSERT_EQ(false, bPool.checkObjectsOverflow());

  // pool destroyed here when aPool goes out of scope
  std::clog << "Object pool being destroyed now... total objects in the pool: "
            << bPool.getNumberOfObjectsCreated()
            << '\n';
}

TEST (objectPoolWithCreator, multiThreadedTest_2)
{
  // Pack the values for the ctor into a tuple and pass it to the ctor
  auto S {"-init-"};
  auto K {-1};
  bCtorTuple t {S, K};
  object_factory::objectFactoryFun<B> objectFactoryFun {};

  objectFactoryFun = object_factory::createObjectFactoryFun<B, const decltype(t)>
                            (std::forward<const decltype(t)>(t));

  const auto poolSize {2};
  const auto hardMaxObjectsLimit {1'000};

  // the pool has 2 object, and 1'000 as the hard max objects limit
  // the lambda that invokes the ctor registered is objectFactoryFun()
  b_op bPool(objectFactoryFun, poolSize, hardMaxObjectsLimit);

  // do NOT reset the objects when returned to the pool
  bPool.doNotResetObjects();

  // check the initial conditions
  ASSERT_EQ(poolSize, bPool.getFreeListSize());
  ASSERT_EQ(poolSize, bPool.getNumberOfObjectsCreated());
  ASSERT_EQ(false, bPool.checkObjectsOverflow());
  ASSERT_EQ(false, bPool.getResetObjectsFlag());

////////////////////////////////////////////////////////////////////////////////
//// start the threads

  // tasks launched asynchronously
  std::future<void> ct1 = std::async(std::launch::async,
                                     clientThread_1,
                                     std::ref(bPool));
  std::future<void> ct2 = std::async(std::launch::async,
                                     clientThread_2,
                                     std::ref(bPool));
  std::future<void> ct3 = std::async(std::launch::async,
                                     clientThread_3,
                                     std::ref(bPool));
  std::future<void> ct4 = std::async(std::launch::async,
                                     clientThread_4,
                                     std::ref(bPool));
  std::future<void> ct5 = std::async(std::launch::async,
                                     clientThread_5,
                                     std::ref(bPool));

  // wait for all threads to be finished and process any exception
  try
  {
    ct1.get();
    ct2.get();
    ct3.get();
    ct4.get();
    ct5.get();
  }
  catch( const std::exception& e )
  {
    std::clog << "EXCEPTION: "
              << e.what()
              << '\n';
  }
////////////////////////////////////////////////////////////////////////////////

  // check the current conditions
  std::clog << "Objects in the free list after the threads' termination: "
            << bPool.getFreeListSize()
            << '\n';
  ASSERT_TRUE(bPool.getNumberOfObjectsCreated() >= 102);
  ASSERT_EQ(false, bPool.checkObjectsOverflow());

  // pool destroyed here when aPool goes out of scope
  std::clog << "Object pool being destroyed now... total objects in the pool: "
            << bPool.getNumberOfObjectsCreated()
            << '\n';
}

#pragma clang diagnostic pop
// END: ignore the warnings when compiled with clang up to here
