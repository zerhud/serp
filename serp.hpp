#pragma once

#include <utility>

namespace serp {

template<typename factory, typename type>
constexpr void pack(factory&& io, const type& what) ;

template<typename factory, typename type, typename size_type=decltype(sizeof(int))>
constexpr void read(factory&& io, type& to, size_type shift=0) ;

namespace details {

struct cpp17_adl_fail {};
template<auto ind, typename type> cpp17_adl_fail get(const type&);

template<template<typename...>class tuple, typename... types>
constexpr auto size(const tuple<types...>*) { return sizeof...(types); }

template<typename type>
class cpp17_concepts {
	constexpr static bool _is_adl_get(const void*) { return false; }
	template<typename t> constexpr static auto _is_adl_get(const t* p) -> decltype(get<0>(*p), void(), true) {
		return !std::is_same_v<decltype(get<0>(*p)), cpp17_adl_fail>;
	}

	constexpr static bool _is_size_member(const void*) { return false; }
	template<typename t> constexpr static auto _is_size_member(const t* p) -> decltype(p->size(), void(), true) {
		return true;
	}
public:
	constexpr static bool is_adl_get = _is_adl_get(static_cast<const type*>(nullptr));
	constexpr static bool is_size_member = _is_size_member(static_cast<type*>(nullptr));
};

template<auto... inds, typename factory_t, typename type>
constexpr auto call_pack_cpp17(factory_t&& io, const type& src) {
	if constexpr(sizeof...(inds)<size(static_cast<const type*>(nullptr)))
		call_pack_cpp17<inds..., sizeof...(inds)>( std::forward<decltype(io)>(io), src );
	else (pack(io, get<inds>(src)),...);
}

/*
template<auto ind, auto shift, typename factory_t, typename type>
constexpr void foreach_get_read(const factory_t& io, type& to) {
	if constexpr(ind < size(static_cast<const type*>(nullptr))) {
		read(io, get<ind>(to), shift);
		foreach_get_read<ind+1,shift+sizeof(decltype(get<ind>(to)))>(io, to);
	}
}
*/

template<auto... inds, typename factory_t, typename type, typename size_type>
constexpr auto foreach_get_read(factory_t&& io, type& to, size_type shift, std::index_sequence<inds...>) {
	shift -= sizeof(get<0>(to));
	(read(io, get<inds>(to), shift += sizeof(get<inds>(to))), ...);
}

} // namespace details

template<typename factory, typename type>
constexpr void pack(factory&& io, const type& what) {
	using concepts = details::cpp17_concepts<type>;
	if constexpr(concepts::is_adl_get)
		details::call_pack_cpp17(io, what);
	else if constexpr(concepts::is_size_member)
		for(auto& item:what) pack(io, item);
	else io.write(&what, 1);
}


template<typename factory, typename type, typename size_type>
constexpr void read(factory&& io, type& to, size_type shift) {
	using concepts = details::cpp17_concepts<type>;
	io.read(shift, to);
	if constexpr(concepts::is_adl_get)
		details::foreach_get_read(io, to, shift, std::make_index_sequence<details::size(static_cast<const type*>(nullptr))>());
	else if constexpr(concepts::is_size_member)
		;
	else io.read(shift, to);
}

} // namespace serp
