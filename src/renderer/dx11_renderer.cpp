//// #pragma once
//
//// #include "render_api.h"
//// #include <d3d11.h>
//// #include <wrl/client.h>
//
//// namespace game_engine {
//// namespace renderer {
//
//// class DX11RenderAPI : public RenderAPI<DX11RenderAPI> {
//// public:
////     void Initialize();
////     void DrawTriangle();
////     void Present();
//
//// private:
////     Microsoft::WRL::ComPtr<ID3D11Device> device_;
////     Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
////     Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
////     Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view_;
//// };
//
//// } // namespace renderer
//// } // namespace game_engine
//
//
//#pragma once
//
//#include "render_api.h"
//#include <d3d11.h>
//#include <wrl/client.h>
//#include <vector>
//
//namespace game_engine {
//namespace renderer {
//
//class DX11RenderAPI : public RenderAPI<DX11RenderAPI> {
//public:
//    //utils::ErrorCode InitializeImpl(HWND hwnd);
//    void InitializeImpl(HWND hwnd);
//    void DrawTriangleImpl();
//    void PresentImpl();
//
//private:
//    Microsoft::WRL::ComPtr<ID3D11Device> device_;
//    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
//    Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
//    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view_;
//    Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_;
//    Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer_;
//    Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout_;
//    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
//    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;
//    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state_;
//
//    void CreateDeviceAndContext();
//    void CreateSwapChain(HWND hwnd);
//    void CreateRenderTargetView();
//    void CreateVertexBuffer();
//    void CreateIndexBuffer();
//    void CreateInputLayout();
//    void CreateShaders();
//    void CreateRasterizerState();
//    void SetViewport();
//};
//
//} // namespace renderer
//} // namespace game_engine
