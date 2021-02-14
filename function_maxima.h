#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <set>
#include <memory>

// Klasa wyjątku który mamy zwrócić
class InvalidArg : public std::exception {
private:
    const char *errorMessage;
public:
    explicit InvalidArg(const char *message) : errorMessage(message) {}

    [[nodiscard]] const char *what() const noexcept override {
        return errorMessage;
    }
};

template<typename A, typename V>
class FunctionMaxima {
private:
    // Typ który reprezentuje sparametryzowany iterator multisetu
    template<typename T>
    using mset_iterator = typename std::multiset<T>::iterator;
public:
    class point_type;

    // Nasz iterator do funkcji będzie iteratorem multisetu, posiada te same
    // funkcjonalności jakie są od nas w treści wymagane
    using iterator = mset_iterator<point_type>;

    // Klasa size_type z treści jest inaczej nazwanym
    // size_t tu to implementujemy
    using size_type = size_t;

    // Konstruktor bezparametrowy
    FunctionMaxima() = default;

    // Konstruktor kopiujący
    FunctionMaxima(const FunctionMaxima<A, V> &funMax) = default;

    // Destruktor
    ~FunctionMaxima() noexcept = default;

    // Operator =
    FunctionMaxima &operator=(const FunctionMaxima<A, V> &funMax) {
        if (&funMax == this) {
            return *this;
        }
        auto prev_fun_size = function.size();
        auto prev_mx_size = maxima.size();
        iterator prev_fun_it[function.size()];
        mset_iterator<iterator> prev_mx_it[maxima.size()];

        // Zapis iteratorów do elementów nadpisywanej funkcji i maksimów
        size_t i = 0;
        for (auto it = function.begin(); it != function.end(); it++) {
            prev_fun_it[i++] = it;
        }
        i = 0;
        for (auto it = maxima.begin(); it != maxima.end(); it++) {
            prev_mx_it[i++] = it;
        }

        // Tablica iteratorów do nowo wstawionych argumentów do
        // multisetu funkcji
        iterator inserted_fun[funMax.function.size()];

        // Tablica iteratorów do elementów które trzeba wstawić do multisetu
        // maximów
        iterator mx_to_insert[funMax.maxima.size()];

        // Tablica iteratorów do nowo wstawionych argumentów do
        // multisetu maximów
        mset_iterator<iterator> inserted_mx[funMax.maxima.size()];

        // ilość obrotów pętli pierwszej w catchu
        size_t fun_count = 0;

        // ilość obrotów drugiej pętli w catchu
        size_t mx_count = 0;

        // Zmienna pomocnicza do zapełniania tablicy mx_to_insert
        size_t mx_ins_count = 0;

        try {
            // Próbujemy wstawić elementy z drugiego FunctionMaxima na nasze
            // multisety
            for (auto it = funMax.function.begin();
                 it != funMax.function.end(); it++) {
                inserted_fun[fun_count] = function.insert(*it);
                fun_count++;

                if (funMax.maxima.find(it) != funMax.maxima.end()) {
                    mx_to_insert[mx_ins_count++] = inserted_fun[fun_count - 1];
                }
            }
            for (size_t k = 0; k < mx_ins_count; k++) {
                inserted_mx[mx_count] = maxima.insert(mx_to_insert[k]);
                mx_count++;
            }
        } catch (...) {
            // W przypadku wyjątku usuwamy elementy które dodaliśmy z drugiej
            // klasy
            for (size_t k = 0; k < fun_count; k++) {
                function.erase(inserted_fun[k]);
            }

            for (size_t k = 0; k < mx_count; k++) {
                maxima.erase(inserted_mx[k]);
            }
            throw;
        }

        // Tutaj usuwamy elementy pierwotne dwóch setów, już ich nie
        // potrzebujemy, nie będziemy rollbackować ani to nie da wyjątku
        for (i = 0; i < prev_fun_size; i++) {
            function.erase(prev_fun_it[i]);
        }

        for (i = 0; i < prev_mx_size; i++) {
            maxima.erase(prev_mx_it[i]);
        }

        return *this;
    }

    // Klasa służąca do reprezentowania pojedynczego punktu w funkcji
    class point_type {
    private:
        // Chcemy żeby FunctionMaxima miał dostęp do konstruktorów point_type
        friend FunctionMaxima<A, V>;

        std::shared_ptr<A> argument;
        std::shared_ptr<V> val;

