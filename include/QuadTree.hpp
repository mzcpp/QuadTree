#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include <list>
#include <iostream>
#include <iomanip>
#include <memory>
#include <array>
#include <vector>
#include <stack>
#include <list>
#include <cassert>
#include <type_traits>
#include <algorithm>
#include <cmath>

template <typename T>
struct Point
{
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
    virtual void MoveTo(const SDL_Point& point_destination) = 0;
};

template <typename T>
class Rect : public Shape<T>
{
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

    void MoveTo(const SDL_Point& point_destination) override
    {
        top_left_.x_ = point_destination.x - (width_ / 2.0);
        top_left_.y_ = point_destination.y - (height_ / 2.0);
    }

    friend std::ostream& operator<<(std::ostream& os, const Rect<T>& rect)
    {
        os << "Top Left: " << std::setw(1) << rect.top_left_;
        os << "Width: " << std::setw(8) << rect.width_ << '\n';
        os << "Height: " << std::setw(7) << rect.height_ << '\n';
        return os;
    }
};

template <typename T>
class Circle : public Shape<T>
{
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

    void MoveTo(const SDL_Point& point_destination) override
    {
        center_.x_ = point_destination.x;
        center_.y_ = point_destination.y;
    }

    friend std::ostream& operator<<(std::ostream& os, const Circle<T>& circle)
    {
        os << "Center: " << circle.center_;
        os << "Radius: " << circle.radius_ << '\n';
        return os;
    }
};

template <typename T>
struct extract_value_type
{
    typedef T value_type;
};

template <template <typename> class X, typename T>
struct extract_value_type<X<T>>
{
    typedef T value_type;
};

template <typename T>
class QuadTree
{
private:
    class Node;

public:
    struct QuadTreeItem;
    typedef typename extract_value_type<T>::value_type NumType;
    typedef typename std::list<QuadTreeItem>::iterator QuadTreeItemListIt;

    static_assert(std::is_arithmetic_v<NumType>);

    struct QuadTreeItem
    {
        T item_;
        Rect<NumType> bbox_;
        Node* node_;
        typename std::list<QuadTreeItemListIt>::iterator node_list_it_;
    };

private:
    class Node
    {
    public:
        Node* parent_;
        std::size_t depth_;
        Rect<NumType> area_;
        std::array<Rect<NumType>, 4> children_areas_;
        std::array<std::unique_ptr<Node>, 4> children_;
        std::list<QuadTreeItemListIt> qt_items_its_;

        Node(Node* parent, std::size_t depth, const Rect<NumType>& area) : parent_(parent), depth_(depth), area_(area)
        {
            children_ = { nullptr, nullptr, nullptr, nullptr };
        }

        void CalculateChildrenAreas()
        {
            const NumType ax = area_.top_left_.x_;
            const NumType ay = area_.top_left_.y_;

            const NumType half_w = static_cast<NumType>(area_.width_ / 2.0);
            const NumType half_h = static_cast<NumType>(area_.height_ / 2.0);
            
            children_areas_[0] = Rect<NumType>(ax, ay, half_w, half_h);
            children_areas_[1] = Rect<NumType>(ax + half_w, ay, half_w, half_h);
            children_areas_[2] = Rect<NumType>(ax, ay + half_h, half_w, half_h);
            children_areas_[3] = Rect<NumType>(ax + half_w, ay + half_h, half_w, half_h);
        }

