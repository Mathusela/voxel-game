module;

#include "voxel_game/debug_macros.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <cstdint>
#include <vector>
#include <set>
#include <functional>
#include <algorithm>
#include <iterator>
#include <ranges>
#include <cassert>

export module voxel_game.core.memory:opengl_allocator;

import :gpu_allocator;
import voxel_game.core.structs;
import voxel_game.core.rendering.structs;
import voxel_game.utilities;
import voxel_game.exceptions;
import voxel_game.logging;
import voxel_game.reflection;

namespace vxg::core::memory {

	/**
	 * @brief Resize an OpenGL buffer retaining its contents.
	 * @param buffer Buffer to resize.
	 * @param newSize New size of the buffer in bytes.
	 */
	void resize_buffer(GLuint buffer, GLsizeiptr newSize) {
		// Get current size of buffer and usage hint for the data
		GLint oldSize;
		glGetNamedBufferParameteriv(buffer, GL_BUFFER_SIZE, &oldSize);
		GLint usageHint;
		glGetNamedBufferParameteriv(buffer, GL_BUFFER_USAGE, &usageHint);

		// Create a temporary buffer in order to hold data while resizing buffer
		GLuint tempBuffer;
		glCreateBuffers(1, &tempBuffer);
		vxg::utilities::DeferredFunction<decltype(glDeleteBuffers), GLsizei, GLuint*> deferredDeleteTempBuffer(glDeleteBuffers, 1, &tempBuffer);
		// TODO: Should this use glNamedBufferStorage? Otherwise should this use GL_STATIC_READ?
		glNamedBufferData(tempBuffer, oldSize, nullptr, GL_STATIC_COPY);

		// Copy contents of buffer to temporary buffer
		glCopyNamedBufferSubData(buffer, tempBuffer, 0, 0, oldSize);

		// Resize buffer
		glNamedBufferData(buffer, newSize, nullptr, usageHint);

		// Copy data from temporary buffer back into the resized buffer
		GLsizeiptr copySize = std::min(newSize, static_cast<GLsizeiptr>(oldSize));
		glCopyNamedBufferSubData(tempBuffer, buffer, 0, 0, copySize);
	}

	/**
	 * @brief Type of data held in an allocation block.
	 */
	enum class ResourceType {
		Vertex,
		Data,
		Draw
	};

	
	// TODO: Rename to OpenGLAllocationIdentifier
	/**
	 * @brief Identifier for memory allocated by OpenGLAllocator.
	 */
	struct AllocationIdentifier {
		std::uint16_t poolIndex;	//!< Index of the owning memory pool.
		size_t offset;	//!< Offset into the buffer in multiples of the sizeof the resource type.
		size_t size;	//!< Size of the allocation in multiples of sizeof the resource type.
		ResourceType type;	//!< Type of resource allocated.
	};

	/**
	 * @brief Get an allocation block representing the remaining free memory after an allocation to the start of a free block.
	 * @param freeMemory Free block which has been allocated to.
	 * @param size Size of the allocation.
	 * @return Allocation block representing the remaining free memory.
	 */
	AllocationIdentifier get_remaining_free_memory(const AllocationIdentifier& freeMemory, size_t size) {
		return AllocationIdentifier{
			.poolIndex = freeMemory.poolIndex,
			.offset = freeMemory.offset + size,
			.size = freeMemory.size - size,
			.type = freeMemory.type
		};
	}

};	// namespace vxg::core::memory

/**
 * @brief Order allocation identifiers by memory location.
 * 
 * This is used to efficiently merge blocks of memory in free lists in vxg::core::memory::OpenGLAllocator.
 */
template <>
struct std::less<vxg::core::memory::AllocationIdentifier> {
	constexpr bool operator()
		(const vxg::core::memory::AllocationIdentifier& left, const vxg::core::memory::AllocationIdentifier& right) const
	{
		return left.poolIndex == right.poolIndex ? left.offset < right.offset : left.poolIndex < right.poolIndex;
	}
};

namespace vxg::core::memory {