        // Konstruktor pomocniczy, w niektórych funkcjach nie mamy podanej
        // wartości lecz sam argument
        explicit point_type(A const &a) {
            argument = std::make_shared<A>(a);
            val = nullptr;
        }

        // Konstruktor punktu
        point_type(A const &a, V const &v) {
            argument = std::make_shared<A>(a);
            val = std::make_shared<V>(v);
        }

    public:
        // Konstruktor kopiujący
        point_type(const point_type &poiTyp) :
                argument(poiTyp.argument), val(poiTyp.val) {}

        A const &arg() const {
            return *argument;
        }

        V const &value() const {
            return *val;
        }
    };

    // Klasa iteratora po maximach
    class mx_iterator {
    private:
        mset_iterator<iterator> itr;
    public:
        // Tagi iteratora, elementarne, żeby STL poprawnie obsługiwał go
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = point_type;
        using pointer = point_type const *;
        using reference = point_type const &;

        // Budujemy go na podstawie iteratora multisetu maximów
        explicit mx_iterator(const mset_iterator<iterator> &it) : itr(it) {}

        // *iterator
        reference operator*() const {
            return **itr;
        }

        // iterator->
        pointer operator->() const {
            return &(**itr);
        }

        // iterator++
        mx_iterator &operator++() noexcept {
            itr++;
            return *this;
        }

        // ++iterator
        mx_iterator operator++(int) noexcept {
            mx_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        // iterator--
        mx_iterator &operator--() noexcept {
            itr--;
            return *this;
        }

        // --iterator
        mx_iterator operator--(int) noexcept {
            mx_iterator tmp = *this;
            --(*this);
            return tmp;
        }

        // iteratorA == iteratorB
        friend bool operator==(const mx_iterator &a,
                               const mx_iterator &b) noexcept {
            return a.itr == b.itr;
        };

        // iteratorA != iteratorB
        friend bool operator!=(const mx_iterator &a,
                               const mx_iterator &b) noexcept {
            return a.itr != b.itr;
        };
    };

private:
    // Komparator do multisetu funkcji
    struct pointTypeCmp {
        bool operator()(point_type a, point_type b) const {
            // Sortujemy po rosnących argumentach
            return a.arg() < b.arg();
        }
    };

    // Multiset który przedstawia całą funkcję na której działamy
    std::multiset<point_type, pointTypeCmp> function;

    struct iteratorCmp {
        bool operator()(iterator a, iterator b) const {
            // Jeżeli wartości są różne sortujemy po nich
            if (a->value() < b->value() || b->value() < a->value())
                return b->value() < a->value();

            // Wpp sortujemy po rosnących argumentach
            return a->arg() < b->arg();
        }
    };

    // Multiset który przedstawia wszystie maxima naszej funkcji
    std::multiset<iterator, iteratorCmp> maxima;

    // Zwraca iterator do punktu znajdującego się na prawo od danego punktu.
    iterator get_right(iterator it, iterator ignore_it) const {
        it++;
        if (it != function.end()) {
            if (it == ignore_it) {
                it++;
                return it;
            }
            return it;
        }
        return function.end();
    }

    // Zwraca iterator do punktu znajdującego się na lewo od danego punktu
    iterator get_left(iterator it, iterator ignore_it) const {
        if (it != function.begin()) {
            it--;
            if (it == ignore_it) {
                if (it != function.begin()) {
                    it--;
                    return it;
                } else {
                    return function.end();
                }
            }
            return it;
        }
        return function.end();
    }

    // Sprawdza czy dany punkt jest lokalnym maksiumum
    bool check_if_maxima(iterator it, iterator ignore_it) const {
        auto point = *it;

        auto left_it = get_left(it, ignore_it);
        auto right_it = get_right(it, ignore_it);

        if (left_it != function.end() && point.value() < left_it->value()) {
            return false;
        }

        if (right_it != function.end() && point.value() < right_it->value()) {
            return false;
        }

        return true;
    }

