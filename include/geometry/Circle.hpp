#ifndef CIRCLE_HPP
#define CIRCLE_HPP

#include "Point.hpp"
#include "Rect.hpp"
#include "CircleTexture.hpp"

#include <iostream>
#include <iomanip>
#include <type_traits>
#include <algorithm>
#include <cmath>
#include <memory>

template <typename T>
class Circle : public Shape<T>
{
    static_assert(std::is_arithmetic_v<T>);

public:
    Point<T> center_;
    T radius_;

    Circle(T x = 0, T y = 0, T radius = 0) : center_(x, y), radius_(radius) 
    {
        this->shape_type_ = ShapeType::CIRCLE;
    }

    bool Contains(const Point<T>& point) const override
    {
        return center_.GetDistance(point) <= radius_;
    }

    bool Contains(const Rect<T>& rect) const override
    {
        return Contains(rect.GetTopLeft()) && Contains(rect.GetTopRight()) && Contains(rect.GetBottomLeft()) && Contains(rect.GetBottomRight());
    }

    bool Intersects(const Rect<T>& rect) const override
    {
        const T clamped_x = std::clamp(center_.x_, rect.GetTopLeft().x_, rect.GetTopRight().x_);
        const T clamped_y = std::clamp(center_.y_, rect.GetTopLeft().y_, rect.GetBottomLeft().y_);

        return center_.GetDistance({ clamped_x, clamped_y }) <= radius_;
    }

    void MoveTo(const Point<T>& point_destination) override
    {
        center_.x_ = point_destination.x_;
        center_.y_ = point_destination.y_;
    }

    friend std::ostream& operator<<(std::ostream& os, const Circle<T>& circle)
    {
        os << "Center: " << circle.center_;
        os << "Radius: " << circle.radius_ << '\n';
        return os;
    }
};

#endif