	/**
	 * @brief If possible merge an allocation block with surrounding blocks from a free list, removing the surrounding blocks from the list.
	 * @param block The block to merge.
	 * @param freeList The free list to merge with.
	 * @return The resulting merged block.
	 */
	[[nodiscard]]
	AllocationIdentifier merge_with_surrounding_blocks(const AllocationIdentifier& block, std::set<AllocationIdentifier>& freeList) {
		if (freeList.empty())
			return block;

		AllocationIdentifier merged = block;
		auto adjacentBlock = freeList.upper_bound(block);	// First element at a memory location greater than the block
		bool validAdjacentBlock = adjacentBlock != freeList.end();

		// Greater block
		if (validAdjacentBlock) {	// If there is a greater block, check if it can be merged
			auto upperBound = adjacentBlock;	// Copy for use in merge
			adjacentBlock--;	// Decrement to get the lesser block - do this now as we may erase lowerBound invalidating the iterator if we do not increment

			if (upperBound->poolIndex == block.poolIndex && upperBound->poolIndex == block.offset + block.size) {
				// Merge
				merged.size += upperBound->size;

				// Remove merged block from the free list
				freeList.erase(upperBound);
			}
		}
		else { // If there was no greater block, the lesser block must be the last in the set
			adjacentBlock = std::prev(freeList.end());
		}

		// Lesser block
		validAdjacentBlock = adjacentBlock != freeList.end();
		if (validAdjacentBlock) {	// If there is a lesser block, check if it can be merged
			if (adjacentBlock->poolIndex == block.poolIndex && adjacentBlock->offset + adjacentBlock->size == block.offset) {
				// Merge
				merged.offset = adjacentBlock->offset;
				merged.size += adjacentBlock->size;

				// Remove merged block from free list
				freeList.erase(adjacentBlock);
			}
		}

		return merged;
	}
	
	/**
	 * @brief Storage and metadata for an OpenGL allocation buffer.
	 * 
	 * Used with external free memory tracking when free memory is tracked between multiple buffers.
	 * 
	 * @see MemoryPoolLocalBufferStorage
	 */
	struct BufferStorage {
		GLuint buffer;
		GLsizeiptr size;	//!< Size of the buffer in multiples of sizeof the resource type.
		GLsizeiptr freeSize;	//!< Free space in the buffer in multiples of sizeof the resource type.
	};

	/**
	 * @brief Storage and metadata for an OpenGL allocation buffer with local free memory tracking.
	 * 
	 * Used when free memory is tracked on a per-buffer basis.
	 * 
	 * @see BufferStorage
	 */
	struct MemoryPoolLocalBufferStorage : public BufferStorage {
		std::set<AllocationIdentifier> freeMemory;
	};

	/**
	 * @brief Unit of memory resources for OpenGLAllocator.
	 */
	struct MemoryPool {
		// Resources
		GLuint vao;
		BufferStorage vertex;
		MemoryPoolLocalBufferStorage data;
		MemoryPoolLocalBufferStorage draw;
		
		// Metadata
		uint16_t index;	//!< Index of the memory pool in the allocator's memory pool list.
		GLsizei storedDrawCommands;	//!< Number of draw commands stored in the draw buffer.
	};

};	// namespace vxg::core::memory

// EXPORTED
export namespace vxg::core::memory {

	/**
	 * @brief ExposedDrawResourcesType of OpenGLAllocator.
	 * @see GpuAllocatorTraits<OpenGLAllocator>
	 */
	struct OpenGLDrawResources {
		GLuint vao;
		GLuint drawCommandBuffer;
		GLsizei drawCommandCount;
	};

	class OpenGLAllocator;
	//! @see GpuAllocatorTraits
	//! @see OpenGLAllocator
	template <>
	struct GpuAllocatorTraits<OpenGLAllocator> {
		using VertexType = vxg::core::structs::Vertex;
		using DataType = vxg::core::structs::ObjectData;
		using DrawType = vxg::core::rendering::structs::DrawArraysIndirectCommand;
		using AllocationIdentifier = vxg::core::memory::AllocationIdentifier;
		using ExposedDrawResourcesType = vxg::core::memory::OpenGLDrawResources;
	};

}; // namespace vxg::core::memory

namespace vxg::core::memory {