    // Aktualizuje maxima z uwględnieniem nowo wstawionego punktu
    // (ignorując przy tym stary punkt jesli istnieje)
    void update_maxima(iterator it, iterator ignore_it) {
        mset_iterator<iterator> old_maxi_mx_it;
        if (ignore_it != function.end()) {
            old_maxi_mx_it = maxima.find(ignore_it);
        }

        bool add_new_maxi;
        if (it == ignore_it) {
            add_new_maxi = false;
        } else {
            add_new_maxi = check_if_maxima(it, ignore_it);
        }

        bool left_add_new_maxi = false, right_add_new_maxi = false;
        bool left_remove_old_maxi = false, right_remove_old_maxi = false;

        auto left_it = get_left(it, ignore_it);
        auto right_it = get_right(it, ignore_it);

        mset_iterator<iterator> mx_it, left_mx_it, right_mx_it;

        if (left_it != function.end()) {
            left_mx_it = maxima.find(left_it);

            if (left_mx_it == maxima.end()) {
                left_add_new_maxi = check_if_maxima(left_it, ignore_it);
            } else {
                left_remove_old_maxi = !check_if_maxima(left_it, ignore_it);
            }
        }

        if (right_it != function.end()) {
            right_mx_it = maxima.find(right_it);
            if (right_mx_it == maxima.end()) {
                right_add_new_maxi = check_if_maxima(right_it, ignore_it);
            } else {
                right_remove_old_maxi = !check_if_maxima(right_it, ignore_it);
            }
        }

        try {
            if (add_new_maxi) {
                mx_it = maxima.insert(it);
            }
        } catch (...) {
            throw;
        }

        try {
            if (left_add_new_maxi) {
                left_mx_it = maxima.insert(left_it);
            }
        } catch (...) {
            if (add_new_maxi) {
                maxima.erase(mx_it);
            }
            throw;
        }

        try {
            if (right_add_new_maxi) {
                right_mx_it = maxima.insert(right_it);
            }
        } catch (...) {
            if (add_new_maxi) {
                maxima.erase(mx_it);
            }
            if (left_add_new_maxi) {
                maxima.erase(left_mx_it);
            }
            throw;
        }

        if (ignore_it != function.end() && old_maxi_mx_it != maxima.end()) {
            maxima.erase(old_maxi_mx_it);
        }
        if (left_remove_old_maxi) {
            maxima.erase(left_mx_it);
        }
        if (right_remove_old_maxi) {
            maxima.erase(right_mx_it);
        }
    }

public:
    // Zmienia funkcję tak, żeby zachodziło f(a) = v. Jeśli a nie należy do
    // obecnej dziedziny funkcji, jest do niej dodawany. Najwyżej O(log n).
    void set_value(A const &a, V const &v) {
        point_type new_point = point_type(a, v);
        auto old_point_it = function.find(new_point);

        iterator it;
        try {
            it = function.insert(new_point);
        } catch (...) {
            throw;
        }

        try {
            update_maxima(it, old_point_it);
        } catch (...) {
            function.erase(it);
            throw;
        }

        if (old_point_it != function.end()) {
            function.erase(old_point_it);
        }
    }

    // Zwraca wartość w punkcie a, rzuca wyjątek InvalidArg, jeśli a nie
    // należy do dziedziny funkcji. Złożoność najwyżej O(log n).
    V const &value_at(A const &a) const {
        point_type searching = point_type(a);
        auto it = function.find(searching);

        if (it != function.end()) {
            return it->value();
        } else {
            throw InvalidArg
                    ("Argument is not in the domain of function!");
        }
    }

    // Usuwa a z dziedziny funkcji. Jeśli a nie należało do dziedziny funkcji,
    // nie dzieje się nic. Złożoność najwyżej O(log n).
    void erase(A const &a) {
        point_type to_erase = point_type(a);
        auto it = function.find(to_erase);

        if (it != function.end()) {
            try {
                update_maxima(it, it);
            } catch (...) {
                // Nic nie usuneliśmy jeszcze więc spokojnie możemy throwować
                throw;
            }
            function.erase(it);
        }
    }

    // iterator wskazujący na pierwszy punkt
    iterator begin() const noexcept {
        return iterator(function.begin());
    }

    // iterator wskazujący za ostatni punkt
    iterator end() const noexcept {
        return iterator(function.end());
    }

    // Iterator, który wskazuje na punkt funkcji o argumencie a lub end(),
    // jeśli takiego argumentu nie ma w dziedzinie funkcji.
    iterator find(A const &a) const {
        point_type searching = point_type(a);
        auto it = function.find(searching);
        return iterator(it);
    }

    // iterator wskazujący na pierwsze lokalne maksimum
    mx_iterator mx_begin() const noexcept {
        return mx_iterator(maxima.begin());
    }

    // iterator wskazujący za ostatnie lokalne maksimum
    mx_iterator mx_end() const noexcept {
        return mx_iterator(maxima.end());
    }

    // funkcja zwracająca rozmiar
    [[nodiscard]] size_type size() const noexcept {
        return function.size();
    }
};

#endif //FUNCTION_MAXIMA_H