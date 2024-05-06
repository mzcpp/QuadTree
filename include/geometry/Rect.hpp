#ifndef RECT_HPP
#define RECT_HPP

#include <iostream>
#include <type_traits>
#include <iomanip>
#include <cmath>

template <typename T>
class Point;

template <typename T>
class Rect : public Shape<T>
{
    static_assert(std::is_arithmetic_v<T>);

public:
    Point<T> top_left_;
    T width_;
    T height_;

    Rect(T x = 0, T y = 0, T width = 0, T height = 0) noexcept : width_(width), height_(height) 
    {
        this->shape_type_ = ShapeType::RECT;
        top_left_.x_ = x;
        top_left_.y_ = y;
    }

    Point<T> GetTopLeft() const
    {
        return top_left_;
    }

    Point<T> GetTopRight() const
    {
        return { top_left_.x_ + width_, top_left_.y_ };
    }

    Point<T> GetBottomRight() const
    {
        return { top_left_.x_ + width_, top_left_.y_ + height_ };
    }

    Point<T> GetBottomLeft() const
    {
        return { top_left_.x_, top_left_.y_ + height_ };
    }

    bool Contains(const Point<T>& point) const override
    {
        return point.x_ >= top_left_.x_ && point.y_ >= top_left_.y_ && point.x_ < GetBottomRight().x_ && point.y_ < GetBottomRight().y_;
    }

    bool Contains(const Rect<T>& rect) const override
    {
        return rect.top_left_.x_ >= top_left_.x_ && rect.top_left_.y_ >= top_left_.y_ && 
            rect.GetBottomRight().x_ < GetBottomRight().x_ && rect.GetBottomRight().y_ < GetBottomRight().y_;
    }

    bool Intersects(const Rect<T>& rect) const override
    {
        return top_left_.x_ < rect.top_left_.x_ + rect.width_ && top_left_.x_ + width_ >= rect.top_left_.x_ && 
            top_left_.y_ < rect.top_left_.y_ + rect.height_ && top_left_.y_ + height_ >= rect.top_left_.y_;
    }

    void MoveTo(const Point<T>& point_destination) override
    {
        top_left_.x_ = point_destination.x_ - (width_ / 2.0);
        top_left_.y_ = point_destination.y_ - (height_ / 2.0);
    }

    friend std::ostream& operator<<(std::ostream& os, const Rect<T>& rect)
    {
        os << "Top Left: " << std::setw(1) << rect.top_left_;
        os << "Width: " << std::setw(8) << rect.width_ << '\n';
        os << "Height: " << std::setw(7) << rect.height_ << '\n';
        return os;
    }
};

#endif
