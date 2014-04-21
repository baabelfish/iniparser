#include "../parser.hpp"
#include <iostream>

int main() {
    IniP ip;
    ip.read("intest.ini");
    ip.set(L"1", L"Toka", L"arvo", L"tosimatala");
    ip.write("outtest.ini");
    return 0;
}
