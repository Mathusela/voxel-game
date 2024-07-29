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

	// === GREEDY MESHER ===
	class GreedyMesher;

	template <>
	struct MesherTraits<GreedyMesher> {
		using VertexType = vxg::core::structs::Vertex;
		using VoxelType = bool;
	};

	class GreedyMesher : public Mesher<GreedyMesher> {
		using Base = Mesher<GreedyMesher>;
		friend Base;

		struct Quad {
			glm::vec3 min;
			glm::vec3 max;

			bool area() const noexcept {
				auto diagonal = max - min;
				return diagonal.x * diagonal.y;
			}
		};

		[[nodiscard]]
		bool is_voxel(const std::vector<bool>& voxels, glm::vec3 position) const noexcept {
			if (position.x < 0 || position.x >= chunkSize.x || position.y < 0 || position.y >= chunkSize.y || position.z < 0 || position.z >= chunkSize.z) {
				return false;
			}
			auto flatOffset = position_to_flat_index(position);
			return voxels[flatOffset];
		}

		[[nodiscard]]
		std::vector<Base::VertexType> get_quad_verts(const Quad& quad, glm::vec3 normal) const noexcept {
			glm::vec3 diagonal = quad.max - quad.min;
			
			auto maxAxis = 0;
			glm::vec3 absDiagonal = glm::abs(diagonal);
			if (absDiagonal.y > absDiagonal[maxAxis]) maxAxis = 1;
			if (absDiagonal.z > absDiagonal[maxAxis]) maxAxis = 2;
			
			glm::vec3 v1(0.0);
			v1[static_cast<size_t>(maxAxis)] = 1.0f;
			glm::vec3 v2 = glm::cross(normal, v1);

			auto height = glm::dot(diagonal, v1);
			auto width = glm::dot(diagonal, v2);
			
			glm::vec3 tl = quad.min + glm::vec3(height)*v1;
			glm::vec3 br = quad.min + glm::vec3(width)*v2;

			return std::vector<Base::VertexType>{
				{.position = quad.min},
				{.position = tl},
				{.position = quad.max},

				{.position = quad.min},
				{.position = quad.max},
				{.position = br}
			};
		}

		[[nodiscard]]
		glm::vec3 flat_index_to_position_slice(size_t flatIndex, int primaryDimension, glm::vec2 traversalDimensions, int slice) const noexcept {
			glm::vec3 position;
			position[primaryDimension] = static_cast<float>(slice);
			position[static_cast<size_t>(traversalDimensions.x)] = static_cast<float>(flatIndex % static_cast<int>(chunkSize[static_cast<size_t>(traversalDimensions.x)]));
			position[static_cast<size_t>(traversalDimensions.y)] = static_cast<float>(flatIndex / static_cast<int>(chunkSize[static_cast<size_t>(traversalDimensions.x)]));
			return position;
		}

		[[nodiscard]]
		size_t position_to_flat_index_slice(glm::vec3 position, glm::vec2 traversalDimensions) const noexcept {
			return static_cast<size_t>(
				position[static_cast<size_t>(traversalDimensions.x)] +
				position[static_cast<size_t>(traversalDimensions.y)] * chunkSize[static_cast<size_t>(traversalDimensions.x)]
			);
		}

		[[nodiscard]]
		bool is_face_visible(glm::vec3 position, int primaryDimension, const std::vector<Base::VoxelType>& voxels) const noexcept {
			auto positionBehind = position;
			positionBehind[primaryDimension]--;

			return !is_voxel(voxels, position) || !is_voxel(voxels, positionBehind);
		}

		[[nodiscard]]
		bool is_face(glm::vec3 position, int primaryDimension, const std::vector<Base::VoxelType>& voxels) const noexcept {
			auto positionBehind = position;
			positionBehind[primaryDimension]--;

			return is_voxel(voxels, position) || is_voxel(voxels, positionBehind);
		}

		struct QuadResult {
			Quad quad;
			bool visible;
		};

		[[nodiscard]]
		QuadResult get_best_quad(glm::vec3 position, int primaryDimension, glm::vec2 traversalDimensions, const std::vector<Base::VoxelType>& voxels, const std::vector<bool>& meshedFaces) const noexcept {
			bool visible = false;
			glm::vec3 max = position;

			// Traverse the first traversal dimension
			while (true) {
				if (!visible)
					visible = is_face_visible(max, primaryDimension, voxels);

				max[static_cast<size_t>(traversalDimensions.x)]++;
				auto consideredFlatIndex = position_to_flat_index_slice(max, traversalDimensions);
				if (!is_face(max, primaryDimension, voxels) || meshedFaces[consideredFlatIndex])
					break;
			}
			max[static_cast<size_t>(traversalDimensions.x)]--;

			// Traverse the second traversal dimension
			while (true) {
				bool complete = true;
				for (int i = static_cast<int>(position[static_cast<size_t>(traversalDimensions.x)]); i <= max[static_cast<size_t>(traversalDimensions.x)]; i++) {
					auto consideredPosition = max;
					consideredPosition[static_cast<size_t>(traversalDimensions.y)]++;
					consideredPosition[static_cast<size_t>(traversalDimensions.x)] = static_cast<float>(i);
					auto consideredFlatIndex = position_to_flat_index_slice(consideredPosition, traversalDimensions);

					if (!is_face(consideredPosition, primaryDimension, voxels) || meshedFaces[consideredFlatIndex]) {
						complete = false;
						break;
					}

					if (!visible)
						visible = is_face_visible(consideredPosition, primaryDimension, voxels);
				}

				if (!complete)
					break;

				max[static_cast<size_t>(traversalDimensions.y)]++;
			}

			// Convert from voxel coords (points to bottom left of face) to top right of face
			auto maxAdjusted = max;
			maxAdjusted[static_cast<size_t>(traversalDimensions.x)]++;
			maxAdjusted[static_cast<size_t>(traversalDimensions.y)]++;

			return { {position, maxAdjusted}, visible };
		}

		[[nodiscard]]
		std::vector<Base::VertexType> handle_slice(int slice, int primaryDimension, glm::vec2 traversalDimensions, const std::vector<Base::VoxelType>& voxels) const noexcept {
			std::vector<Base::VertexType> verts;
			
			std::vector<bool> meshedFaces(static_cast<size_t>(chunkSize[static_cast<size_t>(traversalDimensions.x)] * chunkSize[static_cast<size_t>(traversalDimensions.y)]));

			// For each unmeshed face
			for (size_t flatIndex = 0; flatIndex < meshedFaces.size(); flatIndex++) {
				glm::vec3 position = flat_index_to_position_slice(flatIndex, primaryDimension, traversalDimensions, slice);

				if (meshedFaces[flatIndex] || !is_face(position, primaryDimension, voxels))
					continue;

				auto quadResult1 = get_best_quad(position, primaryDimension, traversalDimensions, voxels, meshedFaces);
				//auto quadResult2 = get_best_quad(position, primaryDimension, { traversalDimensions.y, traversalDimensions.x }, voxels, meshedFaces);
				
				//auto bestQuadResult = std::max(quadResult1, quadResult2, [](const auto& q1, const auto& q2) {
				//	return q1.quad.area() < q2.quad.area();
				//});
				auto bestQuadResult = quadResult1;

				if (bestQuadResult.visible) {
					glm::vec3 normal(0.0f);
					normal[primaryDimension] = 1.0f;

					auto quadVerts = get_quad_verts(bestQuadResult.quad, normal);
					verts.insert(verts.end(), quadVerts.begin(), quadVerts.end());
				}

				for (int i = static_cast<int>(position[static_cast<size_t>(traversalDimensions.x)]); i < bestQuadResult.quad.max[static_cast<size_t>(traversalDimensions.x)]; i++) {
					for (int j = static_cast<int>(position[static_cast<size_t>(traversalDimensions.y)]); j < bestQuadResult.quad.max[static_cast<size_t>(traversalDimensions.y)]; j++) {
						auto facePosition = position;
						facePosition[static_cast<size_t>(traversalDimensions.x)] = static_cast<float>(i);
						facePosition[static_cast<size_t>(traversalDimensions.y)] = static_cast<float>(j);
						meshedFaces[position_to_flat_index_slice(facePosition, traversalDimensions)] = true;
					}
				}

			}

			return verts;
		}

		[[nodiscard]]
		std::vector<Base::VertexType> mesh_impl(const std::vector<Base::VoxelType>& voxels) const noexcept {
			assert(voxels.size() == chunkSize.x * chunkSize.y * chunkSize.z);

			std::vector<Base::VertexType> verts;

			for (int dimension = 0; dimension < 3; dimension++) {
				for (int slice = 0; slice < chunkSize[dimension]+1; slice++) {
					auto newVerts = handle_slice(slice, dimension, { (dimension + 1) % 3, (dimension + 2) % 3 }, voxels);
					verts.insert(verts.end(), newVerts.begin(), newVerts.end());
				}
			}

			return verts;
		}

	public:
		GreedyMesher(const glm::vec3& chunkSize)
			: Base(chunkSize) {}
	};

}; // namespace vxg::core::rendering::meshing