#ifndef ZSTRING_VIEW_HPP
#define ZSTRING_VIEW_HPP

#include <cassert>
#include <compare>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <string>
#include <format>

namespace std {
    // [zstring.view.template], class template basic_zstring_view
    template<class charT, class traits = char_traits<charT>>
    class basic_zstring_view;                                              // partially freestanding

    namespace ranges {
        template<class charT, class traits>
            constexpr bool enable_view<basic_zstring_view<charT, traits>> = true;
        template<class charT, class traits>
            constexpr bool enable_borrowed_range<basic_zstring_view<charT, traits>> = true;
    }

    // [zstring.view.comparison], non-member comparison functions
    template<class charT, class traits>
    constexpr bool operator==(
        basic_zstring_view<charT, traits> x,
        type_identity_t<basic_zstring_view<charT, traits>> y
    ) noexcept;

    template<class charT, class traits>
    constexpr auto operator<=>(
        basic_zstring_view<charT, traits> x,
        type_identity_t<basic_zstring_view<charT, traits>> y
    ) noexcept;

    // [zstring.view.io], inserters and extractors
    template<class charT, class traits>
    basic_ostream<charT, traits>& operator<<(basic_ostream<charT, traits>& os, basic_zstring_view<charT, traits> str);

    // basic_zstring_view typedef-names
    using zstring_view    = basic_zstring_view<char>;
    using u8zstring_view  = basic_zstring_view<char8_t>;
    using u16zstring_view = basic_zstring_view<char16_t>;
    using u32zstring_view = basic_zstring_view<char32_t>;
    using wzstring_view   = basic_zstring_view<wchar_t>;

    // [zstring.view.hash], hash support
    template<class T> struct hash;
    template<> struct hash<zstring_view>;
    template<> struct hash<u8zstring_view>;
    template<> struct hash<u16zstring_view>;
    template<> struct hash<u32zstring_view>;
    template<> struct hash<wzstring_view>;

    inline namespace literals {
        inline namespace zstring_view_literals {
            #ifndef _MSC_VER
            #pragma GCC diagnostic push
            #ifdef __clang__
            #pragma GCC diagnostic ignored "-Wuser-defined-literals"
            #else
            #pragma GCC diagnostic ignored "-Wliteral-suffix"
            #endif
            #else
            #pragma warning(push)
            #pragma warning(disable: 4455)
            #endif
            // [zstring.view.literals], suffix for basic_zstring_view literals
            constexpr zstring_view    operator"" zsv(const char* str, size_t len) noexcept;
            constexpr u8zstring_view  operator"" zsv(const char8_t* str, size_t len) noexcept;
            constexpr u16zstring_view operator"" zsv(const char16_t* str, size_t len) noexcept;
            constexpr u32zstring_view operator"" zsv(const char32_t* str, size_t len) noexcept;
            constexpr wzstring_view   operator"" zsv(const wchar_t* str, size_t len) noexcept;
            #ifndef _MSC_VER
            #pragma GCC diagnostic pop
            #else
            #pragma warning(pop)
            #endif
        }
    }
}

namespace std {
    template<class charT, class traits /* = char_traits<charT> */>
    class basic_zstring_view {
    public:
        // types
        using traits_type            = traits;
        using value_type             = charT;
        using pointer                = value_type*;
        using const_pointer          = const value_type*;
        using reference              = value_type&;
        using const_reference        = const value_type&;
        using const_iterator         = const charT*; // see [zstring.view.iterators]
        using iterator               = const_iterator;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using reverse_iterator       = const_reverse_iterator;
        using size_type              = size_t;
        using difference_type        = ptrdiff_t;
        static constexpr size_type npos = size_type(-1);

        // [zstring.view.cons], construction and assignment
        constexpr basic_zstring_view() noexcept : size_() {
            static const charT empty_string[1]{};
            data_ = std::data(empty_string);
        }
        constexpr basic_zstring_view(const basic_zstring_view&) noexcept = default;
        constexpr basic_zstring_view& operator=(const basic_zstring_view&) noexcept = default;
        constexpr basic_zstring_view(const charT* str) : basic_zstring_view(str, traits::length(str)) {}
        constexpr basic_zstring_view(const charT* str, size_type len) : data_(str), size_(len) {
            assert(str[len] == charT());
        }
        basic_zstring_view(nullptr_t) = delete;

