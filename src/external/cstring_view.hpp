// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_CSTRING_VIEW_CSTRING_VIEW_HPP
#define BEMAN_CSTRING_VIEW_CSTRING_VIEW_HPP

#include <cassert>
#include <cstddef>
#if __has_include(<compare>)
    #include <compare>
#endif
#if __has_include(<format>)
    #include <format>
#endif
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <string>

namespace beman {

// [cstring.view.template], class template basic_cstring_view
template <class charT, class traits = std::char_traits<charT>>
class basic_cstring_view; // partially freestanding
/*
#if __cpp_lib_ranges >= 201911L
namespace ranges {
    template<class charT, class traits>
        constexpr bool enable_view<basic_cstring_view<charT, traits>> = true;
    template<class charT, class traits>
        constexpr bool enable_borrowed_range<basic_cstring_view<charT, traits>> = true;
}
#endif
*/
// [cstring.view.io], inserters and extractors
template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& os,
                                              basic_cstring_view<charT, traits>  str);

// basic_cstring_view typedef-names
using cstring_view = basic_cstring_view<char>;
#if __cpp_char8_t >= 201811L
using u8cstring_view = basic_cstring_view<char8_t>;
#endif
using u16cstring_view = basic_cstring_view<char16_t>;
using u32cstring_view = basic_cstring_view<char32_t>;
using wcstring_view   = basic_cstring_view<wchar_t>;

inline namespace literals {
inline namespace cstring_view_literals {
#ifndef _MSC_VER
    #pragma GCC diagnostic push
    #ifdef __clang__
        #pragma GCC diagnostic ignored "-Wuser-defined-literals"
    #else
        #pragma GCC diagnostic ignored "-Wliteral-suffix"
    #endif
#else
    #pragma warning(push)
    #pragma warning(disable : 4455)
#endif
// [cstring.view.literals], suffix for basic_cstring_view literals
constexpr cstring_view operator""_csv(const char* str, size_t len) noexcept;
#if __cpp_char8_t >= 201811L
constexpr u8cstring_view operator""_csv(const char8_t* str, size_t len) noexcept;
#endif
constexpr u16cstring_view operator""_csv(const char16_t* str, size_t len) noexcept;
constexpr u32cstring_view operator""_csv(const char32_t* str, size_t len) noexcept;
constexpr wcstring_view   operator""_csv(const wchar_t* str, size_t len) noexcept;
#ifndef _MSC_VER
    #pragma GCC diagnostic pop
#else
    #pragma warning(pop)
#endif
} // namespace cstring_view_literals
} // namespace literals
} // namespace beman

namespace beman {

template <class charT, class traits /* = char_traits<charT> */>
class basic_cstring_view {
  public:
    // types
    using traits_type               = traits;
    using value_type                = charT;
    using pointer                   = value_type*;
    using const_pointer             = const value_type*;
    using reference                 = value_type&;
    using const_reference           = const value_type&;
    using const_iterator            = const charT*; // see [cstring.view.iterators]
    using iterator                  = const_iterator;
    using const_reverse_iterator    = std::reverse_iterator<const_iterator>;
    using reverse_iterator          = const_reverse_iterator;
    using size_type                 = std::size_t;
    using difference_type           = std::ptrdiff_t;
    static constexpr size_type npos = size_type(-1);

  private:
    static constexpr charT empty_string[1]{};

  public:
    // [cstring.view.cons], construction and assignment
    constexpr basic_cstring_view() noexcept : size_() { data_ = std::data(empty_string); }
    constexpr basic_cstring_view(const basic_cstring_view&) noexcept            = default;
    constexpr basic_cstring_view& operator=(const basic_cstring_view&) noexcept = default;
    constexpr basic_cstring_view(const charT* str) : basic_cstring_view(str, traits::length(str)) {}
    constexpr basic_cstring_view(const charT* str, size_type len) : data_(str), size_(len) {
        assert(str[len] == charT());
    }
    constexpr basic_cstring_view(std::nullptr_t) = delete;

    // NOTE: Not part of proposal, just to make examples work since I can't add the conversion operator to
    // basic_string.
    template <typename Traits, typename Allocator>
    constexpr basic_cstring_view(const std::basic_string<charT, Traits, Allocator>& str)
        : basic_cstring_view(str.c_str(), str.size()) {}

