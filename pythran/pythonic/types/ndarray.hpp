#ifndef PYTHONIC_TYPES_NDARRAY_HPP
#define PYTHONIC_TYPES_NDARRAY_HPP

#include "pythonic/include/types/ndarray.hpp"

#include "pythonic/types/assignable.hpp"
#include "pythonic/types/empty_iterator.hpp"
#include "pythonic/types/attr.hpp"

#include "pythonic/__builtin__/ValueError.hpp"

#include "pythonic/utils/nested_container.hpp"
#include "pythonic/utils/shared_ref.hpp"
#include "pythonic/utils/reserve.hpp"
#include "pythonic/utils/int_.hpp"
#include "pythonic/utils/broadcast_copy.hpp"

#include "pythonic/types/slice.hpp"
#include "pythonic/types/tuple.hpp"
#include "pythonic/types/list.hpp"
#include "pythonic/types/raw_array.hpp"

#include "pythonic/numpy/bool_.hpp"
#include "pythonic/numpy/uint8.hpp"
#include "pythonic/numpy/int8.hpp"
#include "pythonic/numpy/uint16.hpp"
#include "pythonic/numpy/int16.hpp"
#include "pythonic/numpy/uint32.hpp"
#include "pythonic/numpy/int32.hpp"
#include "pythonic/numpy/uint64.hpp"
#include "pythonic/numpy/int64.hpp"
#include "pythonic/numpy/float32.hpp"
#include "pythonic/numpy/float64.hpp"
#include "pythonic/numpy/complex64.hpp"
#include "pythonic/numpy/complex128.hpp"

#include "pythonic/types/vectorizable_type.hpp"
#include "pythonic/types/numpy_op_helper.hpp"
#include "pythonic/types/numpy_expr.hpp"
#include "pythonic/types/numpy_texpr.hpp"
#include "pythonic/types/numpy_iexpr.hpp"
#include "pythonic/types/numpy_gexpr.hpp"
#include "pythonic/types/numpy_vexpr.hpp"
#include "pythonic/utils/numpy_traits.hpp"
#include "pythonic/utils/array_helper.hpp"

#include "pythonic/__builtin__/len.hpp"
#include "pythonic/operator_/iadd.hpp"
#include "pythonic/operator_/iand.hpp"
#include "pythonic/operator_/idiv.hpp"
#include "pythonic/operator_/imul.hpp"
#include "pythonic/operator_/ior.hpp"
#include "pythonic/operator_/isub.hpp"

#include <cassert>
#include <ostream>
#include <iterator>
#include <array>
#include <initializer_list>
#include <numeric>

PYTHONIC_NS_BEGIN

namespace types
{
  template <class pS, size_t... Is>
  array<long, std::tuple_size<pS>::value>
  make_strides(pS const &shape, utils::index_sequence<Is...>)
  {
    array<long, std::tuple_size<pS>::value> out;
    out[std::tuple_size<pS>::value - 1] = 1;
    std::initializer_list<long> _ = {
        (out[std::tuple_size<pS>::value - Is - 2] =
             out[std::tuple_size<pS>::value - Is - 1] *
             std::get<std::tuple_size<pS>::value - Is - 1>(shape))...};
    return out;
  }

