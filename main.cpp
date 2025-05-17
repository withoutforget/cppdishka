#include <iostream>
#include <string>
#include <cppdishka.h>

class GProvider : public cppdishka::Provider<int, float> {
public:
    request_provide(int, 14)
    app_provide(float, 14)
};

int main() {

}