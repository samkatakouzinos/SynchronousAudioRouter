// SynchronousAudioRouter
// Copyright (C) 2015 Mackenzie Straight
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SynchronousAudioRouter.  If not, see <http://www.gnu.org/licenses/>.

#include "stdafx.h"
#include "tinyasio.h"
#include "utility.h"

namespace Sar {

std::vector<AsioDriver> InstalledAsioDrivers()
{
    std::vector<AsioDriver> result;
    HKEY asio;
    DWORD index = 0, nameSize = 256, valueSize = 256;
    LONG err;
    TCHAR name[256], value[256];

    LOG(INFO) << "Querying installed ASIO drivers.";

    if (!SUCCEEDED(err = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\ASIO"), 0, KEY_READ, &asio))) {

        LOG(INFO) << "Failed to open HKLM\\SOFTWARE\\ASIO: status " << err;
        return result;
    }

    while ((err = RegEnumKeyEx(
        asio, index++, name, &nameSize, nullptr, nullptr, nullptr, nullptr)) ==
        ERROR_SUCCESS) {

        AsioDriver driver;

        nameSize = 256;
        valueSize = 256;

        if ((err = RegGetValue(
            asio, name, TEXT("CLSID"), RRF_RT_REG_SZ,
            nullptr, value, &valueSize)) != ERROR_SUCCESS) {

            LOG(INFO) << "Skipping key "
                << TCHARToUTF8(name) << ": Couldn't get CLSID, error "
                << err;
            continue;
        }

        driver.clsid = TCHARToUTF8(value);
        valueSize = 256;

        if (RegGetValue(
            asio, name, TEXT("Description"), RRF_RT_REG_SZ,
            nullptr, value, &valueSize) != ERROR_SUCCESS) {

            // Workaround for drivers with incomplete ASIO registration.
            // Observed with M-Audio drivers: the main (64bit) registration is
            // fine but the Wow6432Node version is missing the description.
            LOG(INFO) << "Unable to get ASIO driver description, using key name instead.";
            driver.name = TCHARToUTF8(name);
        } else {
            driver.name = TCHARToUTF8(value);
        }

        LOG(INFO) << "Found ASIO driver: " << driver.name
            << " with CLSID " << driver.clsid;
        result.emplace_back(driver);
    }

    LOG(INFO) << "Done querying ASIO drivers. Status: " << err;

    RegCloseKey(asio);
    return result;
}

HRESULT AsioDriver::open(IASIO **ppAsio)
{
    HRESULT status;
    auto wstr = UTF8ToWide(clsid);
    GUID clsid;

    status = CLSIDFromString(wstr.c_str(), &clsid);

    if (!SUCCEEDED(status)) {
        return status;
    }

    return CoCreateInstance(
        clsid, nullptr, CLSCTX_INPROC_SERVER, clsid, (LPVOID *)ppAsio);
}

} // namespace Sar
