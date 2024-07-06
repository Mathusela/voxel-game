export module voxel_game.utilities.tmp;

export namespace vxg::utilities::tmp {

	template <typename T>
	struct ExtractNestedTemplate;
	
	template <template <typename> typename T, typename Nested>
	struct ExtractNestedTemplate<T<Nested>> {
		using Type = Nested;
	};

	template <typename T>
	using ExtractNestedTemplate_t = ExtractNestedTemplate<T>::Type;

}	// namespace vxg::utilities::tmp