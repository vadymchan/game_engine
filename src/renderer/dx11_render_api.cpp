//#include "../../include/game_engine/renderer/dx11_render_api.h"
//#include <DirectXMath.h>
//#include <vector>
//#include <iostream>
//
//namespace game_engine {
//    namespace renderer {
//
//        using namespace DirectX;
//
//        struct Vertex {
//            XMFLOAT3 position;
//            XMFLOAT4 color;
//        };
//
//        // Shader source code
//        const char* vertexShaderSource = R"(
//    cbuffer ConstantBuffer : register(b0) {
//        float4x4 ModelViewProjection;
//    };
//
//    struct VertexInputType {
//        float3 position : POSITION;
//        float4 color : COLOR;
//    };
//
//    struct PixelInputType {
//        float4 position : SV_POSITION;
//        float4 color : COLOR;
//    };
//
//    PixelInputType main(VertexInputType input) {
//        PixelInputType output;
//        output.position = mul(float4(input.position, 1.0), ModelViewProjection);
//        output.color = input.color;
//        return output;
//    }
//)";
//
//        const char* pixelShaderSource = R"(
//    struct PixelInputType {
//        float4 position : SV_POSITION;
//        float4 color : COLOR;
//    };
//
//    float4 main(PixelInputType input) : SV_TARGET {
//        return input.color;
//    }
//)";
//
//        void DX11RenderAPI::InitializeImpl(HWND hwnd) {
//            CreateDeviceAndContext();
//            CreateSwapChain(hwnd);
//            CreateRenderTargetView();
//            CreateVertexBuffer();
//            CreateIndexBuffer();
//            CreateInputLayout();
//            CreateShaders();
//            CreateRasterizerState();
//            SetViewport();
//
//            /*return utils::ErrorCode::Success;*/
//        }
//
//        void DX11RenderAPI::DrawTriangleImpl() {
//            // Clear the render target view
//            const float clearColor[] = { 0.2f, 0.3f, 0.3f, 1.f };
//            context_->ClearRenderTargetView(render_target_view_.Get(), clearColor);
//
//            // Bind the input layout, vertex buffer, and index buffer
//            context_->IASetInputLayout(input_layout_.Get());
//            UINT stride = sizeof(Vertex);
//            UINT offset = 0;
//            context_->IASetVertexBuffers(0, 1, vertex_buffer_.GetAddressOf(), &stride, &offset);
//            context_->IASetIndexBuffer(index_buffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
//
//            // Set the primitive topology
//            context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//            // Bind the shaders
//            context_->VSSetShader(vertex_shader_.Get(), nullptr, 0);
//            context_->PSSetShader(pixel_shader_.Get(), nullptr, 0);
//
//            // Set the rasterizer state
//            context_->RSSetState(rasterizer_state_.Get());
//
//            // Draw the triangle
//            context_->DrawIndexed(3, 0, 0);
//        }
//
//        void DX11RenderAPI::PresentImpl() {
//            swap_chain_->Present(1, 0);
//        }
//
//        void DX11RenderAPI::CreateDeviceAndContext() {
//            UINT createDeviceFlags = 0;
//#ifdef _DEBUG
//            createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif
//
//            D3D_FEATURE_LEVEL featureLevel;
//            const D3D_FEATURE_LEVEL featureLevelArray[1] = { D3D_FEATURE_LEVEL_11_0 };
//
//            HRESULT hr = D3D11CreateDevice(
//                nullptr,
//                D3D_DRIVER_TYPE_HARDWARE,
//                nullptr,
//                createDeviceFlags,
//                featureLevelArray,
//                1,
//                D3D11_SDK_VERSION,
//                &device_,
//                &featureLevel,
//                &context_);
//
//            if (FAILED(hr)) {
//                    std::cerr << "Failed to create DirectX 11 device and context." << std::endl;
//            }
//        }
//
//        void DX11RenderAPI::CreateSwapChain(HWND hwnd) {
//            DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
//            swapChainDesc.BufferCount = 1;
//            /*swapChainDesc.BufferDesc.Width = width_;
//            swapChainDesc.BufferDesc.Height = height_;*/
//            swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//            swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
//            swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
//            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//            swapChainDesc.OutputWindow = hwnd;
//            swapChainDesc.SampleDesc.Count = 1;
//            swapChainDesc.SampleDesc.Quality = 0;
//            swapChainDesc.Windowed = TRUE;
//            Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice = nullptr;
//            Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;
//            Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory = nullptr;
//
//            HRESULT hr = device_.As(&dxgiDevice);
//            if (FAILED(hr)) {
//                std::cerr << "Failed to get DXGI device from D3D11 device." << std::endl;
//            }
//
//            hr = dxgiDevice->GetAdapter(&dxgiAdapter);
//            if (FAILED(hr)) {
//                std::cerr << "Failed to get DXGI adapter." << std::endl;
//            }
//
//            hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
//            if (FAILED(hr)) {
//                std::cerr << "Failed to get DXGI factory." << std::endl;
//            }
//
//            hr = dxgiFactory->CreateSwapChain(device_.Get(), &swapChainDesc, &swap_chain_);
//            if (FAILED(hr)) {
//                std::cerr << "Failed to create DXGI swap chain." << std::endl;
//            }
//        }
//
//        void DX11RenderAPI::CreateRenderTargetView() {
//            Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
//            HRESULT hr = swap_chain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
//            if (FAILED(hr)) {
//                std::cerr << "Failed to get back buffer." << std::endl;
//            }
//
//            hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &render_target_view_);
//            if (FAILED(hr)) {
//                std::cerr << "Failed to create render target view." << std::endl;
//            }
//
//            context_->OMSetRenderTargets(1, render_target_view_.GetAddressOf(), nullptr);
//        }
//
//        void DX11RenderAPI::CreateVertexBuffer() {
//            Vertex vertices[] = {
//                {XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
//                {XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
//                {XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)}
//            };
//
//            D3D11_BUFFER_DESC vertexBufferDesc = {};
//            vertexBufferDesc.ByteWidth = sizeof(vertices);
//            vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
//            vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//            vertexBufferDesc.CPUAccessFlags = 0;
//            D3D11_SUBRESOURCE_DATA vertexData = {};
//            vertexData.pSysMem = vertices;
//
//            HRESULT hr = device_->CreateBuffer(&vertexBufferDesc, &vertexData, &vertex_buffer_);
//            if (FAILED(hr)) {
//                std::cerr << "Failed to create vertex buffer." << std::endl;
//            }
//        }
//
//        void DX11RenderAPI::CreateIndexBuffer() {
//            UINT indices[] = { 0, 1, 2 };
//            D3D11_BUFFER_DESC indexBufferDesc = {};
//            indexBufferDesc.ByteWidth = sizeof(indices);
//            indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
//            indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//            indexBufferDesc.CPUAccessFlags = 0;
//            D3D11_SUBRESOURCE_DATA indexData = {};
//            indexData.pSysMem = indices;
//
//            HRESULT hr = device_->CreateBuffer(&indexBufferDesc, &indexData, &index_buffer_);
//            if (FAILED(hr)) {
//                std::cerr << "Failed to create index buffer." << std::endl;
//            }
//        }
//
//        void DX11RenderAPI::CreateInputLayout() {
//            D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
//            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
//            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
//            };
//
//            HRESULT hr = device_->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc),
//                vertexShaderSource, strlen(vertexShaderSource), &input_layout_);
//            if (FAILED(hr)) {
//                std::cerr << "Failed to create input layout." << std::endl;
//            }
//        }
//
//        void DX11RenderAPI::CreateShaders() {
//            HRESULT hr = device_->CreateVertexShader(vertexShaderSource, strlen(vertexShaderSource), nullptr, &vertex_shader_);
//            if (FAILED(hr)) {
//                std::cerr << "Failed to create vertex shader." << std::endl;
//            }
//            hr = device_->CreatePixelShader(pixelShaderSource, strlen(pixelShaderSource), nullptr, &pixel_shader_);
//            if (FAILED(hr)) {
//                std::cerr << "Failed to create pixel shader." << std::endl;
//            }
//        }
//
//        void DX11RenderAPI::CreateRasterizerState() {
//            D3D11_RASTERIZER_DESC rasterizerDesc = {};
//            rasterizerDesc.FillMode = D3D11_FILL_SOLID;
//            rasterizerDesc.CullMode = D3D11_CULL_BACK;
//            rasterizerDesc.FrontCounterClockwise = FALSE;
//            rasterizerDesc.DepthBias = 0;
//            rasterizerDesc.DepthBiasClamp = 0.0f;
//            rasterizerDesc.SlopeScaledDepthBias = 0.0f;
//            rasterizerDesc.DepthClipEnable = TRUE;
//            rasterizerDesc.ScissorEnable = FALSE;
//            rasterizerDesc.MultisampleEnable = FALSE;
//            rasterizerDesc.AntialiasedLineEnable = FALSE;
//            HRESULT hr = device_->CreateRasterizerState(&rasterizerDesc, &rasterizer_state_);
//            if (FAILED(hr)) {
//                std::cerr << "Failed to create rasterizer state." << std::endl;
//            }
//        }
//
//        void DX11RenderAPI::SetViewport() {
//            D3D11_VIEWPORT viewport = {};
//            viewport.TopLeftX = 0.0f;
//            viewport.TopLeftY = 0.0f;
//            /*viewport.Width = static_cast<float>(width_);
//            viewport.Height = static_cast<float>(height_);*/
//            viewport.MinDepth = 0.0f;
//            viewport.MaxDepth = 1.0f;
//            context_->RSSetViewports(1, &viewport);
//
//        }
//
//} // namespace renderer
//} // namespace game_engine