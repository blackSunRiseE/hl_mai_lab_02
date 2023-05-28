#include "auth_service/auth_rest.h"

int main(int argc, char*argv[]) {
    HTTPAuthWebServer app;
    return app.run(argc, argv);
}