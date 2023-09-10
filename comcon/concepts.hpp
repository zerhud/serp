/*************************************************************************
 * Copyright © 2022 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of comcon.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include <span>
#include <vector>
#include <concepts>

namespace comcon {

template<typename type>
constexpr std::false_type is_vector {};
template<typename... types>
constexpr std::true_type is_vector<std::vector<types...>> {};

template<typename type>
constexpr std::false_type is_string {};
template<typename... types>
constexpr std::true_type is_string<std::basic_string<types...>> {};

template<typename type>
constexpr std::false_type is_span {};
template<typename type, auto sz>
constexpr std::true_type is_span<std::span<type, sz>> {};

template<typename t>
struct view_type{ using type = t; };
template<typename... types>
struct view_type<std::basic_string<types...>>
{
	using type = std::basic_string_view<typename std::basic_string<types...>::value_type>;
};
template<typename... types>
struct view_type<std::vector<types...>>
{
	using type = std::span<const typename std::vector<types...>::value_type>;
};
template<typename t>
using view_type_t = view_type<t>::type;

template<typename type>
constexpr std::false_type is_binary {};
template<typename... types>
constexpr std::true_type is_binary<std::basic_string<types...>> {};
template<typename... types>
constexpr std::true_type is_binary<std::basic_string_view<types...>> {};
template<typename type, std::size_t ex>
constexpr std::true_type is_binary<std::span<type, ex>> {};
template<typename... types>
constexpr std::true_type is_binary<std::vector<types...>> {};

template<typename type>
concept binary_data =
        is_binary<type>.value && !is_binary<std::decay_t<typename type::value_type>>.value;
template<typename type>
concept recursive_binary_data =
        is_binary<type>.value && is_binary<std::decay_t<typename type::value_type>>.value;
template<typename type>
concept any_binary_data = binary_data<type> || recursive_binary_data<type>;
template<typename type>
concept pod_data = !is_binary<type>.value;
template<typename type>
concept data_readable_typename =
        sizeof(typename type::value_type) == 1
     && requires(type& d)
        {
            {d.size()} -> std::same_as<std::size_t>;
            d.begin();
            d.end();
        }
        ;
template<typename type>
concept data_pack_typename =
        data_readable_typename<type>
     && requires(type& d)
        {
            d.resize(100);
            {d.data()} -> std::same_as<typename type::value_type*>;
        }
        ;

template<typename type>
concept is_const_type =
        std::is_const_v<type>
     || std::is_same_v<std::string_view, std::decay_t<type>>
     ;


} // namespace comcon