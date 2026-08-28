// Minimal single-measurement Go!Motion read.
// Stripped from GoIO_DeviceCheck.cpp down to just: find -> open -> start ->
// read one value -> convert -> print -> close.

#include <cstdio>
#include <windows.h>
#include "GoIO_DLL_interface.h"

int main() {
  GoIO_Init();

  // Find the first available Go!Motion.
  char deviceName[GOIO_MAX_SIZE_DEVICE_NAME];
  int numFound = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID);
  if (numFound <= 0) {
      printf("No Go!Motion found.\n");
      GoIO_Uninit();
      return 1;
  }
  GoIO_GetNthAvailableDeviceName(deviceName, GOIO_MAX_SIZE_DEVICE_NAME,
                                 VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID, 0);

  GOIO_SENSOR_HANDLE hDevice = GoIO_Sensor_Open(deviceName, VERNIER_DEFAULT_VENDOR_ID,
                                                 CYCLOPS_DEFAULT_PRODUCT_ID, 0);
  if (hDevice == nullptr) {
      printf("Failed to open Go!Motion.\n");
      GoIO_Uninit();
      return 1;
  }

  double taj_period = 0.1;

  // Start the sensor sampling at 40ms/measurement.
  GoIO_Sensor_SetMeasurementPeriod(hDevice, taj_period, SKIP_TIMEOUT_MS_DEFAULT);
  GoIO_Sensor_SendCmdAndGetResponse(hDevice, SKIP_CMD_ID_START_MEASUREMENTS,
                                     NULL, 0, NULL, NULL, SKIP_TIMEOUT_MS_DEFAULT);

  // The sensor is asynchronous -- give it time for at least one 40ms cycle
  // to complete before asking for a reading.
  Sleep(taj_period * 1000 * 5);

  // bool mes_not_null = false;
  // while (mes_not_null) {
  //
  // }

  // GetLatestRawMeasurement() is the purpose-built call for "just give me
  // one value" -- no array, no count, unlike ReadRawMeasurements().

  gtype_int32 raw = GoIO_Sensor_GetLatestRawMeasurement(hDevice);
  double volts  = GoIO_Sensor_ConvertToVoltage(hDevice, raw);   // for Go!Motion this IS meters already
  double meters = GoIO_Sensor_CalibrateData(hDevice, volts);   // pass-through for this sensor

  printf("Distance: %.3f m\n", meters);

  GoIO_Sensor_Close(hDevice);
  GoIO_Uninit();
  return 0;
}