	/**
	 * @brief Convert a ResourceType enum to it's corresponding type.
	 * @tparam resourceEnum Enum to convert.
	 */
	template <ResourceType resourceEnum>
	struct ResourceEnumToType {};
	template <>
	struct ResourceEnumToType<ResourceType::Vertex> {
		using Type = GpuAllocatorTraits<OpenGLAllocator>::VertexType;
	};
	template <>
	struct ResourceEnumToType<ResourceType::Data> {
		using Type = GpuAllocatorTraits<OpenGLAllocator>::DataType;
	};
	template <>
	struct ResourceEnumToType<ResourceType::Draw> {
		using Type = GpuAllocatorTraits<OpenGLAllocator>::DrawType;
	};
	//! @copydoc ResourceEnumToType
	template <ResourceType resourceEnum>
	using ResourceEnumToType_t = typename ResourceEnumToType<resourceEnum>::Type;

}; // namespace vxg::core::memory

// EXPORTED
export namespace vxg::core::memory {

	/**
	 * @brief GPUAllocator implementation for OpenGL.
	 * 
	 * Manages multiple memory pools for vertex, data, and draw resources, utilizing an exponential growth strategy for resizing buffers with a growth factor of 2.
	 * 
	 * Draw and data resources are allocated to the same memory pool as the associated vertex resources.
	 * Deallocated memory is merged with adjacent free memory blocks to reduce fragmentation and allow reuse of memory.
	 * 
	 * A GpuAllocatorTraits specialization is provided.
	 * 
	 * @see GpuAllocator
	 * @see GpuAllocatorTraits<OpenGLAllocator>
	 */
	class OpenGLAllocator final : public GpuAllocator<OpenGLAllocator> {
		using Base = GpuAllocator<OpenGLAllocator>;
		friend Base;

		const uint16_t m_numMemoryPools;
		const size_t m_vertexBufferInitialSize;
		const size_t m_dataBufferInitialSize;
		const size_t m_drawBufferInitialSize;

		std::vector<MemoryPool> m_memoryPools;
		std::set<AllocationIdentifier> m_freeVertexMemory;

		/**
		 * @brief Resize a buffer such that it can contain the required number of elements of the specified resource type.
		 * @tparam resourceType The underlying resource type of the buffer.
		 * @param bufferStorage The buffer storage containing the buffer to resize.
		 * @param requiredSize The number of elements required.
		 * @param poolIndex The index of the memory pool the buffer belongs to.
		 * @return The allocation identifier of the new memory due to resizing.
		 */
		template <ResourceType resourceType>
		[[nodiscard]]
		AllocationIdentifier resize_to_fit(BufferStorage& bufferStorage, size_t requiredSize, uint16_t poolIndex) noexcept {
			// Calculate new size
			auto bufferSize = bufferStorage.size;
			auto freeSize = bufferStorage.freeSize;
			GLsizeiptr newSize = bufferSize * 2;
			while (newSize - bufferSize + freeSize < static_cast<GLsizeiptr>(requiredSize))	// While the new size is not large enough to contain the new allocation
				newSize *= 2;
		
			// Resize buffer
			resize_buffer(bufferStorage.buffer, newSize*sizeof(ResourceEnumToType_t<resourceType>));	// Convert to bytes
			bufferStorage.size = newSize;
			bufferStorage.freeSize += newSize - bufferSize;

			return AllocationIdentifier {
				.poolIndex = poolIndex,	// Index of the resized buffer's memory pool
				.offset = static_cast<size_t>(bufferSize),	// Offset of the new allocation is the end of the old buffer
				.size = static_cast<size_t>(newSize - bufferSize),	// Size of the new allocation is the difference between the new and old buffer sizes
				.type = resourceType
			};
		}

