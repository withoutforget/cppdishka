#include <iostream>
#include <type_traits>
#include <tuple>
#include <string>
#include <format>
#include <exception>
#include <source_location>

template<typename T, typename ... Ts>
concept is_any_of = (... || std::is_same_v<T, Ts>);


template<typename ... Ts>
using depends = std::tuple<Ts...>;

template<typename T>
struct provider_t {};

template<typename ... Ts>
class Provider {
public:
    template<typename T>
    bool is_provided(provider_t<T>) const {
        if constexpr (is_any_of<T, Ts...>) {
            return true;
        }
        return false;
    }
};

class TestProvider : public Provider<int, float> {
public:
    auto provide(provider_t<int>) const { return 14; }
    float provide(provider_t<float>) const { return 3.14; }
};

template<class ProviderT, class Type, class ... Ts>
void inject_type_impl(const ProviderT& p, std::tuple<Ts...>& t, std::source_location loc = std::source_location::current()) {
    std::get<Type>(t) = p.provide(provider_t<Type>{});
}

template<class ProviderT, class ... Ts>
void inject_type(const ProviderT& p, std::tuple<Ts...>& t) {
    (inject_type_impl<ProviderT, Ts, Ts...>(p, t), ...);
}

template<class T, class ProviderT>
T inject(const ProviderT& provider) {
    auto depends = typename T::depends_t{};
    inject_type(provider, depends);
    return T{depends};
}

#define DI_TN(T1) using depends_t = depends<T1>; 
#define DI_TN2(T1,T2) using depends_t = depends<T1,T2>; 
#define DI_TN3(T1,T2,T3) using depends_t = depends<T1,T2,T3>; 
#define DI_TN4(T1,T2,T3,T4) using depends_t = depends<T1,T2,T3,T4>; 

#define DI_CT1(N1,T1) N1 = std::get<T1>(depends);
#define DI_CT2(N1,T1,N2,T2) DI_CT1(N1, T1) DI_CT1(N2,T2)

#define DI1(Class, N1, T1) DI_TN(T1) Class (depends_t depends) { DI_CT1(N1, T1) }
#define DI2(Class, N1, T1, N2, T2) DI_TN2(T1,T2) Class (depends_t depends) { DI_CT2(N1, T1, N2, T2) }



class MyClass {
public:
    DI2(MyClass, 
        x, int,
        y, float)
private:
    int x;
    float y;
public:
    std::string get_string() const { 
        return std::format("x={}, y = {}.", x, y);
    }                           
};

int main() try {
    TestProvider provider;
    auto object = inject<MyClass>(provider);
    std::cout << object.get_string() << std::endl;
    
} catch(std::exception& e) {
    std::cout << "got exception: " <<  e.what() << std::endl;
} catch(...) {
    std::cout << "got unknown exception." << std::endl;
}