#include <cstdio>
#include <windows.h>
#include "GoIO_DLL_interface.h"

constexpr int    kMaxSamples    = 100;
constexpr double kSamplePeriodS = 0.040;

void InitGoIO()     { GoIO_Init(); }
void ShutdownGoIO() { GoIO_Uninit(); }

bool FindGoMotion(char* deviceNameOut, gtype_int32 nameBufLen)
{
    // GoIO's discovery is always a two-step "ask count, then fetch by index"
    // pattern -- you'll see this same shape again if you ever add other
    // sensor types. This is GetAvailableDeviceName() from the sample,
    // narrowed to just Go!Motion's product id.
    int numFound = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID);
    if (numFound <= 0)
        return false;

    GoIO_GetNthAvailableDeviceName(deviceNameOut, nameBufLen,
                                   VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID, 0);
    return true;
}

GOIO_SENSOR_HANDLE OpenGoMotion(const char* deviceName)
{
    // TODO: look at the line right after GetAvailableDeviceName() succeeds in
    // GoIO_DeviceCheck.cpp -- it's a single call to GoIO_Sensor_Open(). Same
    // call here, swap in CYCLOPS_DEFAULT_PRODUCT_ID for the sample's productId.
    return nullptr;
}

void CloseGoMotion(GOIO_SENSOR_HANDLE hDevice)
{
    // TODO: GoIO_Sensor_Close(hDevice) -- one line, at the bottom of the sample's block.
}

void ConfigureAndStart(GOIO_SENSOR_HANDLE hDevice)
{
    // TODO: two calls back to back in the sample, right after Open succeeds:
    //   GoIO_Sensor_SetMeasurementPeriod(hDevice, kSamplePeriodS, SKIP_TIMEOUT_MS_DEFAULT)
    //   GoIO_Sensor_SendCmdAndGetResponse(hDevice, SKIP_CMD_ID_START_MEASUREMENTS,
    //                                     NULL, 0, NULL, NULL, SKIP_TIMEOUT_MS_DEFAULT)
}

int ReadDistances(GOIO_SENSOR_HANDLE hDevice, double* metersOut, int maxSamples)
{
    // TODO: this is the sample's rawMeasurements -> volts -> calbMeasurements
    // loop, just writing into metersOut instead of the sample's local arrays:
    //   gtype_int32 raw[kMaxSamples];
    //   int n = GoIO_Sensor_ReadRawMeasurements(hDevice, raw, maxSamples);
    //   for i in [0, n):
    //       double v = GoIO_Sensor_ConvertToVoltage(hDevice, raw[i]);
    //       metersOut[i] = GoIO_Sensor_CalibrateData(hDevice, v);
    //   return n;
    return 0;
}

int main()
{
    InitGoIO();

    char deviceName[GOIO_MAX_SIZE_DEVICE_NAME];
    if (!FindGoMotion(deviceName, GOIO_MAX_SIZE_DEVICE_NAME))
    {
        printf("No Go!Motion found.\n");
        ShutdownGoIO();
        return 1;
    }

    GOIO_SENSOR_HANDLE hDevice = OpenGoMotion(deviceName);
    if (hDevice == nullptr)
    {
        printf("Failed to open Go!Motion.\n");
        ShutdownGoIO();
        return 1;
    }

    ConfigureAndStart(hDevice);
    Sleep(1000);

    double distances[kMaxSamples];
    int n = ReadDistances(hDevice, distances, kMaxSamples);
    printf("Got %d distance readings.\n", n);

    CloseGoMotion(hDevice);
    ShutdownGoIO();
    return 0;
}