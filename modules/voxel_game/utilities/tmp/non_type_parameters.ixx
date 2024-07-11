module;

#include <algorithm>

export module voxel_game.utilities.tmp:non_type_parameters;

export namespace vxg::utilities::tmp {

	template <size_t N>
	struct StaticStringLiteral {
		char value[N];

		consteval StaticStringLiteral(const char(&str)[N]) {
			std::copy_n(str, N, value);
		}
	};

	template <typename... T>
	struct TypePack {};
	template <typename... T>
	consteval TypePack<T...> infer_type_pack(T...) {
		return TypePack<T...>{};
	}

	template <typename T, T... V>
	struct ValuePack {};

	template <StaticStringLiteral... Str>
	struct StringPack {};

} // namespace vxg::utilities::tmp