        // NOTE: Not part of proposal, just to make examples work since I can't add the conversion operator to
        // basic_string.
        template<typename Traits, typename Allocator>
        constexpr basic_zstring_view(const std::basic_string<charT, Traits, Allocator>& str)
            : basic_zstring_view(str.c_str(), str.size()) {}

        // [zstring.view.iterators], iterator support
        constexpr const_iterator begin() const noexcept {
            return data_;
        }
        constexpr const_iterator end() const noexcept {
            return data_ + size_;
        }
        constexpr const_iterator cbegin() const noexcept {
            return begin();
        }
        constexpr const_iterator cend() const noexcept {
            return end();
        }
        constexpr const_reverse_iterator rbegin() const noexcept {
            return data_ + size_;
        }
        constexpr const_reverse_iterator rend() const noexcept {
            return data_;
        }
        constexpr const_reverse_iterator crbegin() const noexcept {
            return rbegin();
        }
        constexpr const_reverse_iterator crend() const noexcept {
            return rend();
        }

        // [zstring.view.capacity], capacity
        constexpr size_type size() const noexcept {
            return size_;
        }
        constexpr size_type length() const noexcept {
            return size_;
        }
        constexpr size_type max_size() const noexcept {
            return basic_string_view<charT, traits>{}.max_size() - 1;
        }
        [[nodiscard]] constexpr bool empty() const noexcept {
            return size_ == 0;
        }

        // [zstring.view.access], element access
        constexpr const_reference operator[](size_type pos) const {
            assert(pos <= size_);
            return data_[pos];
        }
        constexpr const_reference at(size_type pos) const {
            if(pos > size_) {
                throw std::out_of_range(std::format("basic_zstring_view::at: pos ({}) > size() {}", pos, size_));
            }
            return data_[pos];
        }
        constexpr const_reference front() const {
            assert(!empty());
            return data_[0];
        }
        constexpr const_reference back() const {
            assert(!empty());
            return data_[size_ - 1];
        }
        constexpr const_pointer data() const noexcept {
            return data_;
        }
        constexpr const_pointer c_str() const noexcept {
            return data_;
        }

        operator basic_string_view<charT, traits>() const noexcept {
            return basic_string_view<charT, traits>{data_, size_};
        }

        // [zstring.view.modifiers], modifiers
        constexpr void remove_prefix(size_type n) {
            assert(n <= size());
            data_ += n;
            size_ -= n;
        }
        constexpr void swap(basic_zstring_view& s) noexcept {
            std::swap(data_, s.data_);
            std::swap(size_, s.size_);
        }

        // [zstring.view.ops], zstring operations
        constexpr size_type copy(charT* s, size_type n, size_type pos = 0) const {
            return basic_string_view<charT, traits>(*this).copy(s, n, pos);
        }

        constexpr basic_string_view<charT, traits> substr(size_type pos = 0, size_type n = npos) const {
            return basic_string_view<charT, traits>(*this).substr(pos, n);
        }

        constexpr int compare(basic_string_view<charT, traits> s) const noexcept {
            return basic_string_view<charT, traits>(*this).compare(s);
        }
        constexpr int compare(size_type pos1, size_type n1, basic_zstring_view s) const {
            return basic_string_view<charT, traits>(*this).compare(pos1, n1, s);
        }
        constexpr int compare(size_type pos1, size_type n1, basic_zstring_view s,
                            size_type pos2, size_type n2) const {
            return basic_string_view<charT, traits>(*this).compare(pos1, n1, s, pos2, n2);
        }
        constexpr int compare(const charT* s) const {
            return basic_string_view<charT, traits>(*this).compare(s);
        }
        constexpr int compare(size_type pos1, size_type n1, const charT* s) const {
            return basic_string_view<charT, traits>(*this).compare(pos1, n1, s);
        }
        constexpr int compare(size_type pos1, size_type n1, const charT* s, size_type n2) const {
            return basic_string_view<charT, traits>(*this).compare(pos1, n1, s, n2);
        }

