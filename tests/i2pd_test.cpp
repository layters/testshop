#include <api.h>
#include <SAM.h>

void start_router(int argc, char* argv[]) {
    i2p::api::InitI2P(argc, argv, "neromon");

    // Your application logic

    i2p::api::StartI2P(std::shared_ptr<std::ostream>(nullptr)); // Pass a log stream if needed

    // Your application continues running

    i2p::api::StopI2P();
}

int main(int argc, char* argv[]) {
    return 0;
}