  template <class pS>
  array<long, std::tuple_size<pS>::value> make_strides(pS const &shape)
  {
    return make_strides(
        shape, utils::make_index_sequence<std::tuple_size<pS>::value - 1>());
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, pS>>::iterator
  type_helper<ndarray<T, pS>>::make_iterator(ndarray<T, pS> &n, long i)
  {
    return {n, i};
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, pS>>::const_iterator
  type_helper<ndarray<T, pS>>::make_iterator(ndarray<T, pS> const &n, long i)
  {
    return {n, i};
  }

  template <class T, class pS>
  template <class S, class Iter>
  T *type_helper<ndarray<T, pS>>::initialize_from_iterable(S &shape, T *from,
                                                           Iter &&iter)
  {
    return type_helper<ndarray<T, pS> const &>::initialize_from_iterable(
        shape, from, std::forward<Iter>(iter));
  }

  template <class T, class pS>
  numpy_iexpr<ndarray<T, pS>>
  type_helper<ndarray<T, pS>>::get(ndarray<T, pS> &&self, long i)
  {
    return {std::move(self), i};
  }

  template <class T, class pS>
  long type_helper<ndarray<T, pS>>::step(ndarray<T, pS> const &self)
  {
    return std::accumulate(self.shape.begin() + 1, self.shape.end(), 1L,
                           std::multiplies<long>());
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, pS> const &>::iterator
  type_helper<ndarray<T, pS> const &>::make_iterator(ndarray<T, pS> &n, long i)
  {
    return {n, i};
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, pS> const &>::const_iterator
  type_helper<ndarray<T, pS> const &>::make_iterator(ndarray<T, pS> const &n,
                                                     long i)
  {
    return {n, i};
  }

  template <class T, class pS>
  template <class S, class Iter>
  T *type_helper<ndarray<T, pS> const &>::initialize_from_iterable(S &shape,
                                                                   T *from,
                                                                   Iter &&iter)
  {
    std::get<std::tuple_size<S>::value - std::tuple_size<pS>::value>(shape) =
        iter.size();
    for (auto content : iter)
      from = type_helper<ndarray<T, sutils::pop_tail_t<pS>> const &>::
          initialize_from_iterable(shape, from, content);
    return from;
  }

  template <class T, class pS>
  numpy_iexpr<ndarray<T, pS> const &>
  type_helper<ndarray<T, pS> const &>::get(ndarray<T, pS> const &self, long i)
  {
    return numpy_iexpr<ndarray<T, pS> const &>(self, i);
  }

  template <class T, class pS>
  long type_helper<ndarray<T, pS> const &>::step(ndarray<T, pS> const &self)
  {
    return std::accumulate(self.shape.begin() + 1, self.shape.end(), 1L,
                           std::multiplies<long>());
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, pshape<pS>>>::iterator
  type_helper<ndarray<T, pshape<pS>>>::make_iterator(ndarray<T, pshape<pS>> &n,
                                                     long i)
  {
    return n.buffer + i;
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, pshape<pS>>>::const_iterator
  type_helper<ndarray<T, pshape<pS>>>::make_iterator(
      ndarray<T, pshape<pS>> const &n, long i)
  {
    return n.buffer + i;
  }

  template <class T, class pS>
  template <class S, class Iter>
  T *type_helper<ndarray<T, pshape<pS>>>::initialize_from_iterable(S &shape,
                                                                   T *from,
                                                                   Iter &&iter)
  {
    std::get<std::tuple_size<S>::value - 1>(shape) = iter.size();
    return std::copy(iter.begin(), iter.end(), from);
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, pshape<pS>>>::type
  type_helper<ndarray<T, pshape<pS>>>::get(ndarray<T, pshape<pS>> &&self,
                                           long i)
  {
    return self.buffer[i];
  }

  template <class T, class pS>
  constexpr long
  type_helper<ndarray<T, pshape<pS>>>::step(ndarray<T, pshape<pS>> const &)
  {
    return 1;
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, pshape<pS>> const &>::iterator
  type_helper<ndarray<T, pshape<pS>> const &>::make_iterator(
      ndarray<T, pshape<pS>> &n, long i)
  {
    return n.buffer + i;
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, pshape<pS>> const &>::const_iterator
  make_iterator(ndarray<T, pshape<pS>> const &n, long i)
  {
    return n.buffer + i;
  }

  template <class T, class pS>
  template <class S, class Iter>
  T *type_helper<ndarray<T, pshape<pS>> const &>::initialize_from_iterable(
      S &shape, T *from, Iter &&iter)
  {
    std::get<std::tuple_size<S>::value - 1>(shape) = iter.size();
    return std::copy(iter.begin(), iter.end(), from);
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, pshape<pS>> const &>::type &
  type_helper<ndarray<T, pshape<pS>> const &>::get(
      ndarray<T, pshape<pS>> const &self, long i)
  {
    return self.buffer[i];
  }

  template <class T, class pS>
  constexpr long type_helper<ndarray<T, pshape<pS>> const &>::step(
      ndarray<T, pshape<pS>> const &)
  {
    return 1;
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, array<pS, 1>>>::iterator
      type_helper<ndarray<T, array<pS, 1>>>::make_iterator(
          ndarray<T, array<pS, 1>> &n, long i)
  {
    return n.buffer + i;
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, array<pS, 1>>>::const_iterator
      type_helper<ndarray<T, array<pS, 1>>>::make_iterator(
          ndarray<T, array<pS, 1>> const &n, long i)
  {
    return n.buffer + i;
  }

  template <class T, class pS>
  template <class S, class Iter>
  T *type_helper<ndarray<T, array<pS, 1>>>::initialize_from_iterable(
      S &shape, T *from, Iter &&iter)
  {
    std::get<std::tuple_size<S>::value - 1>(shape) = iter.size();
    return std::copy(iter.begin(), iter.end(), from);
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, array<pS, 1>>>::type
      type_helper<ndarray<T, array<pS, 1>>>::get(
          ndarray<T, array<pS, 1>> &&self, long i)
  {
    return self.buffer[i];
  }

  template <class T, class pS>
  constexpr long type_helper<ndarray<T, array<pS, 1>>>::step(
      ndarray<T, array<pS, 1>> const &)
  {
    return 1;
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, array<pS, 1>> const &>::iterator
      type_helper<ndarray<T, array<pS, 1>> const &>::make_iterator(
          ndarray<T, array<pS, 1>> &n, long i)
  {
    return n.buffer + i;
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, array<pS, 1>> const &>::const_iterator
      make_iterator(ndarray<T, array<pS, 1>> const &n, long i)
  {
    return n.buffer + i;
  }

  template <class T, class pS>
  template <class S, class Iter>
  T *type_helper<ndarray<T, array<pS, 1>> const &>::initialize_from_iterable(
      S &shape, T *from, Iter &&iter)
  {
    std::get<std::tuple_size<S>::value - 1>(shape) = iter.size();
    return std::copy(iter.begin(), iter.end(), from);
  }

  template <class T, class pS>
  typename type_helper<ndarray<T, array<pS, 1>> const &>::type &
      type_helper<ndarray<T, array<pS, 1>> const &>::get(
          ndarray<T, array<pS, 1>> const &self, long i)
  {
    return self.buffer[i];
  }

  template <class T, class pS>
  constexpr long type_helper<ndarray<T, array<pS, 1>> const &>::step(
      ndarray<T, array<pS, 1>> const &)
  {
    return 1;
  }

  template <size_t L>
  template <size_t M>
  long noffset<L>::operator()(array<long, M> const &strides,
                              array<long, M> const &indices) const
  {
    return noffset<L - 1>{}(strides, indices) + strides[M - L] * indices[M - L];
  }

  template <size_t L>
  template <size_t M, class pS>
  long noffset<L>::operator()(array<long, M> const &strides,
                              array<long, M> const &indices,
                              pS const &shape) const
  {
    return noffset<L - 1>{}(strides, indices, shape) +
           strides[M - L] * ((indices[M - L] < 0)
                                 ? indices[M - L] + std::get<M - L>(shape)
                                 : indices[M - L]);
  }

  template <>
  template <size_t M>
  long noffset<1>::operator()(array<long, M> const &,
                              array<long, M> const &indices) const
  {
    return indices[M - 1];
  }

  template <>
  template <size_t M, class pS>
  long noffset<1>::operator()(array<long, M> const &,
                              array<long, M> const &indices,
                              pS const &shape) const
  {
    return (indices[M - 1] < 0) ? indices[M - 1] + std::get<M - 1>(shape)
                                : indices[M - 1];
  }

  /* constructors */
  template <class T, class pS>
  ndarray<T, pS>::ndarray()
      : mem(utils::no_memory()), buffer(nullptr), _shape(), _strides()
  {
  }

  /* from other memory */
  template <class T, class pS>
  ndarray<T, pS>::ndarray(utils::shared_ref<raw_array<T>> const &mem,
                          pS const &shape)
      : mem(mem), buffer(mem->data), _shape(shape),
        _strides(make_strides(shape))
  {
  }
  template <class T, class pS>
  ndarray<T, pS>::ndarray(utils::shared_ref<raw_array<T>> &&mem,
                          pS const &shape)
      : mem(std::move(mem)), buffer(this->mem->data), _shape(shape),
        _strides(make_strides(shape))
  {
  }

  /* from other array */
  template <class T, class pS>
  template <class Tp, class pSp>
  ndarray<T, pS>::ndarray(ndarray<Tp, pSp> const &other)
      : mem(other.flat_size()), buffer(mem->data), _shape(other._shape),
        _strides(other._strides)
  {
    std::copy(other.fbegin(), other.fend(), fbegin());
  }

  /* from a seed */
  template <class T, class pS>
  ndarray<T, pS>::ndarray(pS const &shape, none_type init)
      : mem(sutils::prod(shape)), buffer(mem->data), _shape(shape),
        _strides(make_strides(shape))
  {
  }

  template <class T, class pS>
  ndarray<T, pS>::ndarray(pS const &shape, T init)
      : ndarray(shape, none_type())
  {
    std::fill(fbegin(), fend(), init);
  }

  /* from a foreign pointer */
  template <class T, class pS>
  template <class S>
  ndarray<T, pS>::ndarray(T *data, S const *pshape, ownership o)
      : mem(data, o), buffer(mem->data), _shape(pshape)
  {
    _strides = make_strides(_shape);
  }
  template <class T, class pS>
  ndarray<T, pS>::ndarray(T *data, pS const &pshape, ownership o)
      : mem(data, o), buffer(mem->data), _shape(pshape)
  {
    _strides = make_strides(_shape);
  }

#ifdef ENABLE_PYTHON_MODULE
  template <class T, class pS>
  template <class S>
  ndarray<T, pS>::ndarray(T *data, S const *pshape, PyObject *obj_ptr)
      : ndarray(data, pshape, ownership::external)
  {
    mem.external(obj_ptr); // mark memory as external to decref at the end of
                           // its lifetime
  }
  template <class T, class pS>
  ndarray<T, pS>::ndarray(T *data, pS const &pshape, PyObject *obj_ptr)
      : ndarray(data, pshape, ownership::external)
  {
    mem.external(obj_ptr); // mark memory as external to decref at the end of
                           // its lifetime
  }

#endif

  template <class T, class pS>
  template <class Iterable, class>
  ndarray<T, pS>::ndarray(Iterable &&iterable)
      : mem(utils::nested_container_size<Iterable>::flat_size(
            std::forward<Iterable>(iterable))),
        buffer(mem->data), _shape()
  {
    type_helper<ndarray>::initialize_from_iterable(
        _shape, mem->data, std::forward<Iterable>(iterable));
    _strides = make_strides(_shape);
  }

  /* from a  numpy expression */
  template <class T, class pS>
  template <class E>
  void ndarray<T, pS>::initialize_from_expr(E const &expr)
  {
    assert(buffer);
    utils::broadcast_copy<ndarray &, E, value, 0,
                          is_vectorizable && E::is_vectorizable &&
                              std::is_same<dtype, typename E::dtype>::value>(
        *this, expr);
  }

  template <class T, class pS>
  template <class Op, class... Args>
  ndarray<T, pS>::ndarray(numpy_expr<Op, Args...> const &expr)
      : mem(expr.flat_size()), buffer(mem->data), _shape(expr.shape()),
        _strides(make_strides(_shape))
  {
    initialize_from_expr(expr);
  }

  template <class T, class pS>
  template <class Arg>
  ndarray<T, pS>::ndarray(numpy_texpr<Arg> const &expr)
      : mem(expr.flat_size()), buffer(mem->data), _shape(expr.shape()),
        _strides(make_strides(_shape))
  {
    initialize_from_expr(expr);
  }

  template <class T, class pS>
  template <class Arg>
  ndarray<T, pS>::ndarray(numpy_texpr_2<Arg> const &expr)
      : mem(expr.flat_size()), buffer(mem->data), _shape(expr.shape()),
        _strides(make_strides(_shape))
  {
    initialize_from_expr(expr);
  }

  template <class T, class pS>
  template <class Arg, class... S>
  ndarray<T, pS>::ndarray(numpy_gexpr<Arg, S...> const &expr)
      : mem(expr.flat_size()), buffer(mem->data), _shape(expr.shape()),
        _strides(make_strides(_shape))
  {
    initialize_from_expr(expr);
  }

  template <class T, class pS>
  template <class Arg>
  ndarray<T, pS>::ndarray(numpy_iexpr<Arg> const &expr)
      : mem(expr.flat_size()), buffer(mem->data), _shape(expr.shape()),
        _strides(make_strides(_shape))
  {
    initialize_from_expr(expr);
  }

  template <class T, class pS>
  template <class Arg, class F>
  ndarray<T, pS>::ndarray(numpy_vexpr<Arg, F> const &expr)
      : mem(expr.flat_size()), buffer(mem->data), _shape(expr.shape()),
        _strides(make_strides(_shape))
  {
    initialize_from_expr(expr);
  }

  /* update operators */

  template <class T, class pS>
  template <class Op, class Expr>
  ndarray<T, pS> &ndarray<T, pS>::update_(Expr const &expr)
  {
    using BExpr =
        typename std::conditional<std::is_scalar<Expr>::value,
                                  broadcast<Expr, T>, Expr const &>::type;
    BExpr bexpr = expr;
    utils::broadcast_update<
        Op, ndarray &, BExpr, value,
        value - (std::is_scalar<Expr>::value + utils::dim_of<Expr>::value),
        is_vectorizable &&
            types::is_vectorizable<typename std::remove_cv<
                typename std::remove_reference<BExpr>::type>::type>::value &&
            std::is_same<dtype, typename dtype_of<typename std::decay<
                                    BExpr>::type>::type>::value>(*this, bexpr);
    return *this;
  }

  template <class T, class pS>
  template <class Expr>
  ndarray<T, pS> &ndarray<T, pS>::operator+=(Expr const &expr)
  {
    return update_<pythonic::operator_::functor::iadd>(expr);
  }

  template <class T, class pS>
  template <class Expr>
  ndarray<T, pS> &ndarray<T, pS>::operator-=(Expr const &expr)
  {
    return update_<pythonic::operator_::functor::isub>(expr);
  }

  template <class T, class pS>
  template <class Expr>
  ndarray<T, pS> &ndarray<T, pS>::operator*=(Expr const &expr)
  {
    return update_<pythonic::operator_::functor::imul>(expr);
  }

  template <class T, class pS>
  template <class Expr>
  ndarray<T, pS> &ndarray<T, pS>::operator/=(Expr const &expr)
  {
    return update_<pythonic::operator_::functor::idiv>(expr);
  }

  template <class T, class pS>
  template <class Expr>
  ndarray<T, pS> &ndarray<T, pS>::operator&=(Expr const &expr)
  {
    return update_<pythonic::operator_::functor::iand>(expr);
  }

  template <class T, class pS>
  template <class Expr>
  ndarray<T, pS> &ndarray<T, pS>::operator|=(Expr const &expr)
  {
    return update_<pythonic::operator_::functor::ior>(expr);
  }

  /* element indexing
   * differentiate const from non const, && r-value from l-value
   * */

  template <class T, class pS>
  T &ndarray<T, pS>::fast(array<long, value> const &indices)
  {
    return *(buffer + noffset<std::tuple_size<pS>::value>{}(_strides, indices));
  }

  template <class T, class pS>
  T ndarray<T, pS>::fast(array<long, value> const &indices) const
  {
    return *(buffer + noffset<std::tuple_size<pS>::value>{}(_strides, indices));
  }

  template <class T, class pS>
  template <size_t M>
  auto ndarray<T, pS>::fast(array<long, M> const &indices) const
      & -> decltype(nget<M - 1>().fast(*this, indices))
  {
    return nget<M - 1>().fast(*this, indices);
  }

  template <class T, class pS>
      template <size_t M>
      auto ndarray<T, pS>::fast(array<long, M> const &indices) &&
      -> decltype(nget<M - 1>().fast(std::move(*this), indices))
  {
    return nget<M - 1>().fast(std::move(*this), indices);
  }

  template <class T, class pS>
  T const &ndarray<T, pS>::operator[](array<long, value> const &indices) const
  {
    return *(buffer +
             noffset<std::tuple_size<pS>::value>{}(_strides, indices, _shape));
  }

  template <class T, class pS>
  T &ndarray<T, pS>::operator[](array<long, value> const &indices)
  {
    return *(buffer +
             noffset<std::tuple_size<pS>::value>{}(_strides, indices, _shape));
  }

  template <class T, class pS>
  template <size_t M>
  auto ndarray<T, pS>::operator[](array<long, M> const &indices) const
      & -> decltype(nget<M - 1>()(*this, indices))
  {
    return nget<M - 1>()(*this, indices);
  }

  template <class T, class pS>
      template <size_t M>
      auto ndarray<T, pS>::operator[](array<long, M> const &indices) &&
      -> decltype(nget<M - 1>()(std::move(*this), indices))
  {
    return nget<M - 1>()(std::move(*this), indices);
  }

  template <class T, class pS>
  template <class Ty0, class Ty1, class... Tys>
  auto ndarray<T, pS>::
  operator[](std::tuple<Ty0, Ty1, Tys...> const &indices) const ->
      typename std::enable_if<!std::is_same<Ty0, long>::value &&
                                  !is_numexpr_arg<Ty0>::value,
                              decltype((*this)[to_array<long>(indices)])>::type
  {
    return (*this)[to_array<long>(indices)];
  }

#ifdef USE_BOOST_SIMD
  template <class T, class pS>
  template <class vectorizer>
  typename ndarray<T, pS>::simd_iterator
      ndarray<T, pS>::vbegin(vectorizer) const
  {
    return {buffer};
  }

  template <class T, class pS>
  template <class vectorizer>
  typename ndarray<T, pS>::simd_iterator ndarray<T, pS>::vend(vectorizer) const
  {
    using vector_type = typename boost::simd::pack<dtype>;
    static const std::size_t vector_size = vector_type::static_size;
    return {buffer + long(std::get<0>(_shape) / vector_size * vector_size)};
  }

#endif

  /* slice indexing */
  template <class T, class pS>
  ndarray<T, sutils::push_front_t<pS, std::integral_constant<long, 1>>>
      ndarray<T, pS>::operator[](none_type) const
  {
    sutils::push_front_t<pS, std::integral_constant<long, 1>> new_shape;
    sutils::copy_shape<1, -1>(
        new_shape, _shape,
        utils::make_index_sequence<std::tuple_size<pS>::value>());
    return reshape(new_shape);
  }

  template <class T, class pS>
  numpy_gexpr<ndarray<T, pS> const &, normalized_slice> ndarray<T, pS>::
  operator[](slice const &s) const &
  {
    return make_gexpr(*this, s);
  }

  template <class T, class pS>
  numpy_gexpr<ndarray<T, pS>, normalized_slice> ndarray<T, pS>::
  operator[](slice const &s) &&
  {
    return make_gexpr(std::move(*this), s);
  }

  template <class T, class pS>
  numpy_gexpr<ndarray<T, pS> const &, contiguous_normalized_slice>
      ndarray<T, pS>::operator[](contiguous_slice const &s) const
  {
    return make_gexpr(*this, s);
  }

  template <class T, class pS>
  long ndarray<T, pS>::size() const
  {
    return std::get<0>(_shape);
  }

  /* extended slice indexing */
  template <class T, class pS>
  template <class S0, class... S>
  auto ndarray<T, pS>::operator()(S0 const &s0, S const &... s) const
      & -> decltype(extended_slice<count_new_axis<S0, S...>::value>{}((*this),
                                                                      s0, s...))
  {
    return extended_slice<count_new_axis<S0, S...>::value>{}((*this), s0, s...);
  }

  template <class T, class pS>
      template <class S0, class... S>
      auto ndarray<T, pS>::operator()(S0 const &s0, S const &... s) &&
      -> decltype(extended_slice<count_new_axis<S0, S...>::value>{}(
          std::move(*this), s0, s...))
  {
    return extended_slice<count_new_axis<S0, S...>::value>{}(std::move(*this),
                                                             s0, s...);
  }

  /* element filtering */
  template <class T, class pS>
  template <class F> // indexing through an array of boolean -- a mask
  typename std::enable_if<
      is_numexpr_arg<F>::value &&
          std::is_same<bool, typename F::dtype>::value && F::value == 1 &&
          !is_pod_array<F>::value,
      numpy_vexpr<ndarray<T, pS>, ndarray<long, pshape<long>>>>::type
  ndarray<T, pS>::fast(F const &filter) const
  {
    long sz = std::get<0>(filter.shape());
    long *raw = (long *)malloc(sz * sizeof(long));
    long n = 0;
    for (long i = 0; i < sz; ++i)
      if (filter.fast(i))
        raw[n++] = i;
    // realloc(raw, n * sizeof(long));
    return this->fast(ndarray<long, pshape<long>>(raw, pshape<long>(n),
                                                  types::ownership::owned));
  }

  template <class T, class pS>
  template <class F> // indexing through an array of boolean -- a mask
  typename std::enable_if<
      is_numexpr_arg<F>::value &&
          std::is_same<bool, typename F::dtype>::value && F::value == 1 &&
          !is_pod_array<F>::value,
      numpy_vexpr<ndarray<T, pS>, ndarray<long, pshape<long>>>>::type
      ndarray<T, pS>::
      operator[](F const &filter) const
  {
    return fast(filter);
  }
  template <class T, class pS>
  template <class F> // indexing through an array of boolean -- a mask
  typename std::enable_if<
      is_numexpr_arg<F>::value &&
          std::is_same<bool, typename F::dtype>::value && F::value != 1 &&
          !is_pod_array<F>::value,
      numpy_vexpr<ndarray<T, pshape<long>>, ndarray<long, pshape<long>>>>::type
  ndarray<T, pS>::fast(F const &filter) const
  {
    return flat()[ndarray<typename F::dtype, typename F::shape_t>(filter)
                      .flat()];
  }

  template <class T, class pS>
  template <class F> // indexing through an array of boolean -- a mask
  typename std::enable_if<
      is_numexpr_arg<F>::value &&
          std::is_same<bool, typename F::dtype>::value && F::value != 1 &&
          !is_pod_array<F>::value,
      numpy_vexpr<ndarray<T, pshape<long>>, ndarray<long, pshape<long>>>>::type
      ndarray<T, pS>::
      operator[](F const &filter) const
  {
    return fast(filter);
  }

  template <class T, class pS>
  template <class F> // indexing through an array of indices -- a view
  typename std::enable_if<is_numexpr_arg<F>::value &&
                              !is_array_index<F>::value &&
                              !std::is_same<bool, typename F::dtype>::value &&
                              !is_pod_array<F>::value,
                          numpy_vexpr<ndarray<T, pS>, F>>::type ndarray<T, pS>::
  operator[](F const &filter) const
  {
    return {*this, filter};
  }

  template <class T, class pS>
  template <class F> // indexing through an array of indices -- a view
  typename std::enable_if<is_numexpr_arg<F>::value &&
                              !is_array_index<F>::value &&
                              !std::is_same<bool, typename F::dtype>::value &&
                              !is_pod_array<F>::value,
                          numpy_vexpr<ndarray<T, pS>, F>>::type
  ndarray<T, pS>::fast(F const &filter) const
  {
    return {*this, filter};
  }

  template <class T, class pS>
  template <class L, class Ty, class... Tys>
  auto ndarray<T, pS>::
  operator[](std::tuple<L, Ty, Tys...> const &indices) const ->
      typename std::enable_if<is_numexpr_arg<L>::value,
                              decltype((*this)[tuple_tail(indices)])>::type
  {
    return ndarray<T, pS>
    {
      (*this)[std::get<0>(indices)]
    }
    [tuple_tail(indices)];
  }

  /* through iterators */
  template <class T, class pS>
  typename ndarray<T, pS>::iterator ndarray<T, pS>::begin()
  {
    return type_helper<ndarray>::make_iterator(*this, 0);
  }

  template <class T, class pS>
  typename ndarray<T, pS>::const_iterator ndarray<T, pS>::begin() const
  {
    return type_helper<ndarray>::make_iterator(*this, 0);
  }

  template <class T, class pS>
  typename ndarray<T, pS>::iterator ndarray<T, pS>::end()
  {
    return type_helper<ndarray>::make_iterator(*this, std::get<0>(_shape));
  }

  template <class T, class pS>
  typename ndarray<T, pS>::const_iterator ndarray<T, pS>::end() const
  {
    return type_helper<ndarray>::make_iterator(*this, std::get<0>(_shape));
  }

  template <class T, class pS>
  typename ndarray<T, pS>::const_flat_iterator ndarray<T, pS>::fbegin() const
  {
    return buffer;
  }

  template <class T, class pS>
  typename ndarray<T, pS>::const_flat_iterator ndarray<T, pS>::fend() const
  {
    return buffer + flat_size();
  }

  template <class T, class pS>
  typename ndarray<T, pS>::flat_iterator ndarray<T, pS>::fbegin()
  {
    return buffer;
  }

  template <class T, class pS>
  typename ndarray<T, pS>::flat_iterator ndarray<T, pS>::fend()
  {
    return buffer + flat_size();
  }

  /* member functions */
  template <class T, class pS>
  long ndarray<T, pS>::flat_size() const
  {
    return sutils::prod(_shape);
  }
  template <class T, class pS>
  bool ndarray<T, pS>::may_overlap(ndarray const &expr) const
  {
    return id() == expr.id();
  }

  template <class T, class pS>
  template <class qS>
  ndarray<T, qS> ndarray<T, pS>::reshape(qS const &shape) const &
  {
    return {mem, shape};
  }

  template <class T, class pS>
  template <class qS>
  ndarray<T, qS> ndarray<T, pS>::reshape(qS const &shape) &&
  {
    return {std::move(mem), shape};
  }

  template <class T, class pS>
  ndarray<T, pS>::operator bool() const
  {
    if (sutils::any_of(_shape, [](long n) { return n != 1; }))
      throw ValueError("The truth value of an array with more than one element "
                       "is ambiguous. Use a.any() or a.all()");
    return *buffer;
  }

  template <class T, class pS>
  ndarray<T, pshape<long>> ndarray<T, pS>::flat() const
  {
    return {mem, array<long, 1>{{flat_size()}}};
  }

  template <class T, class pS>
  ndarray<T, pS> ndarray<T, pS>::copy() const
  {
    ndarray<T, pS> res(_shape, __builtin__::None);
    std::copy(fbegin(), fend(), res.fbegin());
    return res;
  }

  template <class T, class pS>
  intptr_t ndarray<T, pS>::id() const
  {
    return reinterpret_cast<intptr_t>(&(*mem));
  }

  template <class T, class pS>
  pS const &ndarray<T, pS>::shape() const
  {
    return _shape;
  }

  /* pretty printing { */
  namespace impl
  {
    template <class T, class pS>
    int get_spacing(ndarray<T, pS> const &e)
    {
      std::ostringstream oss;
      if (e.flat_size())
        oss << *std::max_element(e.fbegin(), e.fend());
      return oss.str().length();
    }
    template <class T, class pS>
    int get_spacing(ndarray<std::complex<T>, pS> const &e)
    {
      std::ostringstream oss;
      if (e.flat_size())
        oss << *e.fbegin();
      return oss.str().length() + 2;
    }
  }

  template <class T, class pS>
  std::ostream &operator<<(std::ostream &os, ndarray<T, pS> const &e)
  {
    std::array<long, std::tuple_size<pS>::value> strides;
    auto shape = e.shape();
    strides[std::tuple_size<pS>::value - 1] =
        std::get<std::tuple_size<pS>::value - 1>(shape);
    if (strides[std::tuple_size<pS>::value - 1] == 0)
      return os << "[]";
    auto ashape = sutils::array(shape);
    std::transform(strides.rbegin(), strides.rend() - 1, ashape.rbegin() + 1,
                   strides.rbegin() + 1, std::multiplies<long>());
    size_t depth = std::tuple_size<pS>::value;
    int step = -1;
    int size = impl::get_spacing(e);
    auto iter = e.fbegin();
    int max_modulo = 1000;

    os << "[";
    if (std::get<0>(shape) != 0)
      do {
        if (depth == 1) {
          os.width(size);
          os << *iter++;
          for (int i = 1; i < std::get<std::tuple_size<pS>::value - 1>(shape);
               i++) {
            os.width(size + 1);
            os << *iter++;
          }
          step = 1;
          depth++;
          max_modulo = std::lower_bound(
                           strides.begin(), strides.end(), iter - e.buffer,
                           [](int comp, int val) { return val % comp != 0; }) -
                       strides.begin();
        } else if (max_modulo + depth == std::tuple_size<pS>::value + 1) {
          depth--;
          step = -1;
          os << "]";
          for (size_t i = 0; i < depth; i++)
            os << std::endl;
          for (size_t i = 0; i < std::tuple_size<pS>::value - depth; i++)
            os << " ";
          os << "[";
        } else {
          depth += step;
          if (step == 1)
            os << "]";
          else
            os << "[";
        }
      } while (depth != std::tuple_size<pS>::value + 1);

    return os << "]";
  }

  template <class E>
  typename std::enable_if<is_array<E>::value, std::ostream &>::type
  operator<<(std::ostream &os, E const &e)
  {
    return os << ndarray<typename E::dtype, typename E::shape_t>{e};
  }

  /* } */
  template <class T>
  template <class pS>
  list<T> &list<T>::operator=(ndarray<T, pshape<pS>> const &other)
  {
    data = utils::shared_ref<T>(other.begin(), other.end());
    return *this;
  }
}
PYTHONIC_NS_END

/* std::get overloads */
namespace std
{

  template <size_t I, class E>
  auto get(E &&a) -> typename std::enable_if<
      pythonic::types::is_array<typename std::remove_cv<
          typename std::remove_reference<E>::type>::type>::value,
      decltype(std::forward<E>(a)[I])>::type
  {
    return std::forward<E>(a)[I];
  }
}

/* pythran attribute system { */
#include "pythonic/numpy/transpose.hpp"
PYTHONIC_NS_BEGIN
namespace types
{
  namespace __ndarray
  {

    template <class E>
    auto getattr<attr::SHAPE, E>::operator()(E const &a)
        -> decltype(sutils::array(a.shape()))
    {
      return sutils::array(a.shape());
    }

    template <class E>
    long getattr<attr::NDIM, E>::operator()(E const &a)
    {
      return E::value;
    }

    template <class E>
    array<long, E::value> getattr<attr::STRIDES, E>::operator()(E const &a)
    {
      array<long, E::value> strides;
      strides[E::value - 1] = sizeof(typename E::dtype);
      auto shape = sutils::array(a.shape());
      std::transform(strides.rbegin(), strides.rend() - 1, shape.rbegin(),
                     strides.rbegin() + 1, std::multiplies<long>());
      return strides;
    }

    template <class E>
    long getattr<attr::SIZE, E>::operator()(E const &a)
    {
      return a.flat_size();
    }

    template <class E>
    long getattr<attr::ITEMSIZE, E>::operator()(E const &a)
    {
      return sizeof(typename E::dtype);
    }

    template <class E>
    long getattr<attr::NBYTES, E>::operator()(E const &a)
    {
      return a.flat_size() * sizeof(typename E::dtype);
    }

    template <class E>
    auto getattr<attr::FLAT, E>::operator()(E const &a) -> decltype(a.flat())
    {
      return a.flat();
    }

    template <class E>
    dtype_t<typename E::dtype> getattr<attr::DTYPE, E>::operator()(E const &a)
    {
      return {};
    }

    namespace
    {
      template <size_t N>
      template <class E, class... S>
      auto _build_gexpr<N>::operator()(E const &a, S const &... slices)
          -> decltype(_build_gexpr<N - 1>{}(a, contiguous_slice(), slices...))
      {
        return _build_gexpr<N - 1>{}(a, contiguous_slice(0, a.size()),
                                     slices...);
      }

      template <class E, class... S>
      numpy_gexpr<E, normalize_t<S>...> _build_gexpr<1>::
      operator()(E const &a, S const &... slices)
      {
        return E(a)(slices...);
      }
    }

    template <class E>
    E getattr<attr::REAL, E>::make_real(E const &a, utils::int_<0>)
    {
      return a;
    }

    template <class E>
    auto getattr<attr::REAL, E>::make_real(E const &a, utils::int_<1>)
        -> decltype(_build_gexpr<E::value>{}(
            ndarray<typename types::is_complex<typename E::dtype>::type,
                    types::array<long, E::value>>{},
            slice()))
    {
      using stype = typename types::is_complex<typename E::dtype>::type;
      auto new_shape = sutils::array(a.shape());
      std::get<E::value - 1>(new_shape) *= 2;
      // this is tricky && dangerous!
      auto translated_mem =
          reinterpret_cast<utils::shared_ref<raw_array<stype>> const &>(a.mem);
      ndarray<stype, types::array<long, E::value>> translated{translated_mem,
                                                              new_shape};
      return _build_gexpr<E::value>{}(
          translated, slice{0, std::get<E::value - 1>(new_shape), 2});
    }

    template <class E>
    auto getattr<attr::REAL, E>::operator()(E const &a)
        -> decltype(this->make_real(
            a, utils::int_<types::is_complex<typename E::dtype>::value>{}))
    {
      return make_real(
          a, utils::int_<types::is_complex<typename E::dtype>::value>{});
    }

    template <class E>
    auto getattr<attr::REAL, types::numpy_texpr<E>>::
    operator()(types::numpy_texpr<E> const &a) -> decltype(
        types::numpy_texpr<decltype(getattr<attr::REAL, E>{}(a.arg))>{
            getattr<attr::REAL, E>{}(a.arg)})
    {
      auto ta = getattr<attr::REAL, E>{}(a.arg);
      return types::numpy_texpr<decltype(ta)>{ta};
    }

    template <class E>
    types::ndarray<typename E::dtype, typename E::shape_t>
    getattr<attr::IMAG, E>::make_imag(E const &a, utils::int_<0>)
    {
      // cannot use numpy.zero: forward declaration issue
      return {
          (typename E::dtype *)calloc(a.flat_size(), sizeof(typename E::dtype)),
          a.shape(), types::ownership::owned};
    }

    template <class E>
    auto getattr<attr::IMAG, E>::make_imag(E const &a, utils::int_<1>)
        -> decltype(_build_gexpr<E::value>{}(
            ndarray<typename types::is_complex<typename E::dtype>::type,
                    types::array<long, E::value>>{},
            slice()))
    {
      using stype = typename types::is_complex<typename E::dtype>::type;
      auto new_shape = sutils::array(a.shape());
      std::get<E::value - 1>(new_shape) *= 2;
      // this is tricky && dangerous!
      auto translated_mem =
          reinterpret_cast<utils::shared_ref<raw_array<stype>> const &>(a.mem);
      ndarray<stype, types::array<long, E::value>> translated{translated_mem,
                                                              new_shape};
      return _build_gexpr<E::value>{}(
          translated, slice{1, std::get<E::value - 1>(new_shape), 2});
    }

    template <class E>
    auto getattr<attr::IMAG, E>::operator()(E const &a)
        -> decltype(this->make_imag(
            a, utils::int_<types::is_complex<typename E::dtype>::value>{}))
    {
      return make_imag(
          a, utils::int_<types::is_complex<typename E::dtype>::value>{});
    }

    template <class E>
    auto getattr<attr::IMAG, types::numpy_texpr<E>>::
    operator()(types::numpy_texpr<E> const &a) -> decltype(
        types::numpy_texpr<decltype(getattr<attr::IMAG, E>{}(a.arg))>{
            getattr<attr::IMAG, E>{}(a.arg)})
    {
      auto ta = getattr<attr::IMAG, E>{}(a.arg);
      return types::numpy_texpr<decltype(ta)>{ta};
    }
  }
}
namespace __builtin__
{
  template <int I, class T, class pS>
  auto getattr(types::ndarray<T, pS> const &f)
      -> decltype(types::__ndarray::getattr<I, types::ndarray<T, pS>>()(f))
  {
    return types::__ndarray::getattr<I, types::ndarray<T, pS>>()(f);
  }

  template <int I, class O, class... Args>
  auto getattr(types::numpy_expr<O, Args...> const &f) -> decltype(
      types::__ndarray::getattr<I, types::numpy_expr<O, Args...>>()(f))
  {
    return types::__ndarray::getattr<I, types::numpy_expr<O, Args...>>()(f);
  }

  template <int I, class A, class... S>
  auto getattr(types::numpy_gexpr<A, S...> const &f) -> decltype(
      types::__ndarray::getattr<I, types::numpy_gexpr<A, S...>>()(f))
  {
    return types::__ndarray::getattr<I, types::numpy_gexpr<A, S...>>()(f);
  }

  template <int I, class A>
  auto getattr(types::numpy_iexpr<A> const &f)
      -> decltype(types::__ndarray::getattr<I, types::numpy_iexpr<A>>()(f))
  {
    return types::__ndarray::getattr<I, types::numpy_iexpr<A>>()(f);
  }

  template <int I, class A>
  auto getattr(types::numpy_texpr<A> const &f)
      -> decltype(types::__ndarray::getattr<I, types::numpy_texpr<A>>()(f))
  {
    return types::__ndarray::getattr<I, types::numpy_texpr<A>>()(f);
  }

  template <int I, class T, class F>
  auto getattr(types::numpy_vexpr<T, F> const &f)
      -> decltype(types::__ndarray::getattr<I, types::numpy_vexpr<T, F>>()(f))
  {
    return types::__ndarray::getattr<I, types::numpy_vexpr<T, F>>()(f);
  }
}
PYTHONIC_NS_END

/* } */

#include "pythonic/types/numpy_operators.hpp"

#ifdef ENABLE_PYTHON_MODULE

PYTHONIC_NS_BEGIN

namespace details
{
  constexpr int signed_int_types[] = {0, NPY_INT8, NPY_INT16, 0, NPY_INT32, 0,
                                      0, 0,        NPY_INT64};
  constexpr int unsigned_int_types[] = {
      0, NPY_UINT8, NPY_UINT16, 0, NPY_UINT32, 0, 0, 0, NPY_UINT64};
}

template <class T>
struct c_type_to_numpy_type
    : c_type_to_numpy_type<decltype(std::declval<T>()())> {
};

template <>
struct c_type_to_numpy_type<double> : std::integral_constant<int, NPY_DOUBLE> {
};

template <>
struct c_type_to_numpy_type<float> : std::integral_constant<int, NPY_FLOAT> {
};

template <>
struct c_type_to_numpy_type<std::complex<float>>
    : std::integral_constant<int, NPY_CFLOAT> {
};

template <>
struct c_type_to_numpy_type<std::complex<double>>
    : std::integral_constant<int, NPY_CDOUBLE> {
};

template <>
struct c_type_to_numpy_type<signed long long> {
  static const int value = details::signed_int_types[sizeof(signed long long)];
};

template <>
struct c_type_to_numpy_type<unsigned long long> {
  static const int value =
      details::unsigned_int_types[sizeof(unsigned long long)];
};

template <>
struct c_type_to_numpy_type<signed long> {
  static const int value = details::signed_int_types[sizeof(signed long)];
};

template <>
struct c_type_to_numpy_type<unsigned long> {
  static const int value = details::unsigned_int_types[sizeof(unsigned long)];
};

template <>
struct c_type_to_numpy_type<signed int> {
  static const int value = details::signed_int_types[sizeof(signed int)];
};

template <>
struct c_type_to_numpy_type<unsigned int> {
  static const int value = details::unsigned_int_types[sizeof(unsigned int)];
};

template <>
struct c_type_to_numpy_type<signed short> {
  static const int value = details::signed_int_types[sizeof(signed short)];
};

template <>
struct c_type_to_numpy_type<unsigned short> {
  static const int value = details::unsigned_int_types[sizeof(unsigned short)];
};

template <>
struct c_type_to_numpy_type<signed char> {
  static const int value = details::signed_int_types[sizeof(signed char)];
};

template <>
struct c_type_to_numpy_type<unsigned char> {
  static const int value = details::unsigned_int_types[sizeof(unsigned char)];
};

template <>
struct c_type_to_numpy_type<bool> {
  static const int value = NPY_BOOL;
};

template <class T>
struct c_type_to_numpy_type<boost::simd::logical<T>> {
  static const int value = NPY_BOOL;
};

/* wrapper around Python array creation
 * its purpose is to hide the difference between the shape stored in pythran
 * (aka long) && the shape stored in numpy (aka npy_intp)
 * it should work (with an extra copy) on 32 bit architecture && without copy
 * on 64 bits architecture
 */
template <class T, size_t N>
struct pyarray_new {

  static_assert(!std::is_same<T, npy_intp>::value, "correctly specialized");

  PyObject *from_descr(PyTypeObject *subtype, PyArray_Descr *descr, T *dims,
                       void *data, int flags, PyObject *obj)
  {
    npy_intp shape[N];
    std::copy(dims, dims + N, shape);
    return pyarray_new<npy_intp, N>{}.from_descr(subtype, descr, shape, data,
                                                 flags, obj);
  }
  PyObject *from_data(T *dims, int typenum, void *data)
  {
    npy_intp shape[N];
    std::copy(dims, dims + N, shape);
    return pyarray_new<npy_intp, N>{}.from_data(shape, typenum, data);
  }
};

template <size_t N>
struct pyarray_new<npy_intp, N> {

  PyObject *from_descr(PyTypeObject *subtype, PyArray_Descr *descr,
                       npy_intp *dims, void *data, int flags, PyObject *obj)
  {
    return PyArray_NewFromDescr(subtype, descr, N, dims, nullptr, data, flags,
                                obj);
  }

  PyObject *from_data(npy_intp *dims, int typenum, void *data)
  {

    return PyArray_SimpleNewFromData(N, dims, typenum, data);
  }
};

template <class T, class pS>
PyObject *
to_python<types::ndarray<T, pS>>::convert(types::ndarray<T, pS> const &cn,
                                          bool transpose)
{
  types::ndarray<T, pS> &n = const_cast<types::ndarray<T, pS> &>(cn);
  if (PyObject *p = n.mem.get_foreign()) {
    PyArrayObject *arr = reinterpret_cast<PyArrayObject *>(p);
    auto const *pshape = PyArray_DIMS(arr);
    Py_INCREF(p);

    // handle complex trick :-/
    if ((long)sizeof(T) != PyArray_ITEMSIZE((PyArrayObject *)(arr))) {
      arr = (PyArrayObject *)PyArray_View(
          (PyArrayObject *)(arr),
          PyArray_DescrFromType(c_type_to_numpy_type<T>::value), nullptr);
    }

    if (sutils::equals(n.shape(), pshape)) {
      if (transpose && !(PyArray_FLAGS(arr) & NPY_ARRAY_F_CONTIGUOUS))
        return PyArray_Transpose(arr, nullptr);
      else
        return p;
    } else if (sutils::requals(n.shape(), pshape)) {
      if (transpose)
        return p;
      else
        return PyArray_Transpose(arr, nullptr);
    } else {
      Py_INCREF(PyArray_DESCR(arr));
      auto array = sutils::array(n._shape);
      auto *res = pyarray_new<long, std::tuple_size<pS>::value>{}.from_descr(
          Py_TYPE(arr), PyArray_DESCR(arr), array.data(), PyArray_DATA(arr),
          PyArray_FLAGS(arr) & ~NPY_ARRAY_OWNDATA, p);
      if (transpose && (PyArray_FLAGS(arr) & NPY_ARRAY_F_CONTIGUOUS))
        return PyArray_Transpose(reinterpret_cast<PyArrayObject *>(arr),
                                 nullptr);
      else
        return res;
    }
  } else {
    auto array = sutils::array(n._shape);
    PyObject *result =
        pyarray_new<long, std::tuple_size<pS>::value>{}.from_data(
            array.data(), c_type_to_numpy_type<T>::value, n.buffer);
    n.mark_memory_external(result);
    Py_INCREF(result);
    if (!result)
      return nullptr;
    PyArray_ENABLEFLAGS(reinterpret_cast<PyArrayObject *>(result),
                        NPY_ARRAY_OWNDATA);
    if (transpose)
      return PyArray_Transpose(reinterpret_cast<PyArrayObject *>(result),
                               nullptr);
    else
      return result;
  }
}

template <class Arg>
PyObject *
to_python<types::numpy_iexpr<Arg>>::convert(types::numpy_iexpr<Arg> const &v)
{
  return ::to_python(
      types::ndarray<typename types::numpy_iexpr<Arg>::dtype,
                     typename types::numpy_iexpr<Arg>::shape_t>(v));
}

template <class Arg, class... S>
PyObject *to_python<types::numpy_gexpr<Arg, S...>>::convert(
    types::numpy_gexpr<Arg, S...> const &v)
{
  PyObject *slices = ::to_python(v.slices);
  PyObject *base = ::to_python(v.arg);
  return PyObject_GetItem(base, slices);
}

namespace impl
{
  template <class T>
  struct is_integral_constant : std::false_type {
  };
  template <class T, T N>
  struct is_integral_constant<std::integral_constant<T, N>> : std::true_type {
  };

  template <class pS, class T, size_t... Is>
  bool check_shape(T const *dims, utils::index_sequence<Is...>)
  {
    types::array<bool, sizeof...(Is)> dims_match = {
        (is_integral_constant<typename std::tuple_element<Is, pS>::type>::value
             ? (dims[Is] ==
                std::conditional<
                    is_integral_constant<
                        typename std::tuple_element<Is, pS>::type>::value,
                    typename std::tuple_element<Is, pS>::type,
                    std::integral_constant<long, 0>>::type::value)
             : true)...};
    return std::find(dims_match.begin(), dims_match.end(), false) ==
           dims_match.end();
  }

  template <typename T, class pS>
  PyArrayObject *check_array_type_and_dims(PyObject *obj)
  {
    if (!PyArray_Check(obj))
      return nullptr;
    // the array must have the same dtype && the same number of dimensions
    PyArrayObject *arr = reinterpret_cast<PyArrayObject *>(obj);
    if (PyArray_TYPE(arr) != c_type_to_numpy_type<T>::value)
      return nullptr;
    if (PyArray_NDIM(arr) != std::tuple_size<pS>::value)
      return nullptr;
    return arr;
  }

  template <class T, class Slice, class S>
  void fill_slice(Slice &slice, long const *strides, long const *offsets,
                  S const *dims, utils::int_<0>)
  {
  }

  void set_slice(types::contiguous_normalized_slice &cs, long lower, long upper,
                 long step)
  {
    cs.lower = lower;
    cs.upper = upper;
    assert(cs.step == step && "consistent steps");
  }
  void set_slice(types::normalized_slice &s, long lower, long upper, long step)
  {
    s.lower = lower;
    s.upper = upper;
    s.step = step;
  }

  template <class T, class Slice, class S, size_t N>
  void fill_slice(Slice &slice, long const *strides, long const *offsets,
                  S const *dims, utils::int_<N>)
  {
    set_slice(std::get<std::tuple_size<Slice>::value - N>(slice),
              *offsets / sizeof(T),
              *offsets / sizeof(T) + *dims * *strides / sizeof(T),
              *strides / sizeof(T));
    fill_slice<T>(slice, strides + 1, offsets + 1, dims + 1,
                  utils::int_<N - 1>());
  }
}

template <typename T, class pS>
bool from_python<types::ndarray<T, pS>>::is_convertible(PyObject *obj)
{
  PyArrayObject *arr = impl::check_array_type_and_dims<T, pS>(obj);
  if (!arr)
    return false;
  auto const *stride = PyArray_STRIDES(arr);
  auto const *dims = PyArray_DIMS(arr);
  long current_stride = PyArray_ITEMSIZE(arr);
  for (long i = std::tuple_size<pS>::value - 1; i >= 0; i--) {
    if (stride[i] != current_stride)
      return false;
    current_stride *= dims[i];
  }
  // this is supposed to be a texpr
  if ((PyArray_FLAGS(arr) & NPY_ARRAY_F_CONTIGUOUS) &&
      ((PyArray_FLAGS(arr) & NPY_ARRAY_C_CONTIGUOUS) == 0) &&
      (std::tuple_size<pS>::value > 1)) {
    return false;
  }

  // check if dimension size match
  return impl::check_shape<pS>(
      dims, utils::make_index_sequence<std::tuple_size<pS>::value>());
}
template <typename T, class pS>
types::ndarray<T, pS> from_python<types::ndarray<T, pS>>::convert(PyObject *obj)
{
  PyArrayObject *arr = reinterpret_cast<PyArrayObject *>(obj);
  types::ndarray<T, pS> r((T *)PyArray_BYTES(arr), PyArray_DIMS(arr), obj);
  Py_INCREF(obj);
  return r;
}

template <typename T, class pS, class... S>
bool from_python<types::numpy_gexpr<types::ndarray<T, pS>,
                                    S...>>::is_convertible(PyObject *obj)
{
  PyArrayObject *arr = impl::check_array_type_and_dims<T, pS>(obj);
  if (!arr)
    return false;

  PyObject *base_obj = PyArray_BASE(arr);
  if (!base_obj || !PyArray_Check(base_obj))
    return false;
  PyArrayObject *base_arr = reinterpret_cast<PyArrayObject *>(base_obj);

  auto const *stride = PyArray_STRIDES(arr);
  auto const *dims = PyArray_DIMS(arr);

  /* FIXME If we have at least one stride, we convert the whole
   * array to a numpy_gexpr, without trying to be smarter with
   * contiguous slices
   */
  long current_stride = PyArray_ITEMSIZE(arr);
  bool at_least_one_stride = false;
  for (long i = std::tuple_size<pS>::value - 1; i >= 0; i--) {
    if (stride[i] < 0) {
      std::cerr << "array with negative strides are not supported" << std::endl;
      return false;
    } else if (stride[i] != current_stride) {
      at_least_one_stride = true;
      break;
    }
    current_stride *= dims[i];
  }
  if (at_least_one_stride) {
    if (PyArray_NDIM(base_arr) != std::tuple_size<pS>::value) {
      std::cerr << "reshaped array are not supported" << std::endl;
      return false;
    }
    return true;
  } else
    return false;
}

template <typename T, class pS, class... S>
types::numpy_gexpr<types::ndarray<T, pS>, S...>
from_python<types::numpy_gexpr<types::ndarray<T, pS>, S...>>::convert(
    PyObject *obj)
{
  PyArrayObject *arr = reinterpret_cast<PyArrayObject *>(obj);
  PyArrayObject *base_arr =
      reinterpret_cast<PyArrayObject *>(PyArray_BASE(arr));

  /* from the base array pointer && this array pointer, we can recover the
   * full slice informations
   * unfortunately, the PyArray representation is different from our.
   * - PyArray_BYTES gives the start of the base pointer
   * - PyArray_Dims give the dimension array (the shape)
   * - PyArray_STRIDES gives the stride information, but relative to the
   * base
   * pointer && ! relative to the lower dimension
   */
  long offsets[std::tuple_size<pS>::value];
  long strides[std::tuple_size<pS>::value];
  auto const *base_dims = PyArray_DIMS(base_arr);

  auto full_offset = PyArray_BYTES(arr) - PyArray_BYTES(base_arr);
  auto const *arr_strides = PyArray_STRIDES(arr);
  long accumulated_dim = 1;
  offsets[std::tuple_size<pS>::value - 1] =
      full_offset % base_dims[std::tuple_size<pS>::value - 1];
  strides[std::tuple_size<pS>::value - 1] =
      arr_strides[std::tuple_size<pS>::value - 1];
  for (ssize_t i = std::tuple_size<pS>::value - 2; i >= 0; --i) {
    accumulated_dim *= base_dims[i + 1];
    offsets[i] = full_offset / accumulated_dim;
    strides[i] = arr_strides[i] / accumulated_dim;
  }
  types::ndarray<T, pS> base_array((T *)PyArray_BYTES(base_arr),
                                   PyArray_DIMS(base_arr),
                                   (PyObject *)base_arr);
  std::tuple<S...> slices;
  impl::fill_slice<T>(slices, strides, offsets, PyArray_DIMS(arr),
                      utils::int_<sizeof...(S)>());
  types::numpy_gexpr<types::ndarray<T, pS>, S...> r(base_array, slices);

  Py_INCREF(obj);
  return r;
}

template <typename E>
bool from_python<types::numpy_texpr<E>>::

    is_convertible(PyObject *obj)
{
  constexpr auto N = E::value;
  PyArrayObject *arr =
      impl::check_array_type_and_dims<typename E::dtype, typename E::shape_t>(
          obj);
  if (!arr)
    return false;
  // check strides. Note that because it's a texpr, the check is done in the
  // opposite direction compared to ndarrays
  auto const *stride = PyArray_STRIDES(arr);
  auto const *dims = PyArray_DIMS(arr);
  long current_stride = PyArray_ITEMSIZE(arr);
  for (size_t i = 0; i < N; i++) {
    if (stride[i] != current_stride)
      return false;
    current_stride *= dims[i];
  }

  return PyArray_FLAGS(arr) & NPY_ARRAY_F_CONTIGUOUS && N > 1;
}

template <typename E>
types::numpy_texpr<E> from_python<types::numpy_texpr<E>>::convert(PyObject *obj)
{
  constexpr size_t N = E::value;
  using T = typename E::dtype;
  PyArrayObject *arr = reinterpret_cast<PyArrayObject *>(obj);
  typename E::shape_t shape;
  auto const *dims = PyArray_DIMS(arr);
  static_assert(N == 2, "only support texpr of matrices");
  sutils::assign(std::get<0>(shape), std::get<1>(dims));
  sutils::assign(std::get<1>(shape), std::get<0>(dims));
  types::ndarray<T, typename E::shape_t> base_array((T *)PyArray_BYTES(arr),
                                                    shape, obj);
  types::numpy_texpr<types::ndarray<T, typename E::shape_t>> r(base_array);
  Py_INCREF(obj);
  return r;
}
PYTHONIC_NS_END

#endif

#endif