		[[nodiscard]]
		AllocationIdentifier allocate_vertices_impl(size_t size) noexcept {
			// Get the smallest free block that can contain the data preferring sparse buffers
			auto validBlocks = m_freeVertexMemory
				| std::views::filter([size](const AllocationIdentifier& mem) {return size <= mem.size; });
			auto freeIt = std::ranges::min_element(validBlocks, [this](const AllocationIdentifier& left, const AllocationIdentifier& right) {
				if (m_memoryPools[left.poolIndex].vertex.freeSize == m_memoryPools[right.poolIndex].vertex.freeSize)
					return left.size < right.size;
				else
					return m_memoryPools[left.poolIndex].vertex.freeSize > m_memoryPools[right.poolIndex].vertex.freeSize;
			});

			AllocationIdentifier free;
			if (freeIt != validBlocks.end()) {	// If there is a valid block, allocate it
				free = *freeIt;
				m_freeVertexMemory.erase(freeIt.base());
			}
			else { // Otherwise, resize the smallest VBO
				// Get the smallest VBO
				auto smallestMemPoolIt = std::ranges::min_element(m_memoryPools, [](const MemoryPool& left, const MemoryPool& right) {
					return left.vertex.size < right.vertex.size;
				});

				auto newAllocation = resize_to_fit<ResourceType::Vertex>(smallestMemPoolIt->vertex, size, smallestMemPoolIt->index);

				// Logging
				vxg::logging::std_debug_log().log(vxg::logging::LogType::Info, "<OpenGL Allocator> RESIZING VERTS: {{Memory Pool: {}}}\n", smallestMemPoolIt->index);
				
				// Merge this new free data for allocation
				free = merge_with_surrounding_blocks(newAllocation, m_freeVertexMemory);
			}

			// Add remainder of free memory back to free list
			if (free.size - size > 0) {
				auto remainder = get_remaining_free_memory(free, size);
				m_freeVertexMemory.insert(merge_with_surrounding_blocks(remainder, m_freeVertexMemory));
			}
			m_memoryPools[free.poolIndex].vertex.freeSize -= size;

			// Return allocated memory identifier
			auto alloc = free;
			alloc.size = size;
			// Logging
			vxg::logging::std_debug_log().log(vxg::logging::LogType::Info, "<OpenGL Allocator> ALLOC VERTS: {{Memory Pool: {}, Offset: {}, Size: {}}}\n", alloc.poolIndex, alloc.offset, alloc.size);
			return alloc;
		}

		[[nodiscard]]
		AllocationIdentifier allocate_data_impl(AllocationIdentifier verticesLocation) noexcept {
			assert(verticesLocation.type == ResourceType::Vertex && "Passed non vertex memory as vertex allocation in data allocation.");	// Ensure input allocation is for vertices

			auto& memPool = m_memoryPools[verticesLocation.poolIndex];
			AllocationIdentifier free;

			if (!memPool.data.freeMemory.empty()) {	// If there is free memory, allocate it
				auto freeIt = memPool.data.freeMemory.begin();
				free = *freeIt;
				memPool.data.freeMemory.erase(freeIt);
			}
			else {	// If there is no free memory, resize the buffer
				// No need to merge as the free list is empty
				free = resize_to_fit<ResourceType::Data>(memPool.data, 1, memPool.index);

				// Logging
				vxg::logging::std_debug_log().log(vxg::logging::LogType::Info, "<OpenGL Allocator> RESIZING DATA: {{Memory Pool: {}}}\n", memPool.index);
			}

			// Add remainder of free region back to free list
			if (free.size > 1) {
				// No need to merge as the free list is either empty or the new allocation is at the end of the buffer
				memPool.data.freeMemory.insert(get_remaining_free_memory(free, 1));
			}
			memPool.data.freeSize -= 1;

			// Return allocated memory identifier
			auto alloc = free;
			alloc.size = 1;
			// Logging
			vxg::logging::std_debug_log().log(vxg::logging::LogType::Info, "<OpenGL Allocator> ALLOC DATA: {{Memory Pool: {}, Offset: {}, Size: {}}}\n", alloc.poolIndex, alloc.offset, alloc.size);
			return alloc;
		}

