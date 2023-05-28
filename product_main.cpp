#include "product_service/product_rest.h"

int main(int argc, char*argv[]) {
    HTTPProductWebServer app;
    return app.run(argc, argv);
}