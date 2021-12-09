/* 
 * File:   objectPool.h
 * Author: massimo
 *
 * Created on April 12, 2017, 8:17 PM
 */
#pragma once

#include "objectFactory.h"
#include <mutex>
#include <queue>
////////////////////////////////////////////////////////////////////////////////
namespace object_pool
{
//
// This code is a fork & extension from the following code:
//
//  http://media.wiley.com/product_ancillary/50/11188580/DOWNLOAD/c25_code.zip
//
// From the original source file:
// --- cit ON
// Provides an object pool that can be used with any class that provides a
// default constructor.
//
// The object pool constructor creates a pool of objects, which it hands out
// to clients when requested via the acquireObject() method. acquireObject()
// returns an Object which is a std::shared_ptr with a custom deleter that
// automatically puts the object back into the object pool when the shared_ptr
// is destroyed and its reference reaches 0.
//
// The constructor and destructor on each object in the pool will be called only
// once each for the lifetime of the program, not once per acquisition and release.
//
// The primary use of an object pool is to avoid creating and deleting objects
// repeatedly. The object pool is most suited to applications that use large 
// numbers of objects with expensive constructors for short periods of time, if
// a profiler tells you that allocating and deallocating these objects is a
// bottleneck.
// --- cit OFF
//
// It is also possible to create an object pool providing a non-default ctor for
// the objects that is registered and used at any new allocation when the pool
// is empty
//
// Objects returned to the pool are reset by default
//
// See: https://en.wikipedia.org/wiki/Object_pool_pattern
////////////////////////////////////////////////////////////////////////////////
// Base class that does not depend on the class template
class objectPoolBase
{
 protected:
  static const int64_t m_kDefaultPoolSize;
  static const int64_t m_kdefaultHighWaterMark;
  size_t m_poolSize;
  // the max number of objects allowed in the pool
  // this value is not enforced in this implementation: a client should check
  // the tuple returned by the method acquireObject() and implement its strategy
  // or change this implementation
  size_t m_HighWaterMark;
  mutable size_t m_objectsCreated {};
  mutable std::mutex m_mx {};
  // by default the objects are reset when restored in the pool
  mutable bool m_doResetObjects {true};

  // delegating ctor
  explicit objectPoolBase();
  explicit objectPoolBase(int64_t poolSize,
                          int64_t highWaterMark) noexcept(false);

  virtual ~objectPoolBase();

  constexpr
  size_t
  _getNumberOfObjectsCreated() const noexcept
  {
    return m_objectsCreated;
  }

  constexpr
  bool
  _checkObjectsOverflow () const noexcept
  {
    return (_getNumberOfObjectsCreated() > m_HighWaterMark);
  }

  inline
  void
  _doResetObjects() const noexcept
  {
    m_doResetObjects = true;
  }

  inline
  void
  _doNotResetObjects() const noexcept
  {
    m_doResetObjects = false;
  }
  constexpr
  bool
  _getResetObjectsFlag() const noexcept
  {
    return m_doResetObjects;
  }

 public:
  // copy ctor deleted
  objectPoolBase(const objectPoolBase& src) = delete;
  // copy assignment deleted
  objectPoolBase& operator=(const objectPoolBase& rhs) = delete;

  inline
  size_t
  getNumberOfObjectsCreated() const noexcept
  {
    std::lock_guard<std::mutex> mlg(m_mx);

    return m_objectsCreated;
  }

  inline
  bool
  checkObjectsOverflow () const noexcept
  {
    std::lock_guard<std::mutex> mlg(m_mx);

    return (_getNumberOfObjectsCreated() > m_HighWaterMark);
  }

  inline
  void
  doResetObjects() const noexcept
  {
    std::lock_guard<std::mutex> mlg(m_mx);

    m_doResetObjects = true;
  }
  inline
  void
  doNotResetObjects() const noexcept
  {
    std::lock_guard<std::mutex> mlg(m_mx);

    m_doResetObjects = false;
  }
  inline
  bool
  getResetObjectsFlag() const noexcept
  {
    std::lock_guard<std::mutex> mlg(m_mx);

    return m_doResetObjects;
  }
};  // objectPoolBase

template <typename T>
class objectPool final : public objectPoolBase
{
  using objectPoolStatus = std::tuple<size_t, size_t, bool>;

 public:
  explicit
  objectPool() noexcept(false)
  :
  objectPool(m_kDefaultPoolSize)
  {}

  // copy ctor deleted
  objectPool(const objectPool& src) = delete;