		[[nodiscard]]
		AllocationIdentifier allocate_draw_impl(AllocationIdentifier verticesLocation, [[maybe_unused]] AllocationIdentifier dataLocation) noexcept {
			assert(verticesLocation.type == ResourceType::Vertex && dataLocation.type == ResourceType::Data && "Mismatched allocation resource types in draw allocation.");	// Ensure input allocation resource types are correct
			assert(verticesLocation.poolIndex == dataLocation.poolIndex && "Attempted to allocate a draw call using vertex and data allocations in mismatched memory pools.");	// Allocations should be in the same memory pool

			auto& memPool = m_memoryPools[verticesLocation.poolIndex];
			AllocationIdentifier free;

			if (!memPool.draw.freeMemory.empty()) {	// If there is free memory, allocate it
				auto freeIt = memPool.draw.freeMemory.begin();
				free = *freeIt;
				memPool.draw.freeMemory.erase(freeIt);
			}
			else {	// If there is no free memory, resize the buffer
				// No need to merge as the free list is empty
				free = resize_to_fit<ResourceType::Draw>(memPool.draw, 1, memPool.index);

				// Logging
				vxg::logging::std_debug_log().log(vxg::logging::LogType::Info, "<OpenGL Allocator> RESIZING DRAW: {{Memory Pool: {}}}\n", memPool.index);
			}

			// Add remainder of free region back to free list
			if (free.size > 1) {
				// No need to merge as the free list is either empty or the new allocation is at the end of the buffer
				memPool.draw.freeMemory.insert(get_remaining_free_memory(free, 1));
			}
			memPool.draw.freeSize -= 1;

			// Update number of stored draw commands
			auto& storedDrawCommands = memPool.storedDrawCommands;
			storedDrawCommands = std::max(storedDrawCommands, static_cast<GLsizei>(free.offset) + 1);

			// Return allocated memory identifier
			auto alloc = free;
			alloc.size = 1;
			// Logging
			vxg::logging::std_debug_log().log(vxg::logging::LogType::Info, "<OpenGL Allocator> ALLOC DRAW: {{Memory Pool: {}, Offset: {}, Size: {}}}\n", alloc.poolIndex, alloc.offset, alloc.size);
			return alloc;
		}

		void deallocate_impl(AllocationIdentifier alloc) {
			auto& memPool = m_memoryPools[alloc.poolIndex];

			// Get correct resources based on the type of the allocated memory
			std::set<AllocationIdentifier>* freeList;
			BufferStorage* storage;

			switch (alloc.type) {
			case ResourceType::Vertex:
				freeList = &m_freeVertexMemory;
				storage = &memPool.vertex;
				break;
			case ResourceType::Data:
				freeList = &memPool.data.freeMemory;
				storage = &memPool.data;
				break;
			case ResourceType::Draw:
				freeList = &memPool.draw.freeMemory;
				storage = &memPool.draw;
				break;
			default:
				throw vxg::exceptions::InvalidDataError("Encountered invalid resource type when deallocating GPU memory.");
			}

			auto merged = merge_with_surrounding_blocks(alloc, *freeList);

			// Add block to free list to deallocate
			freeList->insert(merged);
			storage->freeSize += alloc.size;
		}

		void construct_vertices_impl(AllocationIdentifier alloc, const std::vector<VertexType>& value) noexcept {
			assert(alloc.type == ResourceType::Vertex && "Attempted to construct non vertex memory as vertices.");	// Ensure the allocation is for vertices
			assert(alloc.size == value.size() && "Size of allocation does not match size of provided data in construction of vertices.");	// Ensure the size of the allocation matches the size of the data

			auto offsetInBytes = alloc.offset*sizeof(VertexType);
			auto sizeInBytes = alloc.size*sizeof(VertexType);
			glNamedBufferSubData(m_memoryPools[alloc.poolIndex].vertex.buffer, offsetInBytes, sizeInBytes, value.data());
		}

		void construct_data_impl(AllocationIdentifier alloc, const DataType& value) noexcept {
			assert(alloc.type == ResourceType::Data && "Attempted to construct non data memory as data.");	// Ensure the allocation is for data
			assert(alloc.size == 1 && "Size of allocation does not match size of provided data in construction of data.");	// Ensure the size of the allocation matches the size of the data

			auto offsetInBytes = alloc.offset*sizeof(DataType);
			auto sizeInBytes = alloc.size*sizeof(DataType);
			glNamedBufferSubData(m_memoryPools[alloc.poolIndex].data.buffer, offsetInBytes, sizeInBytes, &value);
		}

