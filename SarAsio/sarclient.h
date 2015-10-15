// SynchronousAudioRouter
// Copyright (C) 2015 Mackenzie Straight
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SynchronousAudioRouter.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _SAR_ASIO_SARCLIENT_H
#define _SAR_ASIO_SARCLIENT_H

#include "config.h"
#include "sar.h"

namespace Sar {

struct BufferConfig
{
    int frameSampleCount;
    int sampleRate;
    int sampleSize;
    std::vector<std::vector<void *>> asioBuffers;
};

struct SarClient
{
    SarClient(
        const DriverConfig& driverConfig,
        const BufferConfig& bufferConfig);
    void tick(long bufferIndex);
    bool start();
    void stop();

private:
    struct NotificationHandle
    {
        NotificationHandle(): generation(0), handle(nullptr) {}
        ~NotificationHandle()
        {
            if (handle) {
                CloseHandle(handle);
            }
        }

        ULONG generation;
        HANDLE handle;
    };

    struct HandleQueueCompletion: OVERLAPPED
    {
        SarHandleQueueResponse responses[32];
    };

    bool openControlDevice();
    bool setBufferLayout();
    bool createEndpoints();
    bool enableRegistryFilter();
    void updateNotificationHandles();
    void processNotificationHandleUpdates(int updateCount);

    void demux(
        void *muxBufferFirst, size_t firstSize,
        void *muxBufferSecond, size_t secondSize,
        void **targetBuffers, int ntargets,
        size_t targetSize, int sampleSize);
    void mux(
        void *muxBufferFirst, size_t firstSize,
        void *muxBufferSecond, size_t secondSize,
        void **targetBuffers, int ntargets,
        size_t targetSize, int sampleSize);

    DriverConfig _driverConfig;
    BufferConfig _bufferConfig;
    std::vector<NotificationHandle> _notificationHandles;
    HANDLE _device;
    HANDLE _completionPort;
    void *_sharedBuffer;
    DWORD _sharedBufferSize;
    volatile SarEndpointRegisters *_registers;
    HandleQueueCompletion _handleQueueCompletion;
    bool _handleQueueStarted;
};

} // namespace Sar

#endif // _SAR_ASIO_SARCLIENT_H
