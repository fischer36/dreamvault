#include "api.h"
#ifdef __linux__
#include "server_linux.c"
#define OS "LIN"
#elif _WIN32
#include "server_windows.c"
#define OS "WIN"
#else
#define OS "WIN"
#include "server_windows.c"
#endif

/*
gcc src/db.c src/sys.c src/util.c src/worker.c src/http_parser.c src/main.c src/api.c -o libs/server.exe -Isrc/  -I"C:/Program Files/OpenSSL-Win64/include" -L"C:/Program Files/OpenSSL-Win64/lib/vc/x64/md" -I"C:/Program Files/MySQL/MySQL Server 8.3/include" -I"C:/C Libraries/libsodium-win64/include" -L"C:/Program Files/MySQL/MySQL Server 8.3/lib" -L"C:/C Libraries/libsodium-win64/lib" -llibsodium -lssl -lcrypto -lws2_32 -lmysql; cd libs; ./server.exe; cd ..
*/

int main()
{
    if (OS == "WIN")
    {
        start_server();
    }
    else
    {
        start_server();
    }

    // const char *http_request_str = "POST /user/123 HTTP/1.1\r\n"
    //                                "Host: example.com\r\n"
    //                                "Content-Type: application/json\r\n"
    //                                "Content-Length: 60\r\n"
    //                                "\r\n"
    //                                "\"text\": {\"username\": \"test\", "
    //                                "\"password\": \"secret\"}\r\n"
    //                                "\r\n";
    //
    // // End of HTTP request
    // //
    // char t[1024];
    // api_request_handler(http_request_str, t);
    return 0;
}
