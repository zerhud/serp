

    /*************************************************************************
     * Copyright © 2022 Hudyaev Alexy <hudyaev.alexy@gmail.com>
     * This file is part of comcon.
     * Distributed under the GNU Affero General Public License.
     * See accompanying file copying (at the root of this repository)
     * or <http://www.gnu.org/licenses/> for details
     *************************************************************************/

    #pragma once

    #include <iostream>

    namespace std {

    template<typename type_left, size_t ex_left, typename type_right, size_t ex_right>
    bool operator == (
            const std::span<type_left, ex_left>& left,
            const std::span<type_right, ex_right>& right)
    {
    	if(left.size() != right.size()) return false;
    	for(std::size_t i=0;i<left.size();++i)
    		if(left[i] != right[i])
    			return false;
    	return true;
    }

    } // namespace std

    namespace tests {

    template<typename data_t>
    const data_t& print_data(const data_t& data)
    {
    	using namespace std::literals;
    	std::cout << "==== "sv << data.size() << std::endl;
    	std::cout.setf(std::ios::hex, std::ios::basefield);
    	for(auto& s:data) std::cout << (int)s << ' ';
    	std::cout.unsetf(std::ios::hex);
    	std::cout << std::endl << "===="sv << std::endl;
    	return data;
    }

    } // namespace tests

