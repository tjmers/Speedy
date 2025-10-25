#include "../src/sync_client.h"

#include <string>


int main() {
    std::string ip = "52.90.126.152";
    std::string filename = "test.txt";

    start_listening(ip, filename);


    // Observation for the success will need to be done manually.

    return 0;
}