        constexpr bool starts_with(basic_string_view<charT, traits> x) const noexcept {
            return basic_string_view<charT, traits>(*this).starts_with(x);
        }
        constexpr bool starts_with(charT x) const noexcept {
            return basic_string_view<charT, traits>(*this).starts_with(x);
        }
        constexpr bool starts_with(const charT* x) const {
            return basic_string_view<charT, traits>(*this).starts_with(x);
        }
        constexpr bool ends_with(basic_string_view<charT, traits> x) const noexcept {
            return basic_string_view<charT, traits>(*this).ends_with(x);
        }
        constexpr bool ends_with(charT x) const noexcept {
            return basic_string_view<charT, traits>(*this).ends_with(x);
        }
        constexpr bool ends_with(const charT* x) const {
            return basic_string_view<charT, traits>(*this).ends_with(x);
        }

        constexpr bool contains(basic_string_view<charT, traits> x) const noexcept {
            return basic_string_view<charT, traits>(*this).contains(x);
        }
        constexpr bool contains(charT x) const noexcept {
            return basic_string_view<charT, traits>(*this).contains(x);
        }
        constexpr bool contains(const charT* x) const {
            return basic_string_view<charT, traits>(*this).contains(x);
        }

        // [zstring.view.find], searching
        constexpr size_type find(basic_string_view<charT, traits> s, size_type pos = 0) const noexcept {
            return basic_string_view<charT, traits>(*this).find(s, pos);
        }
        constexpr size_type find(charT c, size_type pos = 0) const noexcept {
            return basic_string_view<charT, traits>(*this).find(c, pos);
        }
        constexpr size_type find(const charT* s, size_type pos, size_type n) const {
            return basic_string_view<charT, traits>(*this).find(s, pos, n);
        }
        constexpr size_type find(const charT* s, size_type pos = 0) const {
            return basic_string_view<charT, traits>(*this).find(s, pos);
        }
        constexpr size_type rfind(basic_string_view<charT, traits> s, size_type pos = npos) const noexcept {
            return basic_string_view<charT, traits>(*this).rfind(s, pos);
        }
        constexpr size_type rfind(charT c, size_type pos = npos) const noexcept {
            return basic_string_view<charT, traits>(*this).rfind(c, pos);
        }
        constexpr size_type rfind(const charT* s, size_type pos, size_type n) const {
            return basic_string_view<charT, traits>(*this).rfind(s, pos, n);
        }
        constexpr size_type rfind(const charT* s, size_type pos = npos) const {
            return basic_string_view<charT, traits>(*this).rfind(s, pos);
        }

        constexpr size_type find_first_of(basic_string_view<charT, traits> s, size_type pos = 0) const noexcept {
            return basic_string_view<charT, traits>(*this).find_first_of(s, pos);
        }
        constexpr size_type find_first_of(charT c, size_type pos = 0) const noexcept {
            return basic_string_view<charT, traits>(*this).find_first_of(c, pos);
        }
        constexpr size_type find_first_of(const charT* s, size_type pos, size_type n) const {
            return basic_string_view<charT, traits>(*this).find_first_of(s, pos, n);
        }
        constexpr size_type find_first_of(const charT* s, size_type pos = 0) const {
            return basic_string_view<charT, traits>(*this).find_first_of(s, pos);
        }
        constexpr size_type find_last_of(basic_string_view<charT, traits> s, size_type pos = npos) const noexcept {
            return basic_string_view<charT, traits>(*this).find_last_of(s, pos);
        }
        constexpr size_type find_last_of(charT c, size_type pos = npos) const noexcept {
            return basic_string_view<charT, traits>(*this).find_last_of(c, pos);
        }
        constexpr size_type find_last_of(const charT* s, size_type pos, size_type n) const {
            return basic_string_view<charT, traits>(*this).find_last_of(s, pos, n);
        }
        constexpr size_type find_last_of(const charT* s, size_type pos = npos) const {
            return basic_string_view<charT, traits>(*this).find_last_of(s, pos);
        }
        constexpr size_type find_first_not_of(basic_string_view<charT, traits> s, size_type pos = 0) const noexcept {
            return basic_string_view<charT, traits>(*this).find_first_not_of(s, pos);
        }
        constexpr size_type find_first_not_of(charT c, size_type pos = 0) const noexcept {
            return basic_string_view<charT, traits>(*this).find_first_not_of(c, pos);
        }
        constexpr size_type find_first_not_of(const charT* s, size_type pos, size_type n) const {
            return basic_string_view<charT, traits>(*this).find_first_not_of(s, pos, n);
        }
        constexpr size_type find_first_not_of(const charT* s, size_type pos = 0) const {
            return basic_string_view<charT, traits>(*this).find_first_not_of(s, pos);
        }
        constexpr size_type find_last_not_of(basic_string_view<charT, traits> s, size_type pos = npos) const noexcept {
            return basic_string_view<charT, traits>(*this).find_last_not_of(s, pos);
        }
        constexpr size_type find_last_not_of(charT c, size_type pos = npos) const noexcept {
            return basic_string_view<charT, traits>(*this).find_last_not_of(c, pos);
        }
        constexpr size_type find_last_not_of(const charT* s, size_type pos, size_type n) const {
            return basic_string_view<charT, traits>(*this).find_last_not_of(s, pos, n);
        }
        constexpr size_type find_last_not_of(const charT* s, size_type pos = npos) const {
            return basic_string_view<charT, traits>(*this).find_last_not_of(s, pos);
        }

