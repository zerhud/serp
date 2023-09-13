#include "serp.hpp"

//#define SNITCH_IMPLEMENTATION
//#include <snitch_all.hpp>
#include <snitch/snitch.hpp>

#include <bit>
#include <new>
#include <vector>
#include <string>

struct io {
	using byte_type = std::byte;
	std::vector<byte_type> store;

	template<typename in_type, typename size_type>
	constexpr void write(const in_type* data, size_type size) {
		for(auto i=0;i<size;++i) {
			auto arr = std::bit_cast<std::array<byte_type,sizeof(*data)>>(data[i]);
			for(auto& b:arr) store.emplace_back(b);
		}
	}
	template<typename size_type, typename type>
	constexpr void read(size_type start_pos, type& to) const {
		std::array<byte_type,sizeof(type)> cur_d{};
		for(unsigned i=0;i<sizeof(type);++i)
			cur_d[i] = store.at(i+start_pos);
		to = std::bit_cast<type>(cur_d);
	}
};

static_assert( !serp::details::cpp17_concepts<int>::is_adl_get );
static_assert( serp::details::cpp17_concepts<std::tuple<int,char>>::is_adl_get );

constexpr auto test_pack(auto&& data) {
	io io;
	serp::pack(io, data);
	return io;
}
constexpr auto test_make_tuple_d() {
	return std::make_tuple((std::uint8_t)10, (std::int8_t)11, (std::uint64_t)12, (double)1.3);
}
constexpr auto test_make_tuple_a() {
	return std::make_tuple((std::uint8_t)0, (std::int8_t)0, (std::uint64_t)0, (double)0);
}

TEST_CASE("write simple") {
	auto test = []{
		io io;
		int o_pod = 0;
		int i_pod = 10, i2_pod = 20;
		serp::pack(io, i_pod);
		serp::pack(io, i2_pod);
		serp::read(io, o_pod, sizeof(i_pod));
		return o_pod;
	};
	CHECK(test() == 20);
	static_assert( test() == 20 );
}

static_assert( []{
	auto io = test_pack( test_make_tuple_d() );
	auto ans = std::make_tuple((std::uint8_t)0, (std::int8_t)0);
	serp::read(io, ans);
	return get<0>(ans) + get<1>(ans);}() == 21);
TEST_CASE("write tuple") {
	auto io = test_pack( test_make_tuple_d() );

	auto ans = std::make_tuple((std::uint8_t)0, (std::int8_t)0);
	serp::read(io, ans);

	CHECK(get<0>(ans)==10);
	CHECK(get<1>(ans)==11);
}

TEST_CASE("read tuple") {
	auto src = test_make_tuple_d();
	auto io = test_pack( src );
	auto answ = test_make_tuple_a();
       	serp::read(io, answ);
	CHECK(get<0>(answ) == get<0>(src));
	CHECK(get<1>(answ) == get<1>(src));
	CHECK(get<2>(answ) == get<2>(src));
	CHECK(get<3>(answ) == get<3>(src));
}

TEST_CASE("rw string") {
	using namespace std::literals;
	auto src = "foo"s;
	io io;
	serp::pack(io, src);
	std::string answ;
	serp::read(io, answ);
	CHECK(answ == src);
}
