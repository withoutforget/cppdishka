#include <iostream>
#include <string>
#include <format>
#include <source_location>
#include "cppdishka.h"

using namespace cppdishka;
int c = 0;

class TestProvider : public Provider<int> {
public:
    TestProvider() {
        provide<int>(create_dishkabox<int>(14, []() -> int { return (c++ % 2 == 0) ? 14 : 42 ;} ,e_scope::scope_request));
    }
};

class Object {
public:
    int x;

    Object(TestProvider& provider) : x(provider.get<int>({})) {
    }
};

int main() {
    TestProvider provider;
    for (size_t i = 0; i < 10; i++) {
        std::cout << Object(provider).x << std::endl;
    }
}