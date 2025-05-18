#include <iostream>
#include <type_traits>
#include <tuple>
#include <string>
#include <format>
#include <exception>
#include <source_location>
#include <memory>
#include <functional>

namespace cppdishka {

template<typename T, typename ... Ts>
concept is_any_of = (... || std::is_same_v<T, Ts>);

enum class e_scope : unsigned char {
    scope_none = 0,
    scope_app,
    scope_request
};

template<typename T>
struct DishkaBox {
    std::shared_ptr<T> object{};
    e_scope scope{ e_scope::scope_app };
    std::function<T()> recreate{};

    T& get_object() {
        if (scope == e_scope::scope_request)
            *object = recreate();
        return *object;
    }
};

template<typename T>
DishkaBox<T> create_dishkabox(T&& object, e_scope scope = e_scope::scope_app) {
    return DishkaBox{ std::make_shared<T>(std::forward<T>(object)) , scope };
}

template<typename T, typename F>
DishkaBox<T> create_dishkabox(T&& object, F&& function, e_scope scope = e_scope::scope_app) {
    DishkaBox<T> db;
    db.object = std::make_shared<T>(std::forward<T>(object));
    db.recreate = std::forward<F>(function);
    db.scope = scope;
    return db;
}

template<typename ... Ts>
using depends = std::tuple<DishkaBox<Ts>...>;

template<typename T>
struct provider_type_t{};

template<typename ... Ts>
class Provider {
public:
    depends<Ts...> _depends;
    template<typename T>
    void _init_box(provider_type_t<T>) {
        std::get<DishkaBox<T>>(_depends) = DishkaBox<T>();
    }
public:
    Provider() {
        (_init_box<Ts>({}),...);
    }

    template<typename T>
    bool is_provide(provider_type_t<T>) const {
        if constexpr (is_any_of<T, Ts...>)
            return true;
        return false;
    }

    template<typename T>
    T& get(provider_type_t<T>) {
        return _get_box<T>({}).get_object();
    }    
    template<typename T>
    decltype(auto) _get_box(provider_type_t<T>) {
        return std::get<DishkaBox<T>>(_depends);
    }

    template<typename T>
    void provide(DishkaBox<T>&& db) {
        _get_box<int>({}) = std::move(db);
    }
};


#define inject_impl(var_name, type_name) var_name (p.get< type_name >({}))

#define Inject1(Class, Pr, v1, t1) Class (Pr& p) : inject_impl(v1, t1) {} 


}