		void construct_draw_impl(AllocationIdentifier drawAlloc, AllocationIdentifier vertexAlloc, AllocationIdentifier dataAlloc) noexcept {
			assert(drawAlloc.type == ResourceType::Draw && vertexAlloc.type == ResourceType::Vertex && dataAlloc.type == ResourceType::Data
				&& "Mismatched allocation resource types in draw construction.");	// Ensure allocation types are correct for each input
			assert(drawAlloc.poolIndex == vertexAlloc.poolIndex && vertexAlloc.poolIndex == dataAlloc.poolIndex && dataAlloc.poolIndex == drawAlloc.poolIndex
				&& "Mismatched memory pool indices in draw construction.");	// Allocations should be in the same memory pool
			
			auto& memPool = m_memoryPools[drawAlloc.poolIndex];

			DrawType drawCommand{
				.count = static_cast<GLuint>(vertexAlloc.size),
				.instanceCount = 1,
				.firstVertex = static_cast<GLuint>(vertexAlloc.offset),
				.baseInstance = static_cast<GLuint>(dataAlloc.offset)
			};

			auto offsetInBytes = drawAlloc.offset*sizeof(DrawType);
			auto sizeInBytes = drawAlloc.size*sizeof(DrawType);
			glNamedBufferSubData(memPool.draw.buffer, offsetInBytes, sizeInBytes, &drawCommand);
		}

		void destroy_impl(AllocationIdentifier alloc) noexcept {
			// Only draw commands need explicit cleanup, all other allocations are trivially destructible
			if (alloc.type == ResourceType::Draw) {
				auto& memPool = m_memoryPools[alloc.poolIndex];
				
				// Set instance of draw command to 0 to disable to command - this prevents deallocated/destroyed memory from being drawn
				GLuint instanceCount = 0;
				glNamedBufferSubData(memPool.draw.buffer, alloc.offset*sizeof(DrawType) + offsetof(DrawType, instanceCount), sizeof(DrawType::instanceCount), &instanceCount);
			}
		}

		// Assumes a valid OpenGL context is current
		void initialize_impl() noexcept {
			// Initialize memory pools
			for (uint16_t i = 0; i < m_numMemoryPools; i++) {
				m_memoryPools.push_back({});
				auto& memPool = m_memoryPools.back();

				// Initialize OpenGL resources
				// VAO
				glCreateVertexArrays(1, &memPool.vao);
				// Vertex buffer
				glCreateBuffers(1, &memPool.vertex.buffer);
				glNamedBufferData(memPool.vertex.buffer, m_vertexBufferInitialSize*sizeof(VertexType), nullptr, GL_DYNAMIC_COPY);
				// Data buffer
				glCreateBuffers(1, &memPool.data.buffer);
				glNamedBufferData(memPool.data.buffer, m_dataBufferInitialSize*sizeof(DataType), nullptr, GL_DYNAMIC_COPY);
				// Draw buffer
				glCreateBuffers(1, &memPool.draw.buffer);
				glNamedBufferData(memPool.draw.buffer, m_drawBufferInitialSize*sizeof(DrawType), nullptr, GL_DYNAMIC_COPY);

				// Free memory
				memPool.data.freeMemory.insert(AllocationIdentifier{
					.poolIndex = static_cast<uint16_t>(i),
					.offset = 0,
					.size = m_dataBufferInitialSize,
					.type = ResourceType::Data
				});
				memPool.draw.freeMemory.insert(AllocationIdentifier{
					.poolIndex = static_cast<uint16_t>(i),
					.offset = 0,
					.size = m_drawBufferInitialSize,
					.type = ResourceType::Draw
				});
				m_freeVertexMemory.insert(AllocationIdentifier{
					.poolIndex = static_cast<uint16_t>(i),
					.offset = 0,
					.size = m_vertexBufferInitialSize,
					.type = ResourceType::Vertex
				});

				// Initialize metadata
				memPool.index = i;
				memPool.storedDrawCommands = 0;

				memPool.vertex.size = m_vertexBufferInitialSize;
				memPool.vertex.freeSize = m_vertexBufferInitialSize;

				memPool.data.size = m_dataBufferInitialSize;
				memPool.data.freeSize = m_dataBufferInitialSize;

				memPool.draw.size = m_drawBufferInitialSize;
				memPool.draw.freeSize = m_drawBufferInitialSize;

				// Setup attribute bindings
				auto vao = memPool.vao;
				auto vertexBuffer = memPool.vertex.buffer;
				auto dataBuffer = memPool.data.buffer;

				// Bind VBO to binding index
				glVertexArrayVertexBuffer(vao, 0, vertexBuffer, 0, sizeof(VertexType));
				glVertexArrayVertexBuffer(vao, 1, dataBuffer, 0, sizeof(DataType));
				// Vertex attributes
				using VertexTypeInfo = vxg::reflection::ClassInfo<VertexType>;
				VertexTypeInfo::for_each_member([&](auto member) {
					using Member = decltype(member);
					glEnableVertexArrayAttrib(vao, Member::offset);	// Enable attribute index
					glVertexArrayAttribFormat(vao, Member::offset, vxg::utilities::num_components<typename Member::Type>(), GL_FLOAT, GL_FALSE, Member::offset),	// Bind attribute to attribute index
					glVertexArrayAttribBinding(vao, Member::offset, 0);	// Bind attribute index to binding index
				});
				// Object attributes
				for (int j=0; j<4; j++) {
					glEnableVertexArrayAttrib(vao, 1+j);	// Enable attribute index
					glVertexArrayAttribFormat(vao, 1+j, 4, GL_FLOAT, GL_FALSE, offsetof(DataType, modelMatrix) + sizeof(glm::vec4)*j);	// Bind attribute to attribute index
					glVertexArrayAttribBinding(vao, 1+j, 1);	// Bind attribute index to binding index
				}
				glVertexArrayBindingDivisor(vao, 1, 1);	// Set binding index's divisor
			}
		}

