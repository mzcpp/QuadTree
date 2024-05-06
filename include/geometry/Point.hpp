#ifndef POINT_HPP
#define POINT_HPP

#include <iostream>
#include <type_traits>
#include <iomanip>
#include <cmath>

template <typename T>
class Point
{
    static_assert(std::is_arithmetic_v<T>);

public:
    T x_;
    T y_;

    Point()
    {   
    }

    Point(T x, T y) : x_(x), y_(y)
    {
    }

    double GetDistance(const Point<T>& point) const
    {
        return sqrt(((point.x_ - x_) * (point.x_ - x_)) + ((point.y_ - y_) * (point.y_ - y_)));
    }

    friend Point<T> operator+(const Point<T>& lhs, const Point<T>& rhs)
    {
        Point<T> result = lhs;
        result.x_ += rhs.x_;
        result.y_ += rhs.y_;
        return result;
    }

    friend Point<T> operator-(const Point<T>& lhs, const Point<T>& rhs)
    {
        Point<T> result = lhs;
        result.x_ -= rhs.x_;
        result.y_ -= rhs.y_;
        return result;
    }

    Point<T>& operator+=(const Point<T>& rhs)
    {
        x_ += rhs.x_;
        y_ += rhs.y_;
        return *this;
    }

    Point<T>& operator-=(const Point<T>& rhs)
    {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        return *this;
    }

    friend constexpr bool operator==(const Point<T>& l, const Point<T>& r) noexcept
    {
        return l.x_ == r.x_ && l.y_ == r.y_;
    }
    
    friend constexpr bool operator!=(const Point<T>& l, const Point<T>& r) noexcept
    {
        return !(l == r);
    }

    friend std::ostream& operator<<(std::ostream& os, const Point<T>& point)
    {
        os << '[' << point.x_ << ", " << point.y_ << ']' << '\n';
        return os;
    }
};

#endif
