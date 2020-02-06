#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <Ntddndis.h>
#pragma comment(lib, "IPHLPAPI.lib")

int main()
{
	printf("Allocating space ...\n");

	ULONG buffer_size = 0;
	IP_ADAPTER_ADDRESSES* physical_ip_table = (IP_ADAPTER_ADDRESSES*)malloc(sizeof(IP_ADAPTER_ADDRESSES));
	while(GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, physical_ip_table, &buffer_size) == ERROR_BUFFER_OVERFLOW)
		physical_ip_table = (IP_ADAPTER_ADDRESSES*)realloc(physical_ip_table, buffer_size);

	printf("Allocated: %d bytes ...\nLooping devices and searching MAC's ...\n", buffer_size);

	while (physical_ip_table)
	{
		BYTE permanent_mac_address[6] = {};
		DWORD ioctl_code = OID_802_3_PERMANENT_ADDRESS;
		DWORD bytes_returned = 0;

		char guid_handle[MAX_PATH] = {};
		sprintf(guid_handle, "\\\\.\\%s", physical_ip_table->AdapterName);

		HANDLE device_handle = CreateFileA(guid_handle, NULL, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		if (device_handle && device_handle != INVALID_HANDLE_VALUE)
		{
			BOOL device_status = DeviceIoControl(device_handle, IOCTL_NDIS_QUERY_GLOBAL_STATS, &ioctl_code, sizeof(ioctl_code), permanent_mac_address, sizeof(permanent_mac_address), &bytes_returned, NULL);
			if (device_status)
			{
				printf("Permanent: ");
				for (int i = 0; i < sizeof(permanent_mac_address); i++)
					printf("%02X%s", permanent_mac_address[i], i == 5 ? "" : "-");

				ioctl_code = OID_802_3_CURRENT_ADDRESS;
				device_status = DeviceIoControl(device_handle, IOCTL_NDIS_QUERY_GLOBAL_STATS, &ioctl_code, sizeof(ioctl_code), permanent_mac_address, sizeof(permanent_mac_address), &bytes_returned, NULL);
				if (device_status)
				{
					printf(" | Current: [");
					for (int i = 0; i < sizeof(permanent_mac_address); i++)
						printf("%02X%s", permanent_mac_address[i], i == 5 ? "" : "-");
					printf("]\n");
				}
			}

			CloseHandle(device_handle);
		}

		physical_ip_table = physical_ip_table->Next;
	}

	free(physical_ip_table);

	system("pause");
	return EXIT_SUCCESS;
}
