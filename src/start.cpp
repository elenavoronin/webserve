#include "../include/Config.hpp"

/**
 * @brief Entry point of the server.
 *
 * Checks the command line arguments and calls checkConfig() to parse the
 * configuration file. If no arguments are provided, it defaults to
 * "configs/default.conf".
 */
int main(int argc, char **argv) {
    Config config;

    if (argc == 1)
        config.checkConfig("configs/default.conf");
    else
        config.checkConfig(argv[1]);
    return 0;
}
