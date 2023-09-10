/*************************************************************************
 * Copyright © 2022 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of comcon.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include <span>
#include <tuple>
#include <vector>
#include <string_view>
#include <type_traits>
#include <memory_resource>

#include "comcon/utils.hpp"
#include "comcon/concepts.hpp"

namespace comcon {

template<typename pos_t, typename size_t>
struct basic_binary_descriptor { pos_t pos; size_t size; };
using binary_descriptor = basic_binary_descriptor<std::uint16_t, std::uint16_t>;

template<typename type>
constexpr std::size_t pack_sizeof =
        is_binary<std::decay_t<type>>.value
      ? sizeof(binary_descriptor)
      : sizeof(type)
        ;

template<pod_data type>
std::size_t binary_size(const type& obj) { return 0; }
template<any_binary_data type>
std::size_t binary_size(const type& obj) { return obj.size() * pack_sizeof<typename type::value_type>; }
template<typename... types, std::size_t... inds>
std::size_t binary_size(const std::tuple<types...>& src, std::index_sequence<inds...>)
{
	return (binary_size(std::get<inds>(src)) + ... );
}

template<any_binary_data type>
void check_binary_data_size(const type& data)
{
	if( std::numeric_limits<decltype(binary_descriptor::size)>::max() < data.size() )
		throw std::runtime_error("the binary data is too beig for been packed");
}

template<typename type, data_pack_typename data_type, typename... args_t>
auto pack_create(std::size_t shift, data_type& data, args_t... args)
{
	using obj_type = std::decay_t<type>;
	std::byte* place = &data[shift] - sizeof(obj_type);
	return new (place) obj_type{std::forward<args_t>(args)...};
}

template<pod_data type, data_pack_typename data_type>
auto pack(std::size_t shift, type&& obj, data_type& data)
{
	return pack_create<type>(shift, data, std::forward<type>(obj));
}

template<binary_data type, data_pack_typename data_type>
auto pack(std::size_t shift, const type& obj, data_type& data)
{
	using val_t = typename type::value_type;
	using dval_t = typename data_type::value_type;

	check_binary_data_size(obj);
	auto sz = data.size();
	for(auto s:obj) {
		if constexpr (sizeof(val_t) == sizeof(dval_t))
			data.emplace_back((typename data_type::value_type)s);
		else {
			auto sz = data.size();
			data.resize(sz + sizeof(val_t));
			new (&data[sz]) val_t{ s };
		}
	}
	return pack_create<binary_descriptor>(
	    shift, data,
	    static_cast<decltype(binary_descriptor::pos)>(sz),
	    static_cast<decltype(binary_descriptor::size)>(obj.size())
	);
}

template<recursive_binary_data type, data_pack_typename data_type>
auto pack(std::size_t shift, const type& obj, data_type& data)
{
	using val_t = std::decay_t<typename type::value_type>;

	check_binary_data_size(obj);
	auto sz = data.size();
	auto dpos = data.size();
	data.resize(data.size() + pack_sizeof<val_t>*obj.size());
	for(const auto& s:obj) pack(dpos += pack_sizeof<val_t>, s, data);
	return pack_create<binary_descriptor>(
	    shift, data,
	    static_cast<decltype(binary_descriptor::pos)>(sz),
	    static_cast<decltype(binary_descriptor::size)>(obj.size())
	);
}

template<typename... types, auto... inds, data_pack_typename data_type>
void pack(
        const std::tuple<types...>& src,
        data_type& data,
        std::index_sequence<inds...> seq)
{
	std::size_t shift = 0;
	( (pack(shift += pack_sizeof<decltype(std::get<inds>(src))>, std::get<inds>(src), data)), ... );
}

template<
        typename factory_t=data_type_factory,
        typename... types,
        data_pack_typename data_type = decltype(std::declval<factory_t>()())
        >
data_type pack(const std::tuple<types...>& src, factory_t&& factory=factory_t{})
{
	std::size_t pod_size = (pack_sizeof<types> + ...) ;
	std::size_t bin_size = binary_size(src, std::index_sequence_for<types...>{});
	data_type ret = factory();
	ret.reserve(pod_size + bin_size);
	ret.resize(pod_size);
	pack( src, ret, std::index_sequence_for<types...>{} );
	return ret;
}

template<typename type, data_readable_typename data_type>
type init_from(const binary_descriptor* desc, const data_type& data);

template<typename type, data_readable_typename data_type>
auto extract(std::size_t pos, const data_type& data)
{
	if constexpr (is_binary<type>) {
		const binary_descriptor* desc = extract<binary_descriptor>(pos, data);
		return init_from<type>(desc, data);
	} else {
		assert( (pos + sizeof(type)) <= data.size() );
		return reinterpret_cast<const type*>(&data[pos]);
	}
}

template<std::size_t ind=0, std::size_t pos=0, typename... types, data_readable_typename data_type>
auto& extract(
        const data_type& data,
        std::tuple<types...>& dest
        )
{
	using tuple_type = std::tuple<types...>;
	if constexpr (ind < sizeof...(types)) {
		using tuple_element = std::decay_t<std::tuple_element_t<ind, tuple_type>>;
		if constexpr (recursive_binary_data<tuple_element>) {
			std::get<ind>(dest) = extract<tuple_element>(pos, data);
		}
		else if constexpr (is_string<tuple_element> || is_vector<tuple_element>) {
			auto view = extract<view_type_t<tuple_element>>(pos, data);
			std::get<ind>(dest).assign(view.begin(), view.end());
		} else if constexpr (is_binary<tuple_element>){
			std::get<ind>(dest) = extract<tuple_element>(pos, data);
		}
		else {
			std::get<ind>(dest) = *extract<tuple_element>(pos, data);
		}
		extract<ind+1, pos + pack_sizeof<tuple_element>>(data, dest);
	}
	return dest;
}

template<typename tuple_t, data_readable_typename data_type>
tuple_t extract(const data_type& data)
{
	tuple_t ret;
	extract(data, ret);
	return ret;
}

template<typename type, data_readable_typename data_type>
type init_from(const binary_descriptor* desc, const data_type& data)
{
	if constexpr (recursive_binary_data<type>) {
		static_assert( is_vector<type>, "recursive binary type must be a vector instead of span");
		std::decay_t<type> ret;
		ret.reserve(desc->size);
		for(std::size_t pos = 0;pos<desc->size;++pos)
			ret.emplace_back(extract<std::decay_t<typename type::value_type>>(
			                     desc->pos + pos*pack_sizeof<type>, data));
		return ret;
	} else if constexpr (is_vector<type>) {
		std::decay_t<type> ret;
		ret.reserve(desc->size);
		for(std::size_t pos = 0;pos<desc->size;++pos)
			ret.emplace_back(data[pos + desc->pos]);
		return ret;
	}
	else {
		if constexpr (is_span<type>)
		    static_assert( is_const_type<typename type::element_type>,
		        "it must to be std::span<const T> or string_view insteed of mutable byte");
		return type{ reinterpret_cast<const type::value_type*>(&data[desc->pos]), desc->size };
	}
}

} // namespace comcon