    // [cstring.view.iterators], iterator support
    constexpr const_iterator         begin() const noexcept { return data_; }
    constexpr const_iterator         end() const noexcept { return data_ + size_; }
    constexpr const_iterator         cbegin() const noexcept { return begin(); }
    constexpr const_iterator         cend() const noexcept { return end(); }
    constexpr const_reverse_iterator rbegin() const noexcept { return data_ + size_; }
    constexpr const_reverse_iterator rend() const noexcept { return data_; }
    constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    // [cstring.view.capacity], capacity
    constexpr size_type size() const noexcept { return size_; }
    constexpr size_type length() const noexcept { return size_; }
    constexpr size_type max_size() const noexcept { return std::basic_string_view<charT, traits>{}.max_size() - 1; }
    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }

    // [cstring.view.access], element access
    constexpr const_reference operator[](size_type pos) const {
        assert(pos <= size_);
        return data_[pos];
    }
    constexpr const_reference at(size_type pos) const {
        if (pos > size_) {
#if __cpp_lib_format >= 201907L
            throw std::out_of_range(std::format("basic_cstring_view::at: pos ({}) > size() {}", pos, size_));
#else
            throw std::out_of_range(std::string{"basic_cstring_view::at: pos ("} + std::to_string(pos) +
                                    std::string{") > size() "} + std::to_string(size_));
#endif
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
    constexpr const_pointer data() const noexcept { return data_; }
    constexpr const_pointer c_str() const noexcept { return data_; }

    constexpr operator std::basic_string_view<charT, traits>() const noexcept {
        return std::basic_string_view<charT, traits>{data_, size_};
    }

    // [cstring.view.modifiers], modifiers
    constexpr void remove_prefix(size_type n) {
        assert(n <= size());
        data_ += n;
        size_ -= n;
    }
    constexpr void swap(basic_cstring_view& s) noexcept {
        std::swap(data_, s.data_);
        std::swap(size_, s.size_);
    }

    // [cstring.view.ops], cstring operations
    constexpr size_type copy(charT* s, size_type n, size_type pos = 0) const {
        return std::basic_string_view<charT, traits>(*this).copy(s, n, pos);
    }

    constexpr std::basic_string_view<charT, traits> substr(size_type pos, size_type n) const {
        return std::basic_string_view<charT, traits>(*this).substr(pos, n);
    }
    constexpr basic_cstring_view<charT, traits> substr(size_type pos = 0) const { return {data_ + pos, size_ - pos}; }

    constexpr int compare(std::basic_string_view<charT, traits> s) const noexcept {
        return std::basic_string_view<charT, traits>(*this).compare(s);
    }
    constexpr int compare(size_type pos1, size_type n1, basic_cstring_view s) const {
        return std::basic_string_view<charT, traits>(*this).compare(pos1, n1, s);
    }
    constexpr int compare(size_type pos1, size_type n1, basic_cstring_view s, size_type pos2, size_type n2) const {
        return std::basic_string_view<charT, traits>(*this).compare(pos1, n1, s, pos2, n2);
    }
    constexpr int compare(const charT* s) const { return std::basic_string_view<charT, traits>(*this).compare(s); }
    constexpr int compare(size_type pos1, size_type n1, const charT* s) const {
        return std::basic_string_view<charT, traits>(*this).compare(pos1, n1, s);
    }
    constexpr int compare(size_type pos1, size_type n1, const charT* s, size_type n2) const {
        return std::basic_string_view<charT, traits>(*this).compare(pos1, n1, s, n2);
    }

#if __cpp_lib_starts_ends_with >= 201711L
    constexpr bool starts_with(std::basic_string_view<charT, traits> x) const noexcept {
        return std::basic_string_view<charT, traits>(*this).starts_with(x);
    }
    constexpr bool starts_with(charT x) const noexcept {
        return std::basic_string_view<charT, traits>(*this).starts_with(x);
    }
    constexpr bool starts_with(const charT* x) const {
        return std::basic_string_view<charT, traits>(*this).starts_with(x);
    }
    constexpr bool ends_with(std::basic_string_view<charT, traits> x) const noexcept {
        return std::basic_string_view<charT, traits>(*this).ends_with(x);
    }
    constexpr bool ends_with(charT x) const noexcept {
        return std::basic_string_view<charT, traits>(*this).ends_with(x);
    }
    constexpr bool ends_with(const charT* x) const {
        return std::basic_string_view<charT, traits>(*this).ends_with(x);
    }
#endif

    constexpr bool contains(std::basic_string_view<charT, traits> x) const noexcept {
        return std::basic_string_view<charT, traits>(*this).contains(x);
    }
    constexpr bool contains(charT x) const noexcept {
        return std::basic_string_view<charT, traits>(*this).contains(x);
    }
    constexpr bool contains(const charT* x) const { return std::basic_string_view<charT, traits>(*this).contains(x); }

    // [cstring.view.find], searching
    constexpr size_type find(std::basic_string_view<charT, traits> s, size_type pos = 0) const noexcept {
        return std::basic_string_view<charT, traits>(*this).find(s, pos);
    }
    constexpr size_type find(charT c, size_type pos = 0) const noexcept {
        return std::basic_string_view<charT, traits>(*this).find(c, pos);
    }
    constexpr size_type find(const charT* s, size_type pos, size_type n) const {
        return std::basic_string_view<charT, traits>(*this).find(s, pos, n);
    }
    constexpr size_type find(const charT* s, size_type pos = 0) const {
        return std::basic_string_view<charT, traits>(*this).find(s, pos);
    }
    constexpr size_type rfind(std::basic_string_view<charT, traits> s, size_type pos = npos) const noexcept {
        return std::basic_string_view<charT, traits>(*this).rfind(s, pos);
    }
    constexpr size_type rfind(charT c, size_type pos = npos) const noexcept {
        return std::basic_string_view<charT, traits>(*this).rfind(c, pos);
    }
    constexpr size_type rfind(const charT* s, size_type pos, size_type n) const {
        return std::basic_string_view<charT, traits>(*this).rfind(s, pos, n);
    }
    constexpr size_type rfind(const charT* s, size_type pos = npos) const {
        return std::basic_string_view<charT, traits>(*this).rfind(s, pos);
    }

    constexpr size_type find_first_of(std::basic_string_view<charT, traits> s, size_type pos = 0) const noexcept {
        return std::basic_string_view<charT, traits>(*this).find_first_of(s, pos);
    }
    constexpr size_type find_first_of(charT c, size_type pos = 0) const noexcept {
        return std::basic_string_view<charT, traits>(*this).find_first_of(c, pos);
    }
    constexpr size_type find_first_of(const charT* s, size_type pos, size_type n) const {
        return std::basic_string_view<charT, traits>(*this).find_first_of(s, pos, n);
    }
    constexpr size_type find_first_of(const charT* s, size_type pos = 0) const {
        return std::basic_string_view<charT, traits>(*this).find_first_of(s, pos);
    }
    constexpr size_type find_last_of(std::basic_string_view<charT, traits> s, size_type pos = npos) const noexcept {
        return std::basic_string_view<charT, traits>(*this).find_last_of(s, pos);
    }
    constexpr size_type find_last_of(charT c, size_type pos = npos) const noexcept {
        return std::basic_string_view<charT, traits>(*this).find_last_of(c, pos);
    }
    constexpr size_type find_last_of(const charT* s, size_type pos, size_type n) const {
        return std::basic_string_view<charT, traits>(*this).find_last_of(s, pos, n);
    }
    constexpr size_type find_last_of(const charT* s, size_type pos = npos) const {
        return std::basic_string_view<charT, traits>(*this).find_last_of(s, pos);
    }
    constexpr size_type find_first_not_of(std::basic_string_view<charT, traits> s, size_type pos = 0) const noexcept {
        return std::basic_string_view<charT, traits>(*this).find_first_not_of(s, pos);
    }
    constexpr size_type find_first_not_of(charT c, size_type pos = 0) const noexcept {
        return std::basic_string_view<charT, traits>(*this).find_first_not_of(c, pos);
    }
    constexpr size_type find_first_not_of(const charT* s, size_type pos, size_type n) const {
        return std::basic_string_view<charT, traits>(*this).find_first_not_of(s, pos, n);
    }
    constexpr size_type find_first_not_of(const charT* s, size_type pos = 0) const {
        return std::basic_string_view<charT, traits>(*this).find_first_not_of(s, pos);
    }
    constexpr size_type find_last_not_of(std::basic_string_view<charT, traits> s,
                                         size_type                             pos = npos) const noexcept {
        return std::basic_string_view<charT, traits>(*this).find_last_not_of(s, pos);
    }
    constexpr size_type find_last_not_of(charT c, size_type pos = npos) const noexcept {
        return std::basic_string_view<charT, traits>(*this).find_last_not_of(c, pos);
    }
    constexpr size_type find_last_not_of(const charT* s, size_type pos, size_type n) const {
        return std::basic_string_view<charT, traits>(*this).find_last_not_of(s, pos, n);
    }
    constexpr size_type find_last_not_of(const charT* s, size_type pos = npos) const {
        return std::basic_string_view<charT, traits>(*this).find_last_not_of(s, pos);
    }

#if __cpp_lib_three_way_comparison
    friend constexpr bool operator==(basic_cstring_view x, basic_cstring_view y) noexcept {
        return std::basic_string_view<charT, traits>(x) == std::basic_string_view<charT, traits>(y);
    }
    friend constexpr auto operator<=>(basic_cstring_view x, basic_cstring_view y) noexcept {
        return std::basic_string_view<charT, traits>(x) <=> std::basic_string_view<charT, traits>(y);
    }
#else
    friend constexpr bool operator==(basic_cstring_view x, basic_cstring_view y) noexcept {
        return std::basic_string_view<charT, traits>(x) == std::basic_string_view<charT, traits>(y);
    }
    friend constexpr bool operator!=(basic_cstring_view x, basic_cstring_view y) noexcept {
        return std::basic_string_view<charT, traits>(x) != std::basic_string_view<charT, traits>(y);
    }
    friend constexpr bool operator<(basic_cstring_view x, basic_cstring_view y) noexcept {
        return std::basic_string_view<charT, traits>(x) < std::basic_string_view<charT, traits>(y);
    }
    friend constexpr bool operator>(basic_cstring_view x, basic_cstring_view y) noexcept {
        return std::basic_string_view<charT, traits>(x) > std::basic_string_view<charT, traits>(y);
    }
    friend constexpr bool operator<=(basic_cstring_view x, basic_cstring_view y) noexcept {
        return std::basic_string_view<charT, traits>(x) <= std::basic_string_view<charT, traits>(y);
    }
    friend constexpr bool operator>=(basic_cstring_view x, basic_cstring_view y) noexcept {
        return std::basic_string_view<charT, traits>(x) >= std::basic_string_view<charT, traits>(y);
    }
#endif

  private:
    const_pointer data_; // exposition only
    size_type     size_; // exposition only
};

inline namespace literals {
inline namespace cstring_view_literals {
// [cstring.view.literals], suffix for basic_cstring_view literals
constexpr cstring_view operator""_csv(const char* str, size_t len) noexcept { return basic_cstring_view(str, len); }
#if __cpp_char8_t >= 201811L
constexpr u8cstring_view operator""_csv(const char8_t* str, size_t len) noexcept {
    return basic_cstring_view(str, len);
}
#endif
constexpr u16cstring_view operator""_csv(const char16_t* str, size_t len) noexcept {
    return basic_cstring_view(str, len);
}
constexpr u32cstring_view operator""_csv(const char32_t* str, size_t len) noexcept {
    return basic_cstring_view(str, len);
}
constexpr wcstring_view operator""_csv(const wchar_t* str, size_t len) noexcept {
    return basic_cstring_view(str, len);
}
} // namespace cstring_view_literals
} // namespace literals

template <class charT, class traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& os,
                                              basic_cstring_view<charT, traits>  str) {
    return os << std::basic_string_view<charT, traits>(str);
}

} // namespace beman

