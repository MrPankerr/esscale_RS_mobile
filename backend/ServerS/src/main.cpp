#include "headers.h"
//#include "../TLSsecret/ecdhe_params.pem"

int main(int argc, char* argv[])
{
    cmd_parse(argc, argv);
    
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    io::io_context io_context;
    server srv(io_context, 15001);
    srv.async_accept();
    io_context.run();

    return 0;
}