    private:
        const_pointer data_;        // exposition only
        size_type size_;            // exposition only
    };
}

namespace std {
    template<class charT, class traits>
    constexpr bool operator==(
        basic_zstring_view<charT, traits> x,
        type_identity_t<basic_zstring_view<charT, traits>> y
    ) noexcept {
        return basic_string_view<charT, traits>(x) == basic_string_view<charT, traits>(y);
    }

    template<class charT, class traits>
    constexpr auto operator<=>(
        basic_zstring_view<charT, traits> x,
        type_identity_t<basic_zstring_view<charT, traits>> y
    ) noexcept {
        return basic_string_view<charT, traits>(x) <=> basic_string_view<charT, traits>(y);
    }

    template<class charT, class traits>
    basic_ostream<charT, traits>& operator<<(basic_ostream<charT, traits>& os, basic_zstring_view<charT, traits> str) {
        return os<<basic_string_view<charT, traits>(str);
    }

    template<> struct hash<zstring_view> {
        auto operator()(const zstring_view& sv) const noexcept {
            return std::hash<string_view>{}(sv);
        }
    };
    template<> struct hash<u8zstring_view> {
        auto operator()(const u8zstring_view& sv) const noexcept {
            return std::hash<u8string_view>{}(sv);
        }
    };
    template<> struct hash<u16zstring_view> {
        auto operator()(const u16zstring_view& sv) const noexcept {
            return std::hash<u16string_view>{}(sv);
        }
    };
    template<> struct hash<u32zstring_view> {
        auto operator()(const u32zstring_view& sv) const noexcept {
            return std::hash<u32string_view>{}(sv);
        }
    };
    template<> struct hash<wzstring_view> {
        auto operator()(const wzstring_view& sv) const noexcept {
            return std::hash<wstring_view>{}(sv);
        }
    };

    inline namespace literals {
        inline namespace zstring_view_literals {
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wliteral-suffix"
            // [zstring.view.literals], suffix for basic_zstring_view literals
            constexpr zstring_view    operator""zsv(const char* str, size_t len) noexcept {
                return basic_zstring_view(str, len);
            }
            constexpr u8zstring_view  operator""zsv(const char8_t* str, size_t len) noexcept {
                return basic_zstring_view(str, len);
            }
            constexpr u16zstring_view operator""zsv(const char16_t* str, size_t len) noexcept {
                return basic_zstring_view(str, len);
            }
            constexpr u32zstring_view operator""zsv(const char32_t* str, size_t len) noexcept {
                return basic_zstring_view(str, len);
            }
            constexpr wzstring_view   operator""zsv(const wchar_t* str, size_t len) noexcept {
                return basic_zstring_view(str, len);
            }
            #pragma GCC diagnostic pop
        }
    }

    // [format.formatter.spec]
    template<class charT, class traits>
    struct formatter<basic_zstring_view<charT, traits>, charT> {
        formatter() = default;
        constexpr auto parse(basic_format_parse_context<charT>& context) {
            return sv_formatter.parse(context);
        }
        template<typename _Out>
        auto format(basic_zstring_view<charT, traits> zsv, basic_format_context<_Out, charT>& context) const {
            return sv_formatter.format(zsv, context);
        }
    private:
        formatter<basic_string_view<charT, traits>, charT> sv_formatter;
    };
}

#endif
