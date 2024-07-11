module;

#include <type_traits>

export module voxel_game.utilities.tmp;

export import :non_type_parameters;

export namespace vxg::utilities::tmp {

	template <typename T>
	struct ExtractNestedTemplate;
	template <template <typename> typename T, typename Nested>
	struct ExtractNestedTemplate<T<Nested>> {
		using Type = Nested;
	};
	template <typename T>
	using ExtractNestedTemplate_t = ExtractNestedTemplate<T>::Type;

	template <template <typename> typename T, typename U, typename = void>
	struct IsFullySpecialized : std::false_type {};
	template <template <typename> typename T, typename U>
	struct IsFullySpecialized<T, U, std::void_t<decltype(sizeof(T<U>))>> : std::true_type {};
	template <template <typename> typename T, typename U>
	inline constexpr bool IsFullySpecialized_v = IsFullySpecialized<T, U>::value;

} // namespace vxg::utilities::tmp