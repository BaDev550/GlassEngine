#pragma once

namespace ge {
    // usage
    // for (const auto i : Counter(42));

    class Counter {
    public:
        constexpr explicit Counter(const size_t size) : m_size(size) {}

        class Iterator {
        public:
            constexpr Iterator(const size_t counter) : counter(counter) {}
            ~Iterator() = default;

            Iterator(const Iterator& other) {
                counter = other.counter;
            }

            constexpr size_t& operator*() { return counter; }
            constexpr Iterator& operator++() { ++counter; return *this; }
            constexpr bool operator!=(const Iterator& other) const { return (other.counter != counter); }
        private:
            size_t counter;
        };

        static constexpr Iterator begin() {
            return {0};
        }

        [[nodiscard]] Iterator end() const {
            return {m_size};
        }
    private:
        const size_t m_size;
    };
}
