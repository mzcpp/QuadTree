#ifndef SHAPE_HPP
#define SHAPE_HPP

template <typename T>
class Point;

template <typename T>
class Rect;

enum class ShapeType { RECT, CIRCLE };

template <typename T>
class Shape
{
public:
    ShapeType shape_type_;

    virtual ~Shape() {}
    virtual bool Contains(const Point<T>& point) const = 0;
    virtual bool Contains(const Rect<T>& rect) const = 0;
    virtual bool Intersects(const Rect<T>& rect) const = 0;
    virtual void MoveTo(const Point<T>& point_destination) = 0;
};

#endif
