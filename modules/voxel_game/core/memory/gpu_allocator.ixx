module;

#include <type_traits>
#include <vector>
#include <utility>

export module voxel_game.core.memory:gpu_allocator;

export namespace vxg::core::memory {

	template <typename T>
	struct GpuAllocatorTraits;

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
		
		struct ObjectAllocationIdentifier {
			AllocationIdentifier vertices;
			AllocationIdentifier data;
			AllocationIdentifier draw;
		};

		// Copy constructor
		GpuAllocator(const GpuAllocator& ga) = delete;

		// Copy assignment
		GpuAllocator& operator=(const GpuAllocator& ga) = delete;

		// Move constructor
		GpuAllocator(GpuAllocator&& ga) noexcept {
			m_initialized = ga.m_initialized;
			m_terminated = ga.m_terminated;
			ga.m_terminated = true;
		}

		// Move assignment
		GpuAllocator& operator=(GpuAllocator&& ga) = delete;

		void terminate() noexcept {
			derived_instance()->terminate_impl();
			m_terminated = true;
		}

		virtual ~GpuAllocator() noexcept {
			if (!m_terminated)
				terminate();
		}

		void initialize()
			noexcept(noexcept(derived_instance()->initialize_impl()))
		{
			derived_instance()->initialize_impl();
			m_initialized = true;
		}

		[[nodiscard]]
		AllocationIdentifier allocate_vertices(size_t size)
			noexcept(noexcept(derived_instance()->allocate_vertices_impl(size)))
		{
			return derived_instance()->allocate_vertices_impl(size);
		}

		[[nodiscard]]
		AllocationIdentifier allocate_data(AllocationIdentifier verticesLocation)
			noexcept(noexcept(derived_instance()->allocate_data_impl(verticesLocation)))
		{
			return derived_instance()->allocate_data_impl(verticesLocation);
		}

		[[nodiscard]]
		AllocationIdentifier allocate_draw(AllocationIdentifier verticesLocation, AllocationIdentifier dataLocation)
			noexcept(noexcept(derived_instance()->allocate_draw_impl(verticesLocation, dataLocation)))
		{
			return derived_instance()->allocate_draw_impl(verticesLocation, dataLocation);
		}

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

		void deallocate(AllocationIdentifier alloc)
			noexcept(noexcept(derived_instance()->deallocate_impl(alloc)))
		{
			derived_instance()->deallocate_impl(alloc);
		}

		void deallocate(ObjectAllocationIdentifier alloc)
			noexcept(noexcept(deallocate(std::declval<AllocationIdentifier>())))
		{
			deallocate(alloc.vertices);
			deallocate(alloc.data);
			deallocate(alloc.draw);
		}

		void construct_vertices(AllocationIdentifier alloc, const std::vector<VertexType>& value)
			noexcept(noexcept(derived_instance()->construct_vertices_impl(alloc, value)))
		{
			derived_instance()->construct_vertices_impl(alloc, value);
		}

		void construct_data(AllocationIdentifier alloc, const DataType& value)
			noexcept(noexcept(derived_instance()->construct_data_impl(alloc, value)))
		{
			derived_instance()->construct_data_impl(alloc, value);
		}

		void construct_draw(AllocationIdentifier drawAlloc, AllocationIdentifier vertexAlloc, AllocationIdentifier dataAlloc)
			noexcept(noexcept(derived_instance()->construct_draw_impl(drawAlloc, vertexAlloc, dataAlloc)))
		{
			derived_instance()->construct_draw_impl(drawAlloc, vertexAlloc, dataAlloc);
		}

		void construct_object(ObjectAllocationIdentifier alloc, const std::vector<VertexType>& vertices, const DataType& data)
			noexcept(noexcept(construct_vertices(alloc.vertices, vertices)) &&
				noexcept(construct_data(alloc.data, data)) &&
				noexcept(construct_draw(alloc.draw, alloc.vertices, alloc.data)))
		{
			construct_vertices(alloc.vertices, vertices);
			construct_data(alloc.data, data);
			construct_draw(alloc.draw, alloc.vertices, alloc.data);
		}

		void destroy(AllocationIdentifier alloc)
			noexcept(noexcept(derived_instance()->destroy_impl(alloc)))
		{
			derived_instance()->destroy_impl(alloc);
		}

		void destroy(ObjectAllocationIdentifier alloc)
			noexcept(noexcept(destroy(std::declval<AllocationIdentifier>())))
		{
			destroy(alloc.vertices);
			destroy(alloc.data);
			destroy(alloc.draw);
		}

		std::vector<ExposedDrawResourcesType> get_draw_resources() const
			noexcept(noexcept(derived_instance()->get_draw_resources_impl()))
		{
			return derived_instance()->get_draw_resources_impl();
		}
	};

	template <typename T>
	concept gpu_allocator_implementation = std::is_base_of_v<GpuAllocator<T>, T>;

}; // namespace vxg::core::memory