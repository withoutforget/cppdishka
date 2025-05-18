#include <iostream>
#include <string>
#include <format>
#include <source_location>
#include "cppdishka.h"
template<typename T>
struct ConsoleClass {
    const std::string m_output;
    ConsoleClass(const std::string& output, std::source_location l = std::source_location::current()) : m_output(output) {
        std::cout << std::format("{} ({})\n\n", m_output, l.function_name());
    }
    ConsoleClass(const ConsoleClass& rhs,std::source_location l = std::source_location::current()) : m_output(rhs.m_output) {
        std::cout << std::format("{} ({})\n\n", m_output, l.function_name());
    }
    ~ConsoleClass() {
        std::cout << std::format("~{}\n", m_output);
    }
};

class GProvider : public cppdishka::Provider<ConsoleClass<int>, ConsoleClass<float>> {
public:
    request_provide(ConsoleClass<int>, ConsoleClass<int>("Int"))
    app_provide(ConsoleClass<float>, ConsoleClass<float>("Float"))
};

struct A {
    DI2(A,
    a, ConsoleClass<int>,
    b, ConsoleClass<float>
    )


    ConsoleClass<int>& a;
    ConsoleClass<float>& b;
};

int main() {
    GProvider provider;
    for (volatile size_t i = 0; i < 10; i++) {
        volatile auto obj = cppdishka::inject<A>(provider);
        std::cout << std::format("test ptr = {}\n", const_cast<void *>(reinterpret_cast<volatile void*>(&obj)));
    }
}