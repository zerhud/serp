/*************************************************************************
 * Copyright © 2022 Hudyaev Alexy <hudyaev.alexy@gmail.com>
 * This file is part of comcon.
 * Distributed under the GNU Affero General Public License.
 * See accompanying file copying (at the root of this repository)
 * or <http://www.gnu.org/licenses/> for details
 *************************************************************************/

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

namespace comcon {

struct data_type_factory {
	mutable std::pmr::memory_resource* mem = std::pmr::get_default_resource();
	auto operator()() const { return std::pmr::vector<std::byte>{mem}; }
};

struct container_factory {
	mutable std::pmr::memory_resource* mem = std::pmr::get_default_resource();

	template<typename key, typename value>
	auto map() const { return std::pmr::unordered_map<key,value>( mem ); }

	template<typename type>
	auto vector() const { return std::pmr::vector<type>( mem ); }
};

} // namespace comcon

