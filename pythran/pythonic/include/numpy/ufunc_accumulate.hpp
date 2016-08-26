#ifndef UFUNC_NAME
#error missing UFUNC_NAME
#endif

// clang-format off
#include INCLUDE_FILE(pythonic/include/numpy,UFUNC_NAME)
// clang-format on
#include "pythonic/include/utils/functor.hpp"
#include <pythonic/include/numpy/partial_sum.hpp>

#include <utility>

namespace pythonic
{
  namespace numpy
  {
    namespace UFUNC_NAME
    {
      template <class T, class dtype =
                             numpy::result_dtype<numpy::functor::UFUNC_NAME, T>>
      auto accumulate(T &&a, long axis = 0, dtype d = dtype()) -> decltype(
          partial_sum<numpy::functor::UFUNC_NAME>(std::forward<T>(a), axis, d));
      DECLARE_FUNCTOR(pythonic::numpy::UFUNC_NAME, accumulate);
    }
  }
}
