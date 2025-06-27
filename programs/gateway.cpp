#include "gateway/gateway.h"


int main(int argc, char* argv[])
{
    RustCinder::Gateway gateway;
    gateway.init();
    gateway.start();
    return 0;
}