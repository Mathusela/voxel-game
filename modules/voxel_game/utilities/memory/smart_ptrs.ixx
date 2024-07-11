export module voxel_game.utilities.memory:smart_ptrs;

import voxel_game.exceptions;

export namespace vxg::utilities::memory {
	
	template <typename T>
	class NonOwningPtr {
		T* m_ptr;

		constexpr void check_null() const {
			if (m_ptr == nullptr)
				throw vxg::exceptions::MemorySafetyError("Attempted to dereference a null pointer.");
		}

		constexpr T& safe_deref() const {
			check_null();
			return *m_ptr;
		}

	public:
		using ElementType = T;
		using Pointer = T*;

		constexpr NonOwningPtr(T* ptr = nullptr) noexcept
			: m_ptr(ptr) {}

		[[nodiscard]]
		constexpr T* get() const noexcept {
			return m_ptr;
		}

		constexpr void reset(T* ptr = nullptr) noexcept {
			m_ptr = ptr;
		}

		[[nodiscard]]
		constexpr explicit operator bool() const noexcept {
			return m_ptr != nullptr;
		}

		[[nodiscard]]
		constexpr T& operator*() const {
			return safe_deref();
		}
		[[nodiscard]]
		constexpr T* operator->() const {
			check_null();
			return m_ptr;
		}

		[[nodiscard]]
		friend constexpr bool operator<=>(const NonOwningPtr& lhs, const NonOwningPtr& rhs) noexcept {
			return lhs.m_ptr <=> rhs.m_ptr;
		}
	};

}; // namespace vxg::utilities