		void terminate_impl() noexcept {
			for (auto& memPool : m_memoryPools) {
				glDeleteBuffers(1, &memPool.vertex.buffer);
				glDeleteBuffers(1, &memPool.data.buffer);
				glDeleteBuffers(1, &memPool.draw.buffer);
				glDeleteVertexArrays(1, &memPool.vao);
			}
		}

		[[nodiscard]]
		std::vector<ExposedDrawResourcesType> get_draw_resources_impl() const noexcept {
			std::vector<ExposedDrawResourcesType> drawResources;
			for (auto& memPool : m_memoryPools)
				drawResources.push_back(ExposedDrawResourcesType{
					.vao = memPool.vao,
					.drawCommandBuffer = memPool.draw.buffer,
					.drawCommandCount = memPool.storedDrawCommands
				});
			return drawResources;
		}

	public:
		OpenGLAllocator() = delete;

		OpenGLAllocator(const uint16_t numMemoryPools, const size_t vertexBufferInitialSize, const size_t dataBufferInitialSize, const size_t drawBufferInitialSize)
			: Base(), m_numMemoryPools(numMemoryPools), m_vertexBufferInitialSize(vertexBufferInitialSize), m_dataBufferInitialSize(dataBufferInitialSize), m_drawBufferInitialSize(drawBufferInitialSize) {}
	
		// Not copyable
		OpenGLAllocator(const OpenGLAllocator& oa) = delete;
		OpenGLAllocator& operator=(const OpenGLAllocator& oa) = delete;

		// Move constructable only
		OpenGLAllocator(OpenGLAllocator&& oa) noexcept
			: Base(std::move(oa)), m_numMemoryPools(oa.m_numMemoryPools), m_vertexBufferInitialSize(oa.m_vertexBufferInitialSize), m_dataBufferInitialSize(oa.m_dataBufferInitialSize), m_drawBufferInitialSize(oa.m_drawBufferInitialSize)
		{
			m_memoryPools = std::move(oa.m_memoryPools);
			m_freeVertexMemory = std::move(oa.m_freeVertexMemory);
		}
		OpenGLAllocator& operator=(OpenGLAllocator&& oa) = delete;

		~OpenGLAllocator() noexcept = default;
	};

}; // namespace vxg::core::memory