  // Create an object pool with poolSize objects.
  // Whenever the object pool runs out of objects, poolSize more objects will be
  // added to the pool.
  // The pool only grows: Objects are never removed from the pool (freed), until
  // the pool is destroyed.
  //
  // Throw invalid_argument if poolSize or highWaterMark is <= 0 or if
  // poolSize > highWaterMark
  // Throw bad_alloc if allocation fails
  explicit
  objectPool(const int64_t poolSize,
             const int64_t highWaterMark = m_kdefaultHighWaterMark) noexcept(false)
  :
  objectPoolBase(poolSize, highWaterMark)
  {
    // Create poolSize objects to start
    allocatePool();
  }

  // this ctor is used when a non-default ctor for T is provided as an
  // object_creator::object_creator_fun<T> f
  explicit
  objectPool(object_factory::objectFactoryFun<T> f,
             const int64_t poolSize,
             const int64_t highWaterMark = m_kdefaultHighWaterMark) noexcept(false)
  :
  objectPoolBase(poolSize, highWaterMark),
  m_f(f)
  {
    // when a non-default ctor is provided the default object must be initialized
    // with that ctor
    m_defaultResetObject = *(m_f().get());
    
    // Create poolSize objects to start
    allocatePool();
  }
          
  // The type of smart pointer returned by acquireObject()
  using Object = std::shared_ptr<T>;

  // Reserve an object for use
  auto
  acquireObject() const noexcept(false) -> std::tuple<Object, bool>
  {
    std::lock_guard<std::mutex> mlg(m_mx);

    bool isObjectOverflow {};

    if ( m_FreeList.empty() )
    {
      std::tie(std::ignore, std::ignore, isObjectOverflow) = allocatePool();
    }

    // Move next free object from the queue to a local unique_ptr
    std::unique_ptr<T> obj(std::move(m_FreeList.front()));
    m_FreeList.pop();

    // Convert the object pointer to an Object (a shared_ptr with a custom deleter)
    Object smartObject(obj.release(),
            [this](T* t)
            {
              std::lock_guard<std::mutex> lg(m_mx);

              // when an object is returned to the pool it must be reset
              // this is the default
              if ( _getResetObjectsFlag() )
              {
                resetObject(*t);
              }
              // The custom deleter doesn't actually deallocate the memory,
              // but simply puts the reset object back on the free list
              m_FreeList.push(std::unique_ptr<T>(t));
            } );

    // Return the tuple {Object smartObject, bool isObjectOverflow}
    return std::make_tuple(smartObject,
                           isObjectOverflow );
  }  // acquireObject

  inline
  size_t
  getFreeListSize() const noexcept
  {
    std::lock_guard<std::mutex> mlg(m_mx);

    return m_FreeList.size();
  }

 private:
  // default copy assignment needed because of the method resetObject()
  objectPool& operator=(const objectPool& rhs) = default;

  // object used to reset pool's objects when returned to the pool
  mutable T m_defaultResetObject{};

  // m_FreeList stores the objects that are not currently in use by clients
  mutable std::queue<std::unique_ptr<T>> m_FreeList {};

  // the lambda that invokes a non-default ctor registered at object pool
  // creation; it's nullptr if not registered
  object_factory::objectFactoryFun<T> m_f {};

  // Allocates m_poolSize new objects and adds them to m_FreeList
  auto
  allocatePool() const noexcept(false) -> objectPoolStatus
  {
    // generate a lambda that will invoke the right ctor
    std::function<std::unique_ptr<T>()> lambdaCtor {};
    if (m_f)
    {
      // this lambda invokes the (non-default) ctor registered at object pool creation
      lambdaCtor = [this]() -> std::unique_ptr<T>
                   {
                     return m_f();
                   };
    }
    else
    {
      // this lambda invokes the default ctor
      lambdaCtor = []() -> std::unique_ptr<T>
                   {
                     return std::make_unique<T>();
                   };
    }

    for (size_t i {0}; i < m_poolSize; ++i)
    {
      m_FreeList.emplace(lambdaCtor());
      ++m_objectsCreated;
    }

    return std::make_tuple(_getFreeListSize(),
                           _getNumberOfObjectsCreated(),
                           _checkObjectsOverflow() );
  }  // allocatePool

  constexpr
  size_t
  _getFreeListSize() const noexcept
  {
    return m_FreeList.size();
  }

  inline
  void
  resetObject(T& object) const noexcept
  {
    object = m_defaultResetObject;
  }
};  // class ObjectPool
}  // namespace object_pool
