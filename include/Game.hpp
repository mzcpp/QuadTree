#ifndef GAME_HPP
#define GAME_HPP

#include "QuadTree.hpp"

#include <SDL2/SDL.h>

#include <memory>
#include <list>

template <typename T>
class Shape;

template <typename T>
class Rect;

class CircleTexture;

typedef int NumType;
typedef Rect<NumType> QtItemType;

class Game
{
private:
	bool initialized_;
	bool running_;

	SDL_Point mouse_pos_;
	bool mouse_moved_;
	bool left_shift_pressed_;
	bool debug_areas_;
	std::unique_ptr<QuadTree<QtItemType>> qt_;
	std::unique_ptr<Shape<NumType>> shape_area_;
	std::list<QuadTree<QtItemType>::QuadTreeItemListIt> found_items_;

	int circle_r_;
	int rect_side_;

	bool searching_;
	bool removing_;
	
	std::list<std::list<QtItemType>::iterator> found_;
	std::unique_ptr<CircleTexture> circle_texture_;

	SDL_Window* window_;
	SDL_Renderer* renderer_;

public:
	Game();

	~Game();

	bool Initialize();

	void Finalize();

	void Run();

	void HandleEvents();
	
	void Tick();
	
	void Render();
};

#endif