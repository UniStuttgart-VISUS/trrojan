// <copyright file="device.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <atomic>
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
        /// Close the given command list and execute it immediately.
        /// </summary>
        void close_and_execute_command_list(
            ID3D12GraphicsCommandList *cmd_list);

        /// <summary>
        /// Answer the direct command queue of the device.
        /// </summary>
        /// <returns></returns>
        inline ATL::CComPtr<ID3D12CommandQueue> command_queue(void) {
            return this->_command_queue;
        }

        /// <summary>
        /// Answer the underlying Direct3D device.
        /// </summary>
        inline ATL::CComPtr<ID3D12Device>& d3d_device(void) {
            return this->_d3d_device;
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
        /// Execute the given command list on the direct command queue of the
        ///  device.
        /// </summary>
        /// <param name="cmd_list"></param>
        void execute_command_list(ID3D12CommandList *cmd_list);

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

        /// <summary>
        /// Inject a signal into the command queue and wait for the GPU to reach
        /// it, ie completes all processing until the signal.
        /// </summary>
        /// <remarks>
        /// <para>This method uses a fence allocated in the device. The value of
        /// the fence is atomically incremented, wherefore it is safe to call
        /// this method from different threads. A new event is allocated for
        /// each call to block the calling thread until the fence becomes
        /// signalled.</para>
        /// </remarks>
        void wait_for_gpu(void);

    private:

        ATL::CComPtr<ID3D12CommandQueue> _command_queue;
        ATL::CComPtr<ID3D12Device> _d3d_device;
        ATL::CComPtr<IDXGIFactory4> _dxgi_factory;
        ATL::CComPtr<ID3D12Fence> _fence;
        std::atomic<UINT64> _next_fence;

    };
}
}