#if __cpp_lib_format >= 201907L
// [format.formatter.spec]
template <class charT, class traits>
struct std::formatter<beman::basic_cstring_view<charT, traits>, charT> {
    formatter() = default;
    constexpr auto parse(basic_format_parse_context<charT>& context) { return sv_formatter.parse(context); }
    template <typename _Out>
    auto format(beman::basic_cstring_view<charT, traits> csv, basic_format_context<_Out, charT>& context) const {
        return sv_formatter.format(csv, context);
    }

  private:
    formatter<std::basic_string_view<charT, traits>, charT> sv_formatter;
};
#endif

// [cstring.view.hash], hash support
template <>
struct std::hash<beman::cstring_view> {
    auto operator()(const beman::cstring_view& sv) const noexcept { return std::hash<string_view>{}(sv); }
};
#if __cpp_char8_t >= 201811L
template <>
struct std::hash<beman::u8cstring_view> {
    auto operator()(const beman::u8cstring_view& sv) const noexcept { return std::hash<u8string_view>{}(sv); }
};
#endif
template <>
struct std::hash<beman::u16cstring_view> {
    auto operator()(const beman::u16cstring_view& sv) const noexcept { return std::hash<u16string_view>{}(sv); }
};
template <>
struct std::hash<beman::u32cstring_view> {
    auto operator()(const beman::u32cstring_view& sv) const noexcept { return std::hash<u32string_view>{}(sv); }
};
template <>
struct std::hash<beman::wcstring_view> {
    auto operator()(const beman::wcstring_view& sv) const noexcept { return std::hash<wstring_view>{}(sv); }
};

#endif // BEMAN_CSTRING_VIEW_CSTRING_VIEW_HPP
