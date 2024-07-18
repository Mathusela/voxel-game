module;

#include <glm/glm.hpp>

#include <vector>

export module voxel_game.core.rendering.meshing:mesher_implementations;

import :mesher;
import voxel_game.core.structs;

namespace vxg::core::rendering::meshing {

	std::vector<vxg::core::structs::Vertex> get_cube_verts(glm::vec3 position) {
		return std::vector<vxg::core::structs::Vertex>{
			// Front face
			{.position = position + glm::vec3{ -0.5, -0.5, 0.5 }},
			{ .position = position + glm::vec3{0.5, -0.5, 0.5} },
			{ .position = position + glm::vec3{0.5, 0.5, 0.5} },

			{ .position = position + glm::vec3{0.5, 0.5, 0.5} },
			{ .position = position + glm::vec3{-0.5, 0.5, 0.5} },
			{ .position = position + glm::vec3{-0.5, -0.5, 0.5} },

			// Back face
			{ .position = position + glm::vec3{-0.5, -0.5, -0.5} },
			{ .position = position + glm::vec3{-0.5, 0.5, -0.5} },
			{ .position = position + glm::vec3{0.5, 0.5, -0.5} },

			{ .position = position + glm::vec3{0.5, -0.5, -0.5} },
			{ .position = position + glm::vec3{-0.5, -0.5, -0.5} },
			{ .position = position + glm::vec3{0.5, 0.5, -0.5} },

			// Top face	
			{ .position = position + glm::vec3{-0.5, 0.5, -0.5} },
			{ .position = position + glm::vec3{-0.5, 0.5, 0.5} },
			{ .position = position + glm::vec3{0.5, 0.5, 0.5} },

			{ .position = position + glm::vec3{0.5, 0.5, -0.5} },
			{ .position = position + glm::vec3{-0.5, 0.5, -0.5} },
			{ .position = position + glm::vec3{0.5, 0.5, 0.5} },

			// Bottom face
			{ .position = position + glm::vec3{-0.5, -0.5, -0.5} },
			{ .position = position + glm::vec3{0.5, -0.5, -0.5} },
			{ .position = position + glm::vec3{0.5, -0.5, 0.5} },

			{ .position = position + glm::vec3{-0.5, -0.5, 0.5} },
			{ .position = position + glm::vec3{-0.5, -0.5, -0.5} },
			{ .position = position + glm::vec3{0.5, -0.5, 0.5} },

			// Right face
			{ .position = position + glm::vec3{0.5, -0.5, -0.5} },
			{ .position = position + glm::vec3{0.5, 0.5, -0.5} },
			{ .position = position + glm::vec3{0.5, 0.5, 0.5} },

			{ .position = position + glm::vec3{0.5, -0.5, 0.5} },
			{ .position = position + glm::vec3{0.5, -0.5, -0.5} },
			{ .position = position + glm::vec3{0.5, 0.5, 0.5} },

			// Left face
			{ .position = position + glm::vec3{-0.5, -0.5, -0.5} },
			{ .position = position + glm::vec3{-0.5, -0.5, 0.5} },
			{ .position = position + glm::vec3{-0.5, 0.5, 0.5} },

			{ .position = position + glm::vec3{-0.5, 0.5, -0.5} },
			{ .position = position + glm::vec3{-0.5, -0.5, -0.5} },
			{ .position = position + glm::vec3{-0.5, 0.5, 0.5} }
		};
	}

};

// EXPORTED
export namespace vxg::core::rendering::meshing {

	// === NAIVE MESHER ===
	class NaiveMesher;

	template <>
	struct MesherTraits<NaiveMesher> {
		using VertexType = vxg::core::structs::Vertex;
		using VoxelType = bool;
	};

	class NaiveMesher : public Mesher<NaiveMesher> {
		using Base = Mesher<NaiveMesher>;
		friend Base;
		
		[[nodiscard]]
		std::vector<Base::VertexType> mesh_impl(const std::vector<Base::VoxelType>& voxels) const noexcept {
			assert(voxels.size() == chunkSize.x * chunkSize.y * chunkSize.z);
			
			std::vector<Base::VertexType> verts;
			for (size_t flatIndex = 0; flatIndex < voxels.size(); flatIndex++) {
				if (voxels[flatIndex]) {
					auto newVerts = get_cube_verts(flat_index_to_position(flatIndex));
					verts.insert(verts.end(), newVerts.begin(), newVerts.end());
				}
			}
			
			return verts;
		}

	public:
		NaiveMesher(const glm::vec3& chunkSize)
			: Base(chunkSize) {}
	};

	// === CULLED MESHER ===
	class CulledMesher;

	template <>
	struct MesherTraits<CulledMesher> {
		using VertexType = vxg::core::structs::Vertex;
		using VoxelType = bool;
	};

	class CulledMesher : public Mesher<CulledMesher> {
		using Base = Mesher<CulledMesher>;
		friend Base;

		[[nodiscard]]
		bool is_voxel(const std::vector<bool>& voxels, glm::vec3 position) const noexcept {
			if (position.x < 0 || position.x >= chunkSize.x || position.y < 0 || position.y >= chunkSize.y || position.z < 0 || position.z >= chunkSize.z) {
				return false;
			}
			auto flatOffset = position_to_flat_index(position);
			return voxels[flatOffset];
		}

		[[nodiscard]]
		bool should_draw_face(const std::vector<bool>& voxels, glm::vec3 position, glm::vec3 normal) const noexcept {
			return !is_voxel(voxels, position + normal);
		}

		[[nodiscard]]
		std::vector<Base::VertexType> mesh_impl(const std::vector<Base::VoxelType>& voxels) const noexcept {
			assert(voxels.size() == chunkSize.x * chunkSize.y * chunkSize.z);

			std::vector<vxg::core::structs::Vertex> verts;
			for (size_t flatIndex = 0; flatIndex < voxels.size(); flatIndex++) {
				if (!voxels[flatIndex])
					continue;
				auto coords = flat_index_to_position(flatIndex);
				auto newVerts = get_cube_verts(coords);
				for (size_t i = 0; i < newVerts.size(); i += 6) {
					auto normal = glm::normalize(glm::cross(newVerts[i+1].position - newVerts[i].position, newVerts[i+2].position - newVerts[i].position));
					if (should_draw_face(voxels, coords, normal)) {
						verts.insert(verts.end(), newVerts.begin() + i, newVerts.begin() + i + 6);
					}
				}
			}
		
			return verts;
		}

	public:
		CulledMesher(const glm::vec3& chunkSize)
			: Base(chunkSize) {}
	};

}; // namespace vxg::core::rendering::meshing