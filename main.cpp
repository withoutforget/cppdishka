#include <iostream>
#include <string>
#include <format>
#include <source_location>
#include "cppdishka.h"

int c = 0;

class TestProvider : public cppdishka::Provider<int> {
public:
    TestProvider() {
        provide<int>(cppdishka::create_dishkabox<int>(14, []() -> int { return (c++ % 2 == 0) ? 14 : 42 ;} ,cppdishka::e_scope::scope_request));
    }
};

class Object {
public:
    Inject1(Object, TestProvider, 
        x, int)

    std::string test() const { 
        return std::format("x={}\n", x);
    }
private:
    int x;
};

template<class T, class ProviderT>
T inject(ProviderT& p) {
    return T{p};
}

int main() {
    TestProvider provider;
    for (size_t i = 0; i < 10; i++) {
        auto obj = inject<Object>(provider);
        std::cout << obj.test();
    }
}