        void Insert(const QuadTreeItemListIt& qt_item_it, std::size_t max_depth)
        {
            for (std::size_t i = 0; i < 4; ++i)
            {
                if (children_areas_[i].Contains(qt_item_it->bbox_))
                {
                    if (depth_ + 1 <= max_depth)
                    {
                        if (children_[i] == nullptr)
                        {
                            const NumType ax = area_.top_left_.x_;
                            const NumType ay = area_.top_left_.y_;

                            const NumType half_w = static_cast<NumType>(area_.width_ / 2.0);
                            const NumType half_h = static_cast<NumType>(area_.height_ / 2.0);

                            Rect<NumType> node_area;
                            
                            switch (i)
                            {
                                case 0:
                                    node_area = Rect<NumType>(ax, ay, half_w, half_h);
                                    break;
                                case 1:
                                    node_area = Rect<NumType>(ax + half_w, ay, half_w, half_h);
                                    break;
                                case 2:
                                    node_area = Rect<NumType>(ax, ay + half_h, half_w, half_h);
                                    break;
                                case 3:
                                    node_area = Rect<NumType>(ax + half_w, ay + half_h, half_w, half_h);
                                    break;
                                default:
                                    assert(false);
                            }

                            children_[i] = std::make_unique<Node>(this, depth_ + 1, node_area);
                            children_[i]->CalculateChildrenAreas();
                        }

                        return children_[i]->Insert(qt_item_it, max_depth);
                    }
                }
            }
            
            qt_item_it->node_ = this;
            qt_items_its_.push_back({ qt_item_it });
            qt_item_it->node_list_it_ = std::prev(std::end(qt_items_its_));
        }

        void AddItems(std::list<QuadTreeItemListIt>* out_items_list)
        {
            if (out_items_list == nullptr)
            {
                return;
            }

            out_items_list->insert(std::end(*out_items_list), std::begin(qt_items_its_), std::end(qt_items_its_));

            std::for_each(children_.begin(), children_.end(), [out_items_list](const std::unique_ptr<Node>& child_ptr)
                {
                    if (child_ptr != nullptr)
                    {
                        child_ptr->AddItems(out_items_list);
                    }
                });
        }

        void Search(const std::unique_ptr<Shape<NumType>>& area_to_search, std::list<QuadTreeItemListIt>* out_items_list)
        {
            if (area_to_search == nullptr || out_items_list == nullptr)
            {
                return;
            }
            
            std::for_each(qt_items_its_.begin(), qt_items_its_.end(), [&area_to_search, out_items_list](const QuadTreeItemListIt& qt_item_it)
                {
                    if (area_to_search->Intersects(qt_item_it->bbox_))
                    {
                        out_items_list->push_back(qt_item_it);
                    }
                });

            for (std::size_t i = 0; i < 4; ++i)
            {
                if (children_[i] != nullptr)
                {
                    if (area_to_search->Contains(children_areas_[i]))
                    {
                        children_[i]->AddItems(out_items_list);
                    }
                    else if (area_to_search->Intersects(children_areas_[i]))
                    {
                        children_[i]->Search(area_to_search, out_items_list);
                    }
                }
            }
        }

        void GetAreas(std::vector<Rect<NumType>>* out_areas)
        {
            const bool all_children_empty = std::all_of(children_.begin(), children_.end(), [](const std::unique_ptr<Node>& child_ptr)
                {
                    return child_ptr == nullptr;
                });

            if (all_children_empty)
            {
                return;
            }

            std::for_each(children_areas_.begin(), children_areas_.end(), [out_areas](const Rect<NumType>& child_area)
            {
                out_areas->push_back(child_area);
            });

            std::for_each(children_.begin(), children_.end(), [out_areas](const std::unique_ptr<Node>& child_ptr)
                {
                    if (child_ptr != nullptr)
                    {
                        child_ptr->GetAreas(out_areas);
                    }
                });
        }

        bool CleanUp()
        {
            const bool northwest = children_[0] == nullptr || children_[0]->CleanUp();
            const bool northeast = children_[1] == nullptr || children_[1]->CleanUp();
            const bool southwest = children_[2] == nullptr || children_[2]->CleanUp();
            const bool southeast = children_[3] == nullptr || children_[3]->CleanUp();

            if (northwest)
            {
                children_[0].reset(nullptr);
            }

            if (northeast)
            {
                children_[1].reset(nullptr);
            }

            if (southwest)
            {
                children_[2].reset(nullptr);
            }

            if (southeast)
            {
                children_[3].reset(nullptr);
            }

            return northwest && northeast && southwest && southeast && qt_items_its_.empty();
        }
    };

    std::size_t max_depth_;
    std::unique_ptr<Node> root_;
    std::list<QuadTreeItem> items_;

public:
    QuadTree(const Rect<NumType>& area, const std::size_t max_depth) : max_depth_(max_depth)
    {
        root_ = std::make_unique<Node>(nullptr, 0, area);
        root_->CalculateChildrenAreas();
    }

    void Resize(const Rect<NumType>& area)
    {
        Reset();
        root_->area_ = area;
        root_->CalculateChildrenAreas();
    }

    void Reset()
    {
        root_->qt_items_its_.clear();
        const Rect<NumType> root_area = root_->area_;
        root_.reset(nullptr);
        items_.clear();

        root_ = std::make_unique<Node>(nullptr, 0, root_area);
        root_->CalculateChildrenAreas();
    }

    std::size_t Size()
    {
        return items_.size();
    }

    bool Empty()
    {
        return items_.empty();
    }

    void Insert(const T& item, const Rect<NumType>& item_bbox)
    {
        if (item_bbox.top_left_.x_ < 0 || item_bbox.top_left_.x_ > (root_->area_.top_left_.x_ + root_->area_.width_) || 
        item_bbox.top_left_.y_ < 0 || item_bbox.top_left_.y_ > (root_->area_.top_left_.y_ + root_->area_.height_))
        {
            printf("%s%d%s%d%s\n", "Failed to insert! Position: x { ", item_bbox.top_left_.x_, " } y { ", item_bbox.top_left_.y_, " } is out of bounds!");
        }

        QuadTreeItem qt_item;
        qt_item.item_ = item;
        qt_item.bbox_ = item_bbox;
        items_.push_back(qt_item);
        root_->Insert(std::prev(std::end(items_)), max_depth_);
    }

    void Remove(const QuadTreeItemListIt& qt_item_it)
    {
        qt_item_it->node_->qt_items_its_.erase(qt_item_it->node_list_it_);
        items_.erase(qt_item_it);
    }

    void Relocate(const QuadTreeItemListIt& qt_item_it, const Rect<NumType>& new_area)
    {
        const Rect<NumType>& old_area = qt_item_it->bbox_;
        const std::array<Rect<NumType>, 4>& children_areas = qt_item_it->node_->children_areas_;
        const bool area_contains = qt_item_it->node_->area_.Contains(old_area) == qt_item_it->node_->area_.Contains(new_area);
        const bool nw_contains = children_areas[0].Contains(old_area) == children_areas[0].Contains(new_area);
        const bool ne_contains = children_areas[1].Contains(old_area) == children_areas[1].Contains(new_area);
        const bool sw_contains = children_areas[2].Contains(old_area) == children_areas[2].Contains(new_area);
        const bool se_contains = children_areas[3].Contains(old_area) == children_areas[3].Contains(new_area);

        if (area_contains && (qt_item_it->node_->depth_ + 1 > max_depth_ || (nw_contains && ne_contains && sw_contains && se_contains)))
        {
            return;
        }

        qt_item_it->node_->qt_items_its_.erase(qt_item_it->node_list_it_);
        
        if (qt_item_it->node_->qt_items_its_.empty())
        {
            CleanUp();
        }
        
        qt_item_it->bbox_ = new_area;
        root_->Insert(qt_item_it, max_depth_);
    }

    void CleanUp()
    {
        root_->CleanUp();
    }

    std::list<QuadTreeItemListIt> Search(const std::unique_ptr<Shape<NumType>>& area_to_search) const
    {
        std::list<QuadTreeItemListIt> items_list;
        root_->Search(area_to_search, &items_list);
        return items_list;
    }

    std::vector<Rect<NumType>> GetAreas()
    {
        std::vector<Rect<NumType>> areas;
        root_->GetAreas(&areas);
        return areas;
    }

    std::list<QuadTreeItem>& GetItems()
    {
        return items_;
    }
};

#endif
