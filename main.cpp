#include <iostream>
#include <type_traits>
#include <tuple>
#include <string>
#include <format>
#include <exception>
#include <source_location>
#include <memory>

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

template<typename T>
concept is_injectable = requires(T x) { typename T::depends_t; };

template<class ProviderT, class _Type, class ... Ts>
void inject_type_impl(const ProviderT& p, std::tuple<Ts...>& t, std::source_location loc = std::source_location::current()) {
    using Type = typename _Type::element_type;
    if constexpr (is_injectable<Type>)
    {
        Type&& temp = inject<Type>(p);
        std::get<_Type>(t) = std::move(std::make_unique<Type>(std::move(temp)));
    } else
        std::get<_Type>(t) = std::move(std::make_unique<Type>(p.provide(provider_t<Type>{})));
}

template<class ProviderT, class ... Ts>
void inject_type(const ProviderT& p, std::tuple<Ts...>& t) {
    (inject_type_impl<ProviderT, Ts, Ts...>(p, t), ...);
}

template<typename ... Ts>
std::tuple<std::unique_ptr<Ts>...> init_tuple(std::tuple<Ts...>*) {
    return std::make_tuple<std::unique_ptr<Ts>...>((sizeof(Ts) - sizeof(Ts), nullptr)...);
}

template<typename ... Ts>
decltype(auto) deptr(Ts&&... ps) {
    return std::make_tuple(ps.get()...);
}

template<typename ... Ts>
std::tuple<Ts...> reconvert_tuple(std::tuple<std::unique_ptr<Ts>...>& tuple) {
    return std::apply([](auto&... ptrs) {
        return std::make_tuple(*ptrs...);
    }, tuple);
}

template<is_injectable T, class ProviderT>
T inject(const ProviderT& provider) {
    auto depends = init_tuple((typename T::depends_t*)(nullptr));
    inject_type(provider, depends);
    return T{reconvert_tuple(depends)};
}

#define DI_TN(T1) using depends_t = depends<T1>; 
#define DI_TN2(T1,T2) using depends_t = depends<T1,T2>; 
#define DI_TN3(T1,T2,T3) using depends_t = depends<T1,T2,T3>; 
#define DI_TN4(T1,T2,T3,T4) using depends_t = depends<T1,T2,T3,T4>; 

#define DI_CT1(N1,T1) N1(std::get<T1>(depends))
#define DI_CT2(N1,T1,N2,T2) DI_CT1(N1, T1), DI_CT1(N2,T2)
#define DI_CT3(N1,T1,N2,T2,N3,T3) DI_CT1(N1, T1), DI_CT1(N2,T2), DI_CT1(N3,T3)

#define DI1(Class, N1, T1) DI_TN(T1) \
        Class (depends_t depends) : DI_CT1(N1, T1) { }
#define DI2(Class, N1, T1, N2, T2) DI_TN2(T1,T2) \
        Class (depends_t depends) : DI_CT2(N1, T1, N2, T2) { }
#define DI3(Class, N1, T1, N2, T2, N3, T3) DI_TN3(T1,T2,T3) \
        Class (depends_t depends) : DI_CT3(N1, T1, N2, T2,N3,T3) { }

#define sprovide(T, VAL) decltype(auto) provide(provider_t<T>) const { return VAL ; }


struct A {    
    DI1(A,
        _t, float)
    
    float _t;
    std::string foo() const { return std::to_string(_t);}
};

class MyClass {
public:
    DI3(MyClass, 
        x, int,
        y, float,
        a, A)
private:
    int x;
    float y;
    A a;
public:
    std::string get_string() const { 
        return std::format("x={}, y = {}, a = {}.", x, y, a.foo());
    }                           
};

class TestProvider : public Provider<int, float, A> {
public:
    sprovide(int, 14)
    sprovide(float, 3.14)
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