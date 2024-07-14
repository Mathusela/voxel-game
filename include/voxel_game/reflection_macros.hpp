#ifndef VXG_REFLECTION_MACROS_HPP
#define VXG_REFLECTION_MACROS_HPP

#include <string_view>
#include <cstddef>

#define DETAIL_VXG_SCAN0(...) __VA_ARGS__	// Expand 2 arguments
#define DETAIL_VXG_SCAN1(...) DETAIL_VXG_SCAN0(DETAIL_VXG_SCAN0(__VA_ARGS__))	// Expand 4 arguments
#define DETAIL_VXG_SCAN2(...) DETAIL_VXG_SCAN1(DETAIL_VXG_SCAN1(__VA_ARGS__))	// Expand 8 arguments
#define DETAIL_VXG_SCAN3(...) DETAIL_VXG_SCAN2(DETAIL_VXG_SCAN2(__VA_ARGS__))	// Expand 16 arguments
#define DETAIL_VXG_SCAN4(...) DETAIL_VXG_SCAN3(DETAIL_VXG_SCAN3(__VA_ARGS__))	// Expand 32 arguments
#define DETAIL_VXG_SCAN(...) DETAIL_VXG_SCAN4(__VA_ARGS__)

#define DETAIL_VXG_EMPTY

#define DETAIL_VXG_MAP0(M, x, ...) M(x) __VA_OPT__( DETAIL_VXG_MAP1 DETAIL_VXG_EMPTY (M, __VA_ARGS__) )
#define DETAIL_VXG_MAP1(M, x, ...) M(x) __VA_OPT__( DETAIL_VXG_MAP0 DETAIL_VXG_EMPTY (M, __VA_ARGS__) )
#define DETAIL_VXG_MAP(M, ...) __VA_OPT__( DETAIL_VXG_SCAN(DETAIL_VXG_MAP0(M, __VA_ARGS__)) )

#define DETAIL_VXG_STRINGIFY(x) #x
#define DETAIL_VXG_STRINGIFY_ARGUMENT(x) , DETAIL_VXG_STRINGIFY(x)

#define DETAIL_VXG_GET_OFFSET(x) offsetof(ClassType, x)
#define DETAIL_VXG_GET_OFFSET_ARGUMENT(x) , DETAIL_VXG_GET_OFFSET(x)

#define DETAIL_VXG_PREFIX_WITH_CLASS(x) ClassType::x
#define DETAIL_VXG_PREFIX_WITH_CLASS_ARGUMENT(x) , DETAIL_VXG_PREFIX_WITH_CLASS(x)

#define DETAIL_VXG_PROCESS_TYPES_HELPER(x, ...) DETAIL_VXG_PREFIX_WITH_CLASS(x) DETAIL_VXG_MAP(DETAIL_VXG_PREFIX_WITH_CLASS_ARGUMENT, __VA_ARGS__)
#define DETAIL_VXG_PROCESS_TYPES(...) \
	decltype(vxg::utilities::tmp::infer_type_pack(__VA_OPT__(DETAIL_VXG_PROCESS_TYPES_HELPER(__VA_ARGS__))))

#define DETAIL_VXG_PROCESS_NAMES_HELPER(x, ...) DETAIL_VXG_STRINGIFY(x) DETAIL_VXG_MAP(DETAIL_VXG_STRINGIFY_ARGUMENT, __VA_ARGS__)
#define DETAIL_VXG_PROCESS_NAMES(...) \
	vxg::utilities::tmp::StringPack<__VA_OPT__(DETAIL_VXG_PROCESS_NAMES_HELPER(__VA_ARGS__))>

#define DETAIL_VXG_PROCESS_OFFSETS_HELPER(x, ...) DETAIL_VXG_GET_OFFSET(x) DETAIL_VXG_MAP(DETAIL_VXG_GET_OFFSET_ARGUMENT, __VA_ARGS__)
#define DETAIL_VXG_PROCESS_OFFSETS(...) \
	vxg::utilities::tmp::ValuePack<size_t __VA_OPT__(, DETAIL_VXG_PROCESS_OFFSETS_HELPER(__VA_ARGS__))>

#define VXG_REFLECTION_DESCRIBE_CLASS(classType, ...) \
	template <> \
	struct vxg::reflection::ClassInfoTraits<classType> {\
	private: \
		using ClassType = classType; \
	public: \
		static constexpr std::string_view name = #classType; \
		using Members = vxg::reflection::GenerateMembers_t<classType, DETAIL_VXG_SCAN0(DETAIL_VXG_PROCESS_TYPES(__VA_ARGS__)), DETAIL_VXG_SCAN0(DETAIL_VXG_PROCESS_NAMES(__VA_ARGS__)), DETAIL_VXG_SCAN0(DETAIL_VXG_PROCESS_OFFSETS(__VA_ARGS__))>; \
	};

#endif // VXG_REFLECTION_MACROS_HPP