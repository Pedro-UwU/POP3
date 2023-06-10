#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include "server/serverUtils.h"
#include "utils/logger.h"

int serverRunning = 1;

// Signal handler function
void handleSignal(int signal)
{
        if (signal == SIGINT || signal == SIGTSTP) {
                log(INFO, "Signal Received. Stopping server");
                serverRunning = 0;
        }
}

int main(int argc, char *argv[])
{
        char *defaultService = "110";
        // TODO: Receive port via args

        close(0); // Nothing to read from stdin

        int masterSocket = createTCPSocketServer(defaultService);
        if (masterSocket < 0) {
                log(ERROR, "Couln't create master socket");
                return -1;
        }

        // Register signal handlers
        signal(SIGINT, handleSignal);
        signal(SIGTSTP, handleSignal);

        while (serverRunning) {
                // Handle connections
        }

        close(masterSocket);

        return 0;
}
