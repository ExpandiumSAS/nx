#ifndef __NX_TUPLE_UTILS_H__
#define __NX_TUPLE_UTILS_H__

#include <stdint.h>

#include <type_traits>
#include <tuple>

namespace nx {

template <std::size_t ...>
struct tuple_indices
{};

template <std::size_t N, std::size_t ...I>
struct make_tuple_indices
: make_tuple_indices<N - 1, N - 1, I...>
{};

template<std::size_t ...I>
struct make_tuple_indices<0, I...> {
    typedef tuple_indices<I...> type;
};

namespace detail {

template <std::size_t I, class Tuple, typename F, typename ...Args>
struct for_each_impl {
    static void for_each(Tuple&& t, F&& f, Args&&... args)
    {
        for_each_impl<I - 1, Tuple, F, Args...>::for_each(
            std::forward<Tuple>(t),
            std::forward<F>(f),
            std::forward<Args>(args)...
        );
        f(I, std::get<I>(t), std::forward<Args>(args)...);
    }
};

template <class Tuple, typename F, typename... Args>
struct for_each_impl<0, Tuple, F, Args...> {
    static void for_each(Tuple&& t, F&& f, Args&&... args)
    {
        f(0, std::get<0>(t), std::forward<Args>(args)...);
    }
};

template <std::size_t Index, typename Search, typename First, typename... Types>
struct get_impl
{
    typedef typename get_impl<Index + 1, Search, Types...>::type type;
    static constexpr std::size_t index = Index;
};

template <std::size_t Index, typename Search, typename... Types>
struct get_impl<Index, Search, Search, Types...>
{
    typedef get_impl type;
    static constexpr std::size_t index = Index;
};

} // namespace detail

template<class Tuple, typename F, typename... Args>
void for_each(Tuple&& t, F&& f, Args&&... args)
{
    detail::for_each_impl<
        std::tuple_size<
            typename std::decay<Tuple>::type
        >::value - 1,
        Tuple, F, Args...
    >::for_each(
        std::forward<Tuple>(t),
        std::forward<F>(f),
        std::forward<Args>(args)...
    );
}

template <typename T, typename... Types>
struct tuple_element;

template <typename T, typename... Types, typename... Keys>
struct tuple_element<T, std::tuple<Types...>, std::tuple<Keys...>>
{
    typedef typename std::tuple_element<
        detail::get_impl<0, T, Keys...>::type::index,
        std::tuple<Types...>
    >::type type;
    static constexpr std::size_t index =
        detail::get_impl<0, T, Keys...>::type::index;
};

// Tuple sequence utilities
template <typename T, typename... Ts>
struct is_member_of_type_seq
{ static const bool value = false; };

template <typename T, typename U, typename... Ts>
struct is_member_of_type_seq<T, U, Ts...>
{
    static const bool value = std::conditional<
        std::is_same<T, U>::value,
        std::true_type,
        is_member_of_type_seq<T, Ts...>
    >::type::value;
};

template <typename, typename>
struct append_to_type_seq { };

template <typename T, typename... Ts>
struct append_to_type_seq<T, std::tuple<Ts...>>
{
    using type = std::tuple<Ts..., T>;
};

template<typename, typename>
struct prepend_to_type_seq { };

template<typename T, typename... Ts>
struct prepend_to_type_seq<T, std::tuple<Ts...>>
{
    using type = std::tuple<T, Ts...>;
};

// Tuple sequence meta functions
template <typename, typename>
struct intersect_type_seq
{
    using type = std::tuple<>;
};

template <typename T, typename... Ts, typename... Us>
struct intersect_type_seq<std::tuple<T, Ts...>, std::tuple<Us...>>
{
    using type = typename std::conditional<
        !is_member_of_type_seq<T, Us...>::value,
        typename intersect_type_seq<
            std::tuple<Ts...>,
            std::tuple<Us...>
        >::type,
        typename prepend_to_type_seq<
            T,
            typename intersect_type_seq<
                std::tuple<Ts...>,
                std::tuple<Us...>
            >::type
        >::type
    >::type;
};

template <typename... Ts>
struct all_of : std::integral_constant<bool, false>
{};

template <typename T>
struct all_of<T> : std::integral_constant<bool, T::value>
{};

template<typename T, typename... Ts>
struct all_of<T, Ts...>
: std::integral_constant<
    bool,
    T::value && all_of<Ts...>::value
>
{};

template <typename A, typename... Deriveds>
struct all_base_of
{
    static constexpr bool value =
        all_of<std::is_base_of<A, Deriveds>...>::value;
};

} // namespace nx

#endif // __NX_TUPLE_UTILS_H__
