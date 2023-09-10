

    /*************************************************************************
     * Copyright © 2022 Hudyaev Alexy <hudyaev.alexy@gmail.com>
     * This file is part of comcon.
     * Distributed under the GNU Affero General Public License.
     * See accompanying file copying (at the root of this repository)
     * or <http://www.gnu.org/licenses/> for details
     *************************************************************************/

    #define BOOST_TEST_DYN_LINK
    #define BOOST_TEST_MODULE serialization

    #include <bit>
    #include <tuple>
    #include <boost/test/included/unit_test.hpp>
    #include <comcon.hpp>

    #include "mocks.hpp"

    using namespace std::literals;

    BOOST_AUTO_TEST_SUITE(comon)
    BOOST_AUTO_TEST_SUITE(serialization)
    BOOST_AUTO_TEST_CASE(pods)
    {
    	auto src = std::make_tuple((std::uint8_t)10, (std::int8_t)11, (std::uint64_t)12, (double)1.3);
    	auto data = comcon::pack(src);

    	const auto* src_0 = comcon::extract<std::uint8_t>(0, data);
    	BOOST_TEST(*src_0 == 10 );

    	const auto* src_1 = comcon::extract<std::int8_t>(sizeof(std::uint8_t), data);
    	BOOST_TEST(*src_1 == 11 );

    	using tuple_t = std::tuple<std::uint8_t, std::int8_t, std::uint64_t, double>;
    	tuple_t dest =  comcon::extract<tuple_t>(data);
    	BOOST_CHECK( dest == src );

    	using half_tuple_t = std::tuple<std::uint8_t, std::int8_t>;
    	const half_tuple_t half = comcon::extract<half_tuple_t>(data);
    	BOOST_TEST(std::get<0>(half) == 10);
    	BOOST_TEST(std::get<1>(half) == 11);

    	std::uint16_t swapper = 0x1234;
    	std::uint16_t swapped = std::byteswap(swapper);
    	BOOST_TEST(swapped == 0x3412);
    }
    BOOST_AUTO_TEST_CASE(long_strings)
    {
    	std::string test_str { "long test string long test string"sv };
    	auto src = std::make_tuple((std::uint16_t)0x1234, test_str);
    	auto data = comcon::pack(src);

    	const auto* src_0 = comcon::extract<std::uint16_t>(0, data);
    	BOOST_TEST(*src_0 == 0x1234 );

    	auto src_1 = comcon::extract<std::string_view>(sizeof(std::uint16_t), data);
    	BOOST_TEST(src_1 == test_str);
    }
    BOOST_AUTO_TEST_CASE(short_string_optimization)
    {
    	std::string test_str { "tst"sv };
    	auto src = std::make_tuple(test_str);
    	auto data = comcon::pack(src);
    	auto src_0 = comcon::extract<std::string_view>(0, data);
    	BOOST_TEST(src_0 == test_str);
    }
    BOOST_AUTO_TEST_CASE(string_view)
    {
    	std::string_view test_str = "test string"sv;
    	auto src = std::make_tuple(test_str);
    	auto data = comcon::pack(src);
    	auto src_0 = comcon::extract<std::string_view>(0, data);
    	BOOST_TEST(src_0 == test_str);
    }
    BOOST_AUTO_TEST_CASE(binary_vector_span)
    {
    	std::pmr::vector<std::byte> test_data1, test_data2;
    	test_data1.emplace_back((std::byte)0x12);
    	test_data1.emplace_back((std::byte)0x34);
    	test_data2.emplace_back((std::byte)0x56);
    	test_data2.emplace_back((std::byte)0x78);
    	std::span<const std::byte> span_data(test_data1.data(), test_data1.size());
    	auto src = std::make_tuple(span_data, test_data2);
    	auto data = comcon::pack(src);
    	auto src_0 = comcon::extract<std::span<const std::byte>>(0, data);
    	auto src_1 = comcon::extract<std::vector<std::byte>>(sizeof(comcon::binary_descriptor), data);
    	BOOST_CHECK(src_0 == span_data);
    	BOOST_TEST(src_1.size() == 2);
    	BOOST_TEST((int)src_1.at(0) == 0x56);
    	BOOST_TEST((int)src_1.at(1) == 0x78);
    	BOOST_CHECK(comcon::extract<decltype(src)>(data) == src);
    }
    BOOST_AUTO_TEST_CASE(vector_of_data)
    {
    	std::pmr::vector<std::int64_t> ints{ 1,2,3 };
    	std::pmr::vector<std::pmr::string> strings{ "one", "two", "three" };
    	auto src = std::make_tuple(ints, strings);
    	auto data = comcon::pack(src);

    	auto src_0 = comcon::extract<std::span<const std::int64_t>>(0,  data );
    	BOOST_TEST(src_0.size() == 3);
    	BOOST_TEST(src_0[0] == 1);
    	BOOST_TEST(src_0[1] == 2);
    	BOOST_TEST(src_0[2] == 3);

    	constexpr std::size_t shift = sizeof(comcon::binary_descriptor);
    	auto src_1 = comcon::extract<std::vector<std::string_view>>(shift,  data );
    	BOOST_TEST(src_1.size() == 3);
    	BOOST_TEST(src_1[0] == "one");
    	BOOST_TEST(src_1[1] == "two");
    	BOOST_TEST(src_1[2] == "three");
    }
    BOOST_AUTO_TEST_CASE(deep_data_recursion)
    {
    	std::pmr::vector<std::int64_t> ints{ 1,2,3 };
    	std::pmr::vector<std::pmr::vector<std::pmr::string>> strings{
    		{"one", "two", "three"},
    		{"four", "five", "six"},
    		{"seven", "eight", "nine"}
    	};

    	auto src = std::make_tuple(strings, ints);
    	auto data = comcon::pack(src);

    	auto extracted = comcon::extract<decltype(src)>(data);

    	auto ex_ints = std::get<1>(extracted);
    	BOOST_TEST(ex_ints.size() == 3);
    	BOOST_TEST(ex_ints[0] == 1);
    	BOOST_TEST(ex_ints[1] == 2);
    	BOOST_TEST(ex_ints[2] == 3);

    	auto ex_strings = std::get<0>(extracted);
    	BOOST_TEST(ex_strings.size() == 3);
    	BOOST_TEST(ex_strings[0][0] == "one");
    	BOOST_TEST(ex_strings[0][1] == "two");
    	BOOST_TEST(ex_strings[0][2] == "three");
    	BOOST_TEST(ex_strings[1][0] == "four");
    	BOOST_TEST(ex_strings[1][1] == "five");
    	BOOST_TEST(ex_strings[1][2] == "six");
    	BOOST_TEST(ex_strings[2][0] == "seven");
    	BOOST_TEST(ex_strings[2][1] == "eight");
    	BOOST_TEST(ex_strings[2][2] == "nine");
    }
    BOOST_AUTO_TEST_CASE(wide_string)
    {
    	std::wstring ugly_string = L"test";
    	auto src = std::make_tuple(ugly_string);
    	auto data = comcon::pack(src);
    	auto src_0 = comcon::extract<std::wstring_view>(0, data);
    	BOOST_CHECK(src_0 == ugly_string);
    	BOOST_CHECK(comcon::extract<decltype(src)>(data) == src);
    }
    BOOST_AUTO_TEST_CASE(allocations)
    {
    	struct pmr_remover {
    		std::pmr::memory_resource* saved;
    		pmr_remover() : saved(std::pmr::get_default_resource())
    		{ std::pmr::set_default_resource( std::pmr::null_memory_resource() ); }
    		~pmr_remover() { std::pmr::set_default_resource(saved); }
    	};
    	std::pmr::vector<std::byte> vecdata;
    	vecdata.assign({(std::byte)0x12, (std::byte)0x34, (std::byte)0x56, (std::byte)0x78});
    	auto orig = std::make_tuple( std::pmr::string("test string"), vecdata);
    	decltype(orig) src = orig;
    	pmr_remover remover;
    	auto data = comcon::pack(src, comcon::data_type_factory{remover.saved});
    	BOOST_CHECK(comcon::extract(data, src) == orig);

    	std::tuple<std::string_view, std::span<const std::byte>> view_tuple;
    	comcon::extract(data, view_tuple);
    	BOOST_TEST(std::get<0>(view_tuple) == std::get<0>(orig));
    	auto vecview = std::get<1>(view_tuple);
    	BOOST_TEST(vecview.size() == vecdata.size());
    	for(std::size_t i=0;i<vecview.size();++i) BOOST_TEST((int)vecview[i] == (int)vecdata.at(i));
    }
    BOOST_AUTO_TEST_CASE(check_binary_szie)
    {
    	std::vector<std::byte> bad_data;
    	bad_data.resize( 66'000 );
    	auto src = std::make_tuple(bad_data);
    	BOOST_CHECK_THROW(comcon::pack(src), std::runtime_error);
    }
    BOOST_AUTO_TEST_CASE(can_extract_from_span)
    {
    	using sub_t = std::tuple<int,std::string_view>;
    	using from_t = std::tuple<int, int, std::string_view>;

    	auto src = std::make_tuple(10, 20, "hello"sv);
    	auto data = comcon::pack(src);
    	auto span = std::span(data.data(), data.size());
    	auto got = comcon::extract<from_t>(span);
    	BOOST_TEST(std::get<0>(got) == 10);
    	BOOST_TEST(std::get<1>(got) == 20);
    	BOOST_TEST(std::get<2>(got) == "hello"sv);

    	constexpr std::size_t e1_size = sizeof(std::tuple_element_t<0, from_t>);
    	sub_t got2;
    	comcon::extract<0, e1_size>(span, got2);
    	BOOST_TEST(std::get<0>(got2) == 20);
    	BOOST_TEST(std::get<1>(got2) == "hello"sv);
    }
    BOOST_AUTO_TEST_SUITE_END() // serialization
    BOOST_AUTO_TEST_SUITE_END() // comon

