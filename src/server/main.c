#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <netinet/in.h>
#include <server/serverUtils.h>
#include <utils/logger.h>
#include <server/selector.h>

int serverRunning = 1;

// Signal handler function
void handleSignal(int signal)
{
        if (signal == SIGINT || signal == SIGTSTP) {
                log(INFO, "Signal Received. Stopping server");
                serverRunning = 0;
        }
}

void dummy_read(struct selector_key * key) {
    log(INFO, "Me llego algo");
}

int main(void)
{
        unsigned int port = 110;
        // TODO: Receive port via args

        //close(0); // Nothing to read from stdin
        
        const char *err_msg = NULL;
        selector_status ss = SELECTOR_SUCCESS;
        fd_selector selector = NULL;

        int masterSocket = createTCPSocketServer(port);
        if (masterSocket < 0) {
                log(ERROR, "Couln't create master socket");
                goto finally;
        }

        // Register signal handlers
        signal(SIGINT, handleSignal);
        signal(SIGTSTP, handleSignal);

        if(selector_fd_set_nio(masterSocket) == -1) {
            err_msg = "getting server socket flags";
            goto finally;
        }

        const struct selector_init conf = {
            .signal = SIGALRM,
            .select_timeout = {
                .tv_sec = 10,
                .tv_nsec = 0,
            },
        };

        if(0 != selector_init(&conf)) {
            err_msg = "Error on selector_init";
            goto finally;
        }


        selector = selector_new(1024);
        if (selector == NULL) {
            err_msg = "Unable to create selector";
            goto finally;
        }

        const struct fd_handler masterHandler = {
            .handle_read = NULL,
            .handle_write = NULL,
            .handle_close = NULL,
        };

        ss = selector_register(selector, masterSocket, &masterHandler, OP_READ, NULL);

        if (ss != SELECTOR_SUCCESS) {
            err_msg = "Error registering master handler";
            goto finally;
        }

        while (serverRunning) {
            err_msg = NULL;
            ss = selector_select(selector);
            if (ss != SELECTOR_SUCCESS) {
                err_msg = "Error serving";
                goto finally;
            }
        }

        int ret = 0;
finally:
        
        if (ss != SELECTOR_SUCCESS)  {
            log(ERROR, "%s: %s", (err_msg == NULL) ? "" : err_msg, ss == SELECTOR_IO ? strerror(errno) : selector_error(ss));
            ret = 2;
        } else if (err_msg) {
            log(ERROR, "%s", err_msg);
            ret = 1;
        }

        if (selector != NULL) {
            selector_destroy(selector);
        } 

        if (masterSocket >= 0) {
            close(masterSocket);
        }

        selector_close();

        return ret;
}
