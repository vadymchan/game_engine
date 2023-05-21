#pragma once
#include <Windows.h>

namespace game_engine {
	namespace renderer {

		template <typename T>
		class RenderAPI {
		public:
			void Initialize(HWND hwnd) { static_cast<T*>(this)->InitializeImpl(hwnd); }
			void DrawTriangle() { static_cast<T*>(this)->DrawTriangleImpl(); }
			void Present() { static_cast<T*>(this)->PresentImpl(); }
		};




	} // namespace renderer
} // namespace game_engine

