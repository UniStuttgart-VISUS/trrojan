// <copyright file="device.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <memory>

#include <Windows.h>
#include <atlbase.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#include "trrojan/device.h"

#include "trrojan/d3d12/export.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Base class for devices to be tested.
    /// </summary>
    class TRROJAND3D12_API device : public trrojan::device_base {

    public:

        typedef std::shared_ptr<device> pointer;

        /// <summary>
        /// Create a command allocator for the given device.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        /// <exception cref="std::invalid_argument">If
        /// <paramref name="device" /> is <c>nullptr</c>.</exception>
        /// <exception cref="ATL::CAtlException">If the object could not
        /// be created.</exception>
        static ATL::CComPtr<ID3D12CommandAllocator> create_command_allocator(
            ID3D12Device *device, const D3D12_COMMAND_LIST_TYPE type);

        /// <summary>
        /// Allocate a command queue for the given device.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        /// <paramref name="device" /> is <c>nullptr</c>.</exception>
        /// <exception cref="ATL::CAtlException">If the object could not
        /// be created.</exception>
        static ATL::CComPtr<ID3D12CommandQueue> create_command_queue(
            ID3D12Device *device, const D3D12_COMMAND_LIST_TYPE type
            = D3D12_COMMAND_LIST_TYPE_DIRECT);

        /// <summary>
        /// Initialises a new instance representing the given D3D device.
        /// </summary>
        /// <param name="d3dDevice">The Direct3D device to be represented by
        /// this instance. This must not be <c>nullptr</c>.</param>
        /// <param name="dxgiFactory">The DXGI factory used to obtain the
        /// underlying adapter. This is required due to the changes described
        /// on https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-1-4-improvements
        /// </param>
        device(const ATL::CComPtr<ID3D12Device>& d3dDevice,
            const ATL::CComPtr<IDXGIFactory4>& dxgiFactory);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~device(void);

        /// <summary>
        /// Answer the direct command queue of the device.
        /// </summary>
        /// <returns></returns>
        inline ATL::CComPtr<ID3D12CommandQueue> command_queue(void) {
            return this->_command_queue;
        }

        /// <summary>
        /// Gets the command allocator from which the device creates command
        /// lists for computing.
        /// </summary>
        /// <returns>The command allocator for the device.</returns>
        inline ATL::CComPtr<ID3D12CommandAllocator> compute_command_allocator(
                void) {
            return this->_compute_command_allocator;
        }

        /// <summary>
        /// Gets the command allocator from which the device creates command
        /// lists for copying.
        /// </summary>
        /// <returns>The command allocator for the device.</returns>
        inline ATL::CComPtr<ID3D12CommandAllocator> copy_command_allocator(
                void) {
            return this->_copy_command_allocator;
        }

        /// <summary>
        /// Allocate a command list of the given type on the device.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="inital_state"></param>
        /// <returns></returns>
        ATL::CComPtr<ID3D12CommandList> create_command_list(
            const D3D12_COMMAND_LIST_TYPE type,
            ID3D12PipelineState *initial_state = nullptr);

        /// <summary>
        /// Allocate a command list and try to cast it to a
        /// <see cref="ID3D12GraphicsCommandList" />.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="initial_state"></param>
        /// <returns></returns>
        ATL::CComPtr<ID3D12GraphicsCommandList> create_graphics_command_list(
            const D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            ID3D12PipelineState *initial_state = nullptr);

        /// <summary>
        /// Answer the underlying Direct3D device.
        /// </summary>
        inline ATL::CComPtr<ID3D12Device>& d3d_device(void) {
            return this->_d3d_device;
        }

        /// <summary>
        /// Gets the command allocator from which the device creates command
        /// lists for command buffers that the GPU can execute directly.
        /// </summary>
        /// <returns>The command allocator for the device.</returns>
        inline ATL::CComPtr<ID3D12CommandAllocator> direct_command_allocator(
            void) {
            return this->_direct_command_allocator;
        }

        /// <summary>
        /// Answer the underlying DXGI adapter on which the device was created.
        /// </summary>
        /// <returns></returns>
        ATL::CComPtr<IDXGIAdapter> dxgi_adapter(void);

        /// <summary>
        /// Answer the DXGI factory that was used to create the device.
        /// </summary>
        /// <returns></returns>
        inline ATL::CComPtr<IDXGIFactory4>& dxgi_factory(void) {
            return this->_dxgi_factory;
        }

        /// <summary>
        /// Enables or disables the stable power state on the device.
        /// </summary>
        /// <remarks>
        /// For reliable measurements, it is recommended to put all devices
        /// into a stable power state. This can be achieved by creating a
        /// <see cref="trrojan::power_state_scope" /> or by setting it
        /// individually per device using this method.
        /// </remarks>
        /// <param name="enabled">Determines whether the stable power state is
        /// enabled or disabled.</param>
        void set_stable_power_state(const bool enabled);

    private:

        ATL::CComPtr<ID3D12CommandQueue> _command_queue;
        ATL::CComPtr<ID3D12CommandAllocator> _compute_command_allocator;
        ATL::CComPtr<ID3D12CommandAllocator> _copy_command_allocator;
        ATL::CComPtr<ID3D12Device> _d3d_device;
        ATL::CComPtr<ID3D12CommandAllocator> _direct_command_allocator;
        ATL::CComPtr<IDXGIFactory4> _dxgi_factory;

    };
}
}
