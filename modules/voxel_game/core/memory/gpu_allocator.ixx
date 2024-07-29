module;

#include <type_traits>
#include <vector>
#include <utility>

export module voxel_game.core.memory:gpu_allocator;

export namespace vxg::core::memory {

	/**
	 * @brief Traits interface for a GpuAllocator implementation.
	 * @tparam T GpuAllocator implementation.
	 * 
	 * A GpuAllocator implementation should specialize this struct to provide necessary type metadata.
	 * 
	 * The following types are required:
	 * - VertexType: Type used to represent per vertex data.
	 * - DataType: Type used to represent per object data.
	 * - DrawType: Type used to represent indirect draw commands.
	 * - AllocationIdentifier: An opaque type representing an allocation block.
	 * - ExposedDrawResourcesType: The type returned by get_draw_resources exposing resources required by the API specific backend for drawing.
	 * 
	 * @see GpuAllocatorTraits<OpenGLAllocator>
	 */
	template <typename T>
	struct GpuAllocatorTraits;

	/**
	 * @brief CRTP interface for a GPU memory allocator.
	 * @tparam Derived Implementation of the allocator.
	 * 
	 * A GpuAllocator is responsible for managing VRAM allocations, object construction, destruction, and deallocation.
	 * 
	 * An implementation should provide the following methods (noexcept is optional unless specified):
	 * - void initialize_impl(): @copybrief GpuAllocator::initialize()
	 * - void terminate_impl() noexcept: @copybrief GpuAllocator::terminate()
	 * - AllocationIdentifier allocate_vertices_impl(size_t size): @copybrief GpuAllocator::allocate_vertices()
	 * - AllocationIdentifier allocate_data_impl(AllocationIdentifier verticesLocation): @copybrief GpuAllocator::allocate_data()
	 * - AllocationIdentifier allocate_draw_impl(AllocationIdentifier verticesLocation, AllocationIdentifier dataLocation): @copybrief GpuAllocator::allocate_draw()
	 * - void deallocate_impl(AllocationIdentifier alloc): @copybrief GpuAllocator::deallocate(AllocationIdentifier)
	 * - void construct_vertices_impl(AllocationIdentifier alloc, const std::vector<VertexType>& value): @copybrief GpuAllocator::construct_vertices()
	 * - void construct_data_impl(AllocationIdentifier alloc, const DataType& value): @copybrief GpuAllocator::construct_data()
	 * - void construct_draw_impl(AllocationIdentifier drawAlloc, AllocationIdentifier vertexAlloc, AllocationIdentifier dataAlloc): @copybrief GpuAllocator::construct_draw()
	 * - void destroy_impl(AllocationIdentifier alloc): @copybrief GpuAllocator::destroy(AllocationIdentifier)
	 * - std::vector<ExposedDrawResourcesType> get_draw_resources_impl() const: @copybrief GpuAllocator::get_draw_resources()
	 * 
	 * An implementation should also specialize GpuAllocatorTraits to provide necessary type metadata.
	 * 
	 * @see OpenGLAllocator
	 */
	template <typename Derived>
	class GpuAllocator {
		GpuAllocator() = default;
		friend Derived;

		const Derived* derived_instance() const noexcept {
			return static_cast<const Derived*>(this);
		}

		Derived* derived_instance() noexcept {
			return static_cast<Derived*>(this);
		}

		bool m_terminated = false;
		bool m_initialized = false;

	public:
		using VertexType = typename GpuAllocatorTraits<Derived>::VertexType;
		using DataType = typename GpuAllocatorTraits<Derived>::DataType;
		using DrawType = typename GpuAllocatorTraits<Derived>::DrawType;
		using AllocationIdentifier = typename GpuAllocatorTraits<Derived>::AllocationIdentifier;
		using ExposedDrawResourcesType = typename GpuAllocatorTraits<Derived>::ExposedDrawResourcesType;
		
		/**
		 * @brief Aggregate type wrapping all allocation identifiers needed to represent an object.
		 */
		struct ObjectAllocationIdentifier {
			AllocationIdentifier vertices;
			AllocationIdentifier data;
			AllocationIdentifier draw;
		};

		// Not copyable
		GpuAllocator(const GpuAllocator& ga) = delete;
		GpuAllocator& operator=(const GpuAllocator& ga) = delete;

		// Move constructible only
		GpuAllocator(GpuAllocator&& ga) noexcept {
			m_initialized = ga.m_initialized;
			m_terminated = ga.m_terminated;
			ga.m_terminated = true;
		}
		GpuAllocator& operator=(GpuAllocator&& ga) = delete;

		/**
		 * @brief Destroy the allocator.
		 * 
		 * @note If not called explicitly, terminate will be called at the end of the object's lifetime.
		 */
		void terminate() noexcept {
			derived_instance()->terminate_impl();
			m_terminated = true;
		}

		virtual ~GpuAllocator() noexcept {
			if (!m_terminated)
				terminate();
		}

		/**
		 * @brief Initialize the allocator.
		 * 
		 * @note initialize should be called explicitly before any other method.
		 */
		void initialize()
			noexcept(noexcept(derived_instance()->initialize_impl()))
		{
			derived_instance()->initialize_impl();
			m_initialized = true;
		}

		/**
		 * @brief Allocate a block of memory for n-vertices' per vertex data.
		 * @param size Number of vertices to allocate memory for.
		 * @return Identifier of the allocated block.
		 */
		[[nodiscard]]
		AllocationIdentifier allocate_vertices(size_t size)
			noexcept(noexcept(derived_instance()->allocate_vertices_impl(size)))
		{
			return derived_instance()->allocate_vertices_impl(size);
		}

		/**
		 * @brief Allocate a block of memory for per object data.
		 * @param verticesLocation Identifier of the associated vertices allocation block.
		 * @return Identifier of the allocated block.
		 */
		[[nodiscard]]
		AllocationIdentifier allocate_data(AllocationIdentifier verticesLocation)
			noexcept(noexcept(derived_instance()->allocate_data_impl(verticesLocation)))
		{
			return derived_instance()->allocate_data_impl(verticesLocation);
		}

		/**
		 * @brief Allocate a block of memory for an indirect draw command.
		 * @param verticesLocation Identifier of the associated vertices allocation block.
		 * @param dataLocation Identifier of the associated data allocation block.
		 * @return Identifier of the allocated block.
		 */
		[[nodiscard]]
		AllocationIdentifier allocate_draw(AllocationIdentifier verticesLocation, AllocationIdentifier dataLocation)
			noexcept(noexcept(derived_instance()->allocate_draw_impl(verticesLocation, dataLocation)))
		{
			return derived_instance()->allocate_draw_impl(verticesLocation, dataLocation);
		}

		/**
		 * @brief Allocate memory for an entire object.
		 * @param numVertices Number of vertices which make up the object.
		 * @return ObjectAllocationIdentifier wrapping all allocation identifiers needed to represent the object.
		 */
		[[nodiscard]]
		ObjectAllocationIdentifier allocate_object(size_t numVertices)
			noexcept(std::is_nothrow_invocable_v<decltype(&GpuAllocator::allocate_vertices), GpuAllocator, size_t> &&
				std::is_nothrow_invocable_v<decltype(&GpuAllocator::allocate_data), GpuAllocator, AllocationIdentifier> &&
				std::is_nothrow_invocable_v<decltype(&GpuAllocator::allocate_draw), GpuAllocator, AllocationIdentifier, AllocationIdentifier> &&
				std::is_nothrow_constructible_v<ObjectAllocationIdentifier, AllocationIdentifier, AllocationIdentifier, AllocationIdentifier>)
		{
			auto vertices = allocate_vertices(numVertices);
			auto data = allocate_data(vertices);
			auto draw = allocate_draw(vertices, data);
			return ObjectAllocationIdentifier{ vertices, data, draw };
		}

		/**
		 * @brief Deallocate a block of memory.
		 * @param alloc Identifier of the block to deallocate.
		 */
		void deallocate(AllocationIdentifier alloc)
			noexcept(noexcept(derived_instance()->deallocate_impl(alloc)))
		{
			derived_instance()->deallocate_impl(alloc);
		}

