#ifndef PYTHONIC_NUMPY_ANGLEINRAD_HPP
#define PYTHONIC_NUMPY_ANGLEINRAD_HPP

#include "pythonic/include/numpy/angle_in_rad.hpp"

#include "pythonic/utils/functor.hpp"
#include "pythonic/types/ndarray.hpp"
#include "pythonic/utils/numpy_traits.hpp"
#include "pythonic/numpy/arctan.hpp"
#include "pythonic/numpy/pi.hpp"

/* NOTE: angle_in_rad is ! part of the official Numpy API,
 * this file is here only to split the angle function in two parts
 */

PYTHONIC_NS_BEGIN

namespace numpy
{
  namespace wrapper
  {
    template <class T>
    auto angle_in_rad(T const &t)
        -> decltype(boost::simd::atan(std::imag(t) / std::real(t)))
    {
      if (std::real(t))
        return boost::simd::atan(std::imag(t) / std::real(t));
      else
        return pythonic::numpy::pi / 2;
    }
  }

#define NUMPY_NARY_FUNC_NAME angle_in_rad
#define NUMPY_NARY_FUNC_SYM wrapper::angle_in_rad
#include "pythonic/types/numpy_nary_expr.hpp"
}
PYTHONIC_NS_END

#endif
