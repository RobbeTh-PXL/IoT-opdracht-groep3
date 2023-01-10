#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "iothub.h"
#include "iothub_device_client_ll.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/shared_util_options.h"

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
#include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

#include "iothubtransportmqtt.h"

/* Paste in the your iothub connection string  */
static const char* connectionString = "HostName=Opdracht1Hub.azure-devices.net;DeviceId=PYNQZ2-ROBBE;SharedAccessKey=n7DiNuComUT/UPXnTjcvVeWgby/sOiHzkJmsFN7gxbE=";
#define MESSAGE_COUNT        200
static bool g_continueRunning = true;
static size_t g_message_count_send_confirmations = 0;

static void send_confirm_callback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    (void)userContextCallback;
    // When a message is sent this callback will get invoked
    g_message_count_send_confirmations++;
    (void)printf("Confirmation callback received for message %lu with result %s\r\n", (unsigned long)g_message_count_send_confirmations, MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}

static void connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{
    (void)reason;
    (void)user_context;
    // This sample DOES NOT take into consideration network outages.
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
    {
        (void)printf("The device client is connected to iothub\r\n");
    }
    else
    {
        (void)printf("The device client has been disconnected\r\n");
    }
}

int main(void)
{
  IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol;
  IOTHUB_MESSAGE_HANDLE message_handle;
  size_t messages_sent = 0;
  srand(time(NULL));
  int value_msg;
  const char* telemetry_msg = "test_message";

  protocol = MQTT_Protocol;

  // Used to initialize IoTHub SDK subsystem
  (void)IoTHub_Init();

  IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;

  (void)printf("Creating IoTHub Device handle\r\n");
  // Create the iothub handle here
  device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString, protocol);
  if (device_ll_handle == NULL)
  {
      (void)printf("Failure creating IotHub device. Hint: Check your connection string.\r\n");
  }
  else
  {
      // Set any option that are necessary.
      // For available options please see the iothub_sdk_options.md documentation
      // Setting connection status callback to get indication of connection to iothub
      (void)IoTHubDeviceClient_LL_SetConnectionStatusCallback(device_ll_handle, connection_status_callback, NULL);

      do
      {
          if (messages_sent < MESSAGE_COUNT)
          {
              value_msg = rand() % 101;
              // Construct the iothub message from a string or a byte array
              message_handle = IoTHubMessage_CreateFromString((char)value_msg);
              //message_handle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText)));

              // Set Message property
              /*
              (void)IoTHubMessage_SetMessageId(message_handle, "MSG_ID");
              (void)IoTHubMessage_SetCorrelationId(message_handle, "CORE_ID");
              (void)IoTHubMessage_SetContentTypeSystemProperty(message_handle, "application%2fjson");
              (void)IoTHubMessage_SetContentEncodingSystemProperty(message_handle, "utf-8");
              (void)IoTHubMessage_SetMessageCreationTimeUtcSystemProperty(message_handle, "2020-07-01T01:00:00.346Z");
              */


              // Add custom properties to message
              (void)IoTHubMessage_SetProperty(message_handle, "property_key", "property_value");

              (void)printf("Sending message %d to IoTHub\r\n", (int)(messages_sent + 1));
              IoTHubDeviceClient_LL_SendEventAsync(device_ll_handle, message_handle, send_confirm_callback, NULL);

              // The message is copied to the sdk so the we can destroy it
              IoTHubMessage_Destroy(message_handle);

              messages_sent++;
          }
          else if (g_message_count_send_confirmations >= MESSAGE_COUNT)
          {
              // After all messages are all received stop running
              g_continueRunning = false;
          }

          IoTHubDeviceClient_LL_DoWork(device_ll_handle);
          ThreadAPI_Sleep(1);

      } while (g_continueRunning);

      // Clean up the iothub sdk handle
      IoTHubDeviceClient_LL_Destroy(device_ll_handle);
  }
  // Free all the sdk subsystem
  IoTHub_Deinit();

  printf("Program FINISHED! Press any key to continue...");
  (void)getchar();

  return 0;
}
