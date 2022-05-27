#include "include/main.h"
#include "include/invocation.h"

int main(int argc, char* argv[]) {
    // return compile(argc, argv);
    return invocation_entry(argc, argv);
}