/* 
 * File:   objectPool.cpp
 * Author: massimo
 * 
 * Created on April 12, 2017, 8:17 PM
 */
#include "objectPool.h"
#include <stdexcept>
////////////////////////////////////////////////////////////////////////////////
namespace object_pool
{
const int64_t objectPoolBase::m_kDefaultPoolSize {10};
const int64_t objectPoolBase::m_kdefaultHighWaterMark {1'000};

objectPoolBase::objectPoolBase()
:
objectPoolBase(m_kDefaultPoolSize, m_kdefaultHighWaterMark)
{}

objectPoolBase::objectPoolBase(const int64_t poolSize,
                               const int64_t highWaterMark) noexcept(false)
:
m_poolSize(static_cast<size_t>(poolSize)),
m_HighWaterMark(static_cast<size_t>(highWaterMark))
{
  if ( poolSize <= 0 )
  {
    throw std::invalid_argument("pool size must be positive");
  }
  if ( highWaterMark <= 0 )
  {
    throw std::invalid_argument("high water mark must be positive");
  }
  if ( poolSize > highWaterMark )
  {
    throw std::invalid_argument("high water mark must be greater than pool size");
  }
}

objectPoolBase::~objectPoolBase()
{}
}  // namespace object_pool
////////////////////////////////////////////////////////////////////////////////
