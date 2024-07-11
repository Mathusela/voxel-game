module;

#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <cstdint>
#include <functional>
#include <new>
#include <cstddef>

export module voxel_game.reflection:class_info;

import voxel_game.utilities;

using namespace vxg::utilities::tmp;

// EXPORTED
export namespace vxg::reflection {

	template <typename T>
	struct ClassInfoTraits;

	template <typename T>
	concept registered_class = IsFullySpecialized_v<ClassInfoTraits, T>;

	template <registered_class T>
	class ClassInfo;

}; // namespace vxg::reflection

namespace vxg::reflection {
	template <typename T, bool = registered_class<T>>
	struct TypeInfoHelper {
		using Type = void;
	};
	template <typename T>
	struct TypeInfoHelper<T, true> {
		using Type = ClassInfo<T>;
	};
	template <typename T>
	using TypeInfoHelper_t = TypeInfoHelper<T>::Type;
}; // namespace vxg::reflection

// EXPORTED
export namespace vxg::reflection {

	template <typename T, typename Class, StaticStringLiteral Name, size_t Offset>
	struct MemberInfo {
		using Type = T;
		using ParentClass = Class;
		static constexpr std::string_view name = Name.value;
		static constexpr size_t offset = Offset;

		using TypeInfo = TypeInfoHelper_t<T>;

		[[nodiscard]]
		static consteval std::uint64_t identifier_hash() noexcept {
			std::string mangledName = std::string(ClassInfo<Class>::name);
			mangledName += name;

			return vxg::utilities::hash_string(mangledName);
		}
	};

	template <typename, typename, typename, typename>
	struct GenerateMembers;
	template <typename Class, typename... Types, StaticStringLiteral... Strings, size_t... Offsets>
	struct GenerateMembers<Class, TypePack<Types...>, StringPack<Strings...>, ValuePack<size_t, Offsets...>> {
		using Type = std::tuple<MemberInfo<Types, Class, Strings, Offsets>...>;
	};
	template <typename C, typename T, typename S, typename O>
	using GenerateMembers_t = GenerateMembers<C, T, S, O>::Type;

	template <registered_class T>
	class ClassInfo {
		template <typename F, size_t... I>
		static void for_each_member_impl(F&& f, std::index_sequence<I...>) {
			(std::invoke(f, Member<I>{}), ...);
		}
		template <typename F, typename U, size_t... I>
		static void for_each_member_impl(F&& f, U&& x, std::index_sequence<I...>) {
			(std::invoke(f, Member<I>{}, 
				*std::launder(reinterpret_cast<const typename Member<I>::Type*>(reinterpret_cast<const std::byte*>(&x) + Member<I>::offset))
			), ...);
		}

	public:
		using Members = ClassInfoTraits<T>::Members;
		static constexpr size_t memberCount = std::tuple_size_v<Members>;

		template <typename F>
		static void for_each_member(F&& f) {
			for_each_member_impl(std::forward<F>(f), std::make_index_sequence<memberCount>{});
		}
		template <typename F, typename U>
		static void for_each_member(F&& f, U&& x) {
			for_each_member_impl(std::forward<F>(f), std::forward<U>(x), std::make_index_sequence<memberCount>{});
		}

		template <int Index>
		using Member = std::tuple_element_t<Index, Members>;

		static constexpr std::string_view name = ClassInfoTraits<T>::name;

		[[nodiscard]]
		static consteval std::uint64_t hash() noexcept {
			return vxg::utilities::hash_string(name);
		}
	};

}; // namespace vxg::reflection