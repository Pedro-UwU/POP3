#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <netinet/in.h>
#include <server/pop3.h>
#include <server/serverUtils.h>
#include <server/selector.h>
#include <server/user.h>
#include <server/parsers/authParser.h>
#include <server/parsers/transParser.h>
#include <server/parsers/updateParser.h>
#include <utils/logger.h>

int serverRunning = 1;

// Signal handler function
void handleSignal(int signal)
{
        if (signal == SIGINT || signal == SIGTSTP) {
                log(INFO, "Signal Received. Stopping server");
                serverRunning = 0;
        }
}

int main(void)
{
        setLogLevel(DEBUG);
        unsigned int port = 60711;
        // TODO: Receive port via args
        user_add("USER1", "12345");

        close(0); // Nothing to read from stdin

        const char *err_msg = NULL;
        selector_status ss = SELECTOR_SUCCESS;
        fd_selector selector = NULL;

        int masterSocket = createTCPSocketServer(port);
        if (masterSocket < 0) {
                log(FATAL, "Couln't create master socket");
                goto finally;
        }

        // Register signal handlers
        signal(SIGINT, handleSignal);
        signal(SIGTSTP, handleSignal);

        if (selector_fd_set_nio(masterSocket) == -1) {
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

        if (0 != selector_init(&conf)) {
                err_msg = "Error on selector_init";
                goto finally;
        }

        selector = selector_new(1024);
        if (selector == NULL) {
                err_msg = "Unable to create selector";
                goto finally;
        }

        const struct fd_handler masterHandler = {
                .handle_read = pop3AcceptPassive,
                .handle_write = NULL,
                .handle_close = NULL,
        };

        ss = selector_register(selector, masterSocket, &masterHandler, OP_READ, NULL);

        if (ss != SELECTOR_SUCCESS) {
                err_msg = "Error registering master handler";
                goto finally;
        }

        conf_auth_parser();
        conf_trans_parser();
        conf_update_parser();

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
        if (ss != SELECTOR_SUCCESS) {
                log(ERROR, "%s: %s", (err_msg == NULL) ? "" : err_msg,
                    ss == SELECTOR_IO ? strerror(errno) : selector_error(ss));
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

        free_auth_parser();
        free_trans_parser();
        free_update_parser();

        return ret;
}
