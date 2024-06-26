#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include "geometry/Point.hpp"
#include "geometry/Shape.hpp"
#include "geometry/Rect.hpp"
#include "geometry/Circle.hpp"

#include <list>
#include <iostream>
#include <iomanip>
#include <memory>
#include <array>
#include <vector>
#include <cassert>
#include <type_traits>
#include <algorithm>
#include <cmath>

// template <typename T>
// struct extract_value_type
// {
//     typedef T value_type;
// };

// template <template <typename> class X, typename T>
// struct extract_value_type<X<T>>
// {
//     typedef T value_type;
// };

// template <typename T>
// struct innermost_impl
// {
//     using type = T;
// };

// template <template <typename...> class E, typename Head, typename... Tail>
// struct innermost_impl<E<Head, Tail...>>
// {
//     using type = typename innermost_impl<Head>::type;
// };

// template <typename T>
// using innermost = typename innermost_impl<T>::type;

template <typename T>
class QuadTree
{
private:
    class Node;

public:
    struct QuadTreeItem;

    // typedef innermost<T> NumType;
    // static_assert(std::is_arithmetic_v<NumType>);
    
    typedef typename std::list<QuadTreeItem>::iterator QuadTreeItemListIt;

    struct QuadTreeItem
    {
        T item_;
        Rect<float> bbox_;
        Node* node_;
        typename std::list<QuadTreeItemListIt>::iterator node_list_it_;
    };

private:
    class Node
    {
    public:
        Node* parent_;
        std::size_t depth_;
        Rect<float> area_;
        std::array<Rect<float>, 4> children_areas_;
        std::array<std::unique_ptr<Node>, 4> children_;
        std::list<QuadTreeItemListIt> qt_items_its_;

        Node(Node* parent, std::size_t depth, const Rect<float>& area) : parent_(parent), depth_(depth), area_(area)
        {
            children_ = { nullptr, nullptr, nullptr, nullptr };
        }

        void CalculateChildrenAreas()
        {
            const float ax = area_.top_left_.x_;
            const float ay = area_.top_left_.y_;

            const float half_w = static_cast<float>(area_.width_ / 2.0f);
            const float half_h = static_cast<float>(area_.height_ / 2.0f);
            
            children_areas_[0] = Rect<float>(ax, ay, half_w, half_h);
            children_areas_[1] = Rect<float>(ax + half_w, ay, half_w, half_h);
            children_areas_[2] = Rect<float>(ax, ay + half_h, half_w, half_h);
            children_areas_[3] = Rect<float>(ax + half_w, ay + half_h, half_w, half_h);
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
                            const float ax = area_.top_left_.x_;
                            const float ay = area_.top_left_.y_;

                            const float half_w = static_cast<float>(area_.width_ / 2.0f);
                            const float half_h = static_cast<float>(area_.height_ / 2.0f);

                            Rect<float> node_area;
                            
                            switch (i)
                            {
                                case 0:
                                    node_area = Rect<float>(ax, ay, half_w, half_h);
                                    break;
                                case 1:
                                    node_area = Rect<float>(ax + half_w, ay, half_w, half_h);
                                    break;
                                case 2:
                                    node_area = Rect<float>(ax, ay + half_h, half_w, half_h);
                                    break;
                                case 3:
                                    node_area = Rect<float>(ax + half_w, ay + half_h, half_w, half_h);
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

        void Search(const std::unique_ptr<Shape<float>>& area_to_search, std::list<QuadTreeItemListIt>* out_items_list)
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

        void GetAreas(std::vector<Rect<float>>* out_areas)
        {
            const bool all_children_empty = std::all_of(children_.begin(), children_.end(), [](const std::unique_ptr<Node>& child_ptr)
                {
                    return child_ptr == nullptr;
                });

            if (all_children_empty)
            {
                return;
            }

            std::for_each(children_areas_.begin(), children_areas_.end(), [out_areas](const Rect<float>& child_area)
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
    QuadTree(const Rect<float>& area, const std::size_t max_depth) : max_depth_(max_depth)
    {
        root_ = std::make_unique<Node>(nullptr, 0, area);
        root_->CalculateChildrenAreas();
    }

    void Resize(const Rect<float>& area)
    {
        Reset();
        root_->area_ = area;
        root_->CalculateChildrenAreas();
    }

    void Reset()
    {
        root_->qt_items_its_.clear();
        const Rect<float> root_area = root_->area_;
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

    void Insert(const T& item, const Rect<float>& item_bbox)
    {
        if (item_bbox.top_left_.x_ < 0 || item_bbox.top_left_.x_ > (root_->area_.top_left_.x_ + root_->area_.width_) || 
        item_bbox.top_left_.y_ < 0 || item_bbox.top_left_.y_ > (root_->area_.top_left_.y_ + root_->area_.height_))
        {
            printf("%s%f%s%f%s\n", "Failed to insert! Position: x { ", item_bbox.top_left_.x_, " } y { ", item_bbox.top_left_.y_, " } is out of bounds!");
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

    void Relocate(const QuadTreeItemListIt& qt_item_it, const Rect<float>& new_area)
    {
        const Rect<float>& old_area = qt_item_it->bbox_;
        const std::array<Rect<float>, 4>& children_areas = qt_item_it->node_->children_areas_;
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

    std::list<QuadTreeItemListIt> Search(const std::unique_ptr<Shape<float>>& area_to_search) const
    {
        std::list<QuadTreeItemListIt> items_list;
        root_->Search(area_to_search, &items_list);
        return items_list;
    }

    std::vector<Rect<float>> GetAreas()
    {
        std::vector<Rect<float>> areas;
        root_->GetAreas(&areas);
        return areas;
    }

    std::list<QuadTreeItem>& GetItems()
    {
        return items_;
    }
};

#endif