		/**
		 * @brief Deallocate resources associated with an object.
		 * @param alloc Identifier of the object to deallocate.
		 */
		void deallocate(ObjectAllocationIdentifier alloc)
			noexcept(noexcept(deallocate(std::declval<AllocationIdentifier>())))
		{
			deallocate(alloc.vertices);
			deallocate(alloc.data);
			deallocate(alloc.draw);
		}

		/**
		 * @brief Construct per vertex data in an allocated block.
		 * @param alloc Identifier of the block in which to construct the data.
		 * @param value Data with which to construct the vertices.
		 */
		void construct_vertices(AllocationIdentifier alloc, const std::vector<VertexType>& value)
			noexcept(noexcept(derived_instance()->construct_vertices_impl(alloc, value)))
		{
			derived_instance()->construct_vertices_impl(alloc, value);
		}

		/**
		 * @brief Construct per object data in an allocated block.
		 * @param alloc Identifier of the block in which to construct the data.
		 * @param value Data with which to construct the per object data.
		 */
		void construct_data(AllocationIdentifier alloc, const DataType& value)
			noexcept(noexcept(derived_instance()->construct_data_impl(alloc, value)))
		{
			derived_instance()->construct_data_impl(alloc, value);
		}

		/**
		 * @brief Construct an indirect draw command in an allocated block.
		 * @param drawAlloc Identifier of the block in which to construct the draw command.
		 * @param vertexAlloc Identifier of the associated vertices allocation block.
		 * @param dataAlloc Identifier of the associated data allocation block.
		 */
		void construct_draw(AllocationIdentifier drawAlloc, AllocationIdentifier vertexAlloc, AllocationIdentifier dataAlloc)
			noexcept(noexcept(derived_instance()->construct_draw_impl(drawAlloc, vertexAlloc, dataAlloc)))
		{
			derived_instance()->construct_draw_impl(drawAlloc, vertexAlloc, dataAlloc);
		}

		/**
		 * @brief Construct an entire object in an object allocation block.
		 * @param alloc ObjectAllocationIdentifier in which to construct the object.
		 * @param vertices Data with which to construct the vertices.
		 * @param data Data with which to construct the per object data.
		 */
		void construct_object(ObjectAllocationIdentifier alloc, const std::vector<VertexType>& vertices, const DataType& data)
			noexcept(noexcept(construct_vertices(alloc.vertices, vertices)) &&
				noexcept(construct_data(alloc.data, data)) &&
				noexcept(construct_draw(alloc.draw, alloc.vertices, alloc.data)))
		{
			construct_vertices(alloc.vertices, vertices);
			construct_data(alloc.data, data);
			construct_draw(alloc.draw, alloc.vertices, alloc.data);
		}

		/**
		 * @brief Destroy the object in an allocated block.
		 * @param alloc Identifier of the block containing the object to destroy.
		 */
		void destroy(AllocationIdentifier alloc)
			noexcept(noexcept(derived_instance()->destroy_impl(alloc)))
		{
			derived_instance()->destroy_impl(alloc);
		}

		/**
		 * @brief Destroy an entire object.
		 * @param alloc Identifier of the object to destroy.
		 */
		void destroy(ObjectAllocationIdentifier alloc)
			noexcept(noexcept(destroy(std::declval<AllocationIdentifier>())))
		{
			destroy(alloc.vertices);
			destroy(alloc.data);
			destroy(alloc.draw);
		}

		/**
		 * @brief Get resources required by the API specific backend for drawing.
		 * @return Vector of required resources, each element of which representing a draw.
		 */
		[[nodiscard]]
		std::vector<ExposedDrawResourcesType> get_draw_resources() const
			noexcept(noexcept(derived_instance()->get_draw_resources_impl()))
		{
			return derived_instance()->get_draw_resources_impl();
		}
	};

	/**
	 * @brief Concept is satisfied if T implements GpuAllocator.
	 * @tparam T Type to check.
	 */
	template <typename T>
	concept gpu_allocator_implementation = std::is_base_of_v<GpuAllocator<T>, T>;

}; // namespace vxg::core::memory