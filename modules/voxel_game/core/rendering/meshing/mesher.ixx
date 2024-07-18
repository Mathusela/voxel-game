module;

#include <glm/glm.hpp>

#include <vector>

export module voxel_game.core.rendering.meshing:mesher;

export namespace vxg::core::rendering::meshing {

	template <typename T>
	struct MesherTraits;

	template <typename Derived>
	class Mesher {
	public:
		using VertexType = typename MesherTraits<Derived>::VertexType;
		using VoxelType = typename MesherTraits<Derived>::VoxelType;
		const glm::vec3 chunkSize;
	
	private:
		Mesher(const glm::vec3& chunkSize)
			: chunkSize(chunkSize) {}
		friend Derived;
	
		[[nodiscard]]
		const Derived* derived_instance() const noexcept {
			return static_cast<const Derived*>(this);
		}
	
		[[nodiscard]]
		Derived* derived_instance() noexcept {
			return static_cast<Derived*>(this);
		}
	
		[[nodiscard]]
		glm::vec3 flat_index_to_position(size_t index) const noexcept {
			return glm::vec3(
				static_cast<float>(index % static_cast<const int>(chunkSize.x)),
				static_cast<float>((index / static_cast<const int>(chunkSize.x)) % static_cast<const int>(chunkSize.y)),
				static_cast<float>(index / (static_cast<const int>(chunkSize.x) * static_cast<const int>(chunkSize.y)))
			);
		}
		
		[[nodiscard]]
		size_t position_to_flat_index(glm::vec3 position) const noexcept {
			return static_cast<size_t>(
				position.x +
				position.y * chunkSize.x +
				position.z * chunkSize.y * chunkSize.x
			);
		}
	
	public:
		[[nodiscard]]
		std::vector<VertexType> mesh(const std::vector<VoxelType>& voxels) const
			noexcept(noexcept(derived_instance()->mesh_impl(voxels)))
		{
			return derived_instance()->mesh_impl(voxels);
		}
	};

	template <typename T>
	concept mesher_implementation = std::is_base_of_v<Mesher<T>, T>;

}; // namespace vxg::core::rendering::meshing