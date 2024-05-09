#include "Game.hpp"
#include "Constants.hpp"
#include "QuadTree.hpp"
#include "CircleTexture.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cstdint>
#include <iostream>
#include <memory>

Game::Game() : 
	initialized_(false), 
	running_(false), 
	mouse_moved_(false), 
	left_shift_pressed_(false), 
	debug_areas_(true), 
	qt_(nullptr), 
	shape_area_(nullptr), 
	circle_r_(40), 
	rect_side_(80), 
	searching_(false),
	removing_(false), 
	circle_texture_(nullptr)
{
	initialized_ = Initialize();

	int w = 0;
	int h = 0;
	SDL_GetWindowSize(window_, &w, &h);

	const Rect<float> area = { static_cast<float>(0), static_cast<float>(0), static_cast<float>(w), static_cast<float>(h) };
	constexpr std::size_t max_depth = 6;

	qt_ = std::make_unique<QuadTree<QtItemType>>(area, max_depth);

	const SDL_Color color = { 0x00, 0x00, 0xff, 0xff };
	const SDL_Point center = { 0, 0 };
	circle_texture_ = std::make_unique<CircleTexture>(renderer_, center, circle_r_, color);
}

Game::~Game()
{
	Finalize();
}

bool Game::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not be initialized! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"))
	{
		printf("%s\n", "Warning: Texture filtering is not enabled!");
	}

	window_ = SDL_CreateWindow(constants::game_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, constants::screen_width, constants::screen_height, SDL_WINDOW_SHOWN);

	if (window_ == nullptr)
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);

	if (renderer_ == nullptr)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	constexpr int img_flags = IMG_INIT_PNG;

	if (!(IMG_Init(img_flags) & img_flags))
	{
		printf("SDL_image could not be initialized! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	return true;
}

void Game::Finalize()
{
	SDL_DestroyWindow(window_);
	window_ = nullptr;
	
	SDL_DestroyRenderer(renderer_);
	renderer_ = nullptr;

	SDL_Quit();
	IMG_Quit();
}

void Game::Run()
{
	if (!initialized_)
	{
		printf("%s\n", "Game has not been initialized!");
		return;
	}

	running_ = true;

	constexpr double ms = 1.0 / 60.0;
	std::uint64_t last_time = SDL_GetPerformanceCounter();
	long double delta = 0.0;

	double timer = SDL_GetTicks();

	int frames = 0;
	int ticks = 0;

	while (running_)
	{
		const std::uint64_t now = SDL_GetPerformanceCounter();
		const long double elapsed = static_cast<long double>(now - last_time) / static_cast<long double>(SDL_GetPerformanceFrequency());

		last_time = now;
		delta += elapsed;

		HandleEvents();

		while (delta >= ms)
		{
			Tick();
			delta -= ms;
			++ticks;
		}

		//printf("%Lf\n", delta / ms);
		Render();
		++frames;

		if (SDL_GetTicks() - timer > 1000.0)
		{
			timer += 1000.0;
			// printf("Frames: %d, Ticks: %d\n", frames, ticks);
			frames = 0;
			ticks = 0;
		}
	}
}

void Game::HandleEvents()
{
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
		{
			running_ = false;
			return;
		}
		if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
		{
			SDL_GetMouseState(&mouse_pos_.x, &mouse_pos_.y);

			// float px = static_cast<float>(mouse_pos_.x);
			// float py = static_cast<float>(mouse_pos_.y);
			// float pw = static_cast<float>(1);
			// float ph = static_cast<float>(1);

			// QtItemType p = { px, py };
			// Rect<float> p_area = { px, py, pw, ph };

			// qt_->Insert(p, p_area);

			float px = static_cast<float>(mouse_pos_.x);
			float py = static_cast<float>(mouse_pos_.y);
			float pw = static_cast<float>(30);
			float ph = static_cast<float>(30);

			QtItemType p = { px - (pw / 2), py - (ph / 2), pw, ph };
			Rect<float> p_area = { px - (pw / 2), py - (ph / 2), pw, ph };

			qt_->Insert(p, p_area);
		}

		if (e.type == SDL_KEYDOWN)
		{
			if ((e.key.keysym.sym == SDLK_s || e.key.keysym.sym == SDLK_r) && e.key.repeat == 0)
			{
				SDL_GetMouseState(&mouse_pos_.x, &mouse_pos_.y);
				
				if (left_shift_pressed_)
				{
					shape_area_ = std::make_unique<Circle<float>>(mouse_pos_.x, mouse_pos_.y, circle_r_);
				}
				else
				{
					shape_area_ = std::make_unique<Rect<float>>(mouse_pos_.x - circle_r_, mouse_pos_.y - circle_r_, rect_side_, rect_side_);
				}

				searching_ = e.key.keysym.sym == SDLK_s;
				removing_ = e.key.keysym.sym == SDLK_r;
			}
			
			if (e.key.keysym.sym == SDLK_LSHIFT)
			{
				if (shape_area_ != nullptr && shape_area_->shape_type_ != ShapeType::CIRCLE)
				{
					shape_area_ = std::make_unique<Circle<float>>(mouse_pos_.x, mouse_pos_.y, circle_r_);
				}

				left_shift_pressed_ = true;
			}
			
			if (e.key.keysym.sym == SDLK_d)
			{
				debug_areas_ = !debug_areas_;
			}

			if (e.key.keysym.sym == SDLK_x)
			{
				qt_->Reset();
			}
		}
		else if (e.type == SDL_KEYUP)
		{
			if (e.key.keysym.sym == SDLK_s || e.key.keysym.sym == SDLK_r)
			{
				shape_area_.reset(nullptr);
				searching_ = e.key.keysym.sym == SDLK_s ? false : searching_;
				removing_ = e.key.keysym.sym == SDLK_r ? false : searching_;
			}

			if (e.key.keysym.sym == SDLK_LSHIFT)
			{
				if (shape_area_ != nullptr && shape_area_->shape_type_ == ShapeType::CIRCLE)
				{
					shape_area_ = std::make_unique<Rect<float>>(mouse_pos_.x - circle_r_, mouse_pos_.y - circle_r_, rect_side_, rect_side_);
				}

				left_shift_pressed_ = false;
			}
		}
		
		if (e.type == SDL_MOUSEMOTION)
		{
			mouse_moved_ = true;
		}
	}
}

void Game::Tick()
{
	if (shape_area_ != nullptr)
	{
		if (mouse_moved_)
		{
			SDL_GetMouseState(&mouse_pos_.x, &mouse_pos_.y);
			shape_area_->MoveTo( { static_cast<float>(mouse_pos_.x), static_cast<float>(mouse_pos_.y) } );
			mouse_moved_ = false;
		}

		if (searching_ || removing_)
		{
			found_items_ = qt_->Search(shape_area_);

			if (removing_)
			{
				for (auto& qt_item_it : found_items_)
				{
					qt_->Remove(qt_item_it);
					
					if (qt_item_it->node_->qt_items_its_.empty())
					{
						qt_->CleanUp();
					}
				}
			}
		}
	}
}

void Game::Render()
{
	SDL_RenderSetViewport(renderer_, NULL);
	SDL_SetRenderDrawColor(renderer_, 0xff, 0xff, 0xff, 0xff);
	SDL_RenderClear(renderer_);

	if (debug_areas_)
	{
		for (const auto& area : qt_->GetAreas())
		{
			SDL_SetRenderDrawColor(renderer_, 0xff, 0x00, 0x00, 0xff);
			SDL_FRect rect = { area.top_left_.x_, area.top_left_.y_, area.width_, area.width_ };
			SDL_RenderDrawRectF(renderer_, &rect);
		}
	}

	for (const auto& qt_item : qt_->GetItems())
	{
		SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0xff);
		SDL_FRect r = { qt_item.item_.top_left_.x_, qt_item.item_.top_left_.y_, qt_item.item_.width_, qt_item.item_.height_ };
		SDL_RenderFillRectF(renderer_, &r);
		//SDL_RenderDrawPoint(renderer_, qt_item.item_.x_, qt_item.item_.y_);
	}

	if (searching_ || removing_)
	{
		SDL_GetMouseState(&mouse_pos_.x, &mouse_pos_.y);

		if (searching_)
		{
			for (const auto& qt_item_it : found_items_)
			{
				SDL_SetRenderDrawColor(renderer_, 0x00, 0xff, 0x00, 0xff);
				SDL_FRect r = { qt_item_it->item_.top_left_.x_, qt_item_it->item_.top_left_.y_, qt_item_it->item_.width_, qt_item_it->item_.height_ };
				SDL_RenderFillRectF(renderer_, &r);
				// SDL_RenderDrawPoint(renderer_, qt_item_it->item_.x_, qt_item_it->item_.y_);
			}
		}

		if (left_shift_pressed_)
		{
			circle_texture_->Render(mouse_pos_);
		}
		else
		{
			SDL_FRect rect = { mouse_pos_.x - circle_r_, mouse_pos_.y - circle_r_, rect_side_, rect_side_ };
			SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0xff, 0xff);
			SDL_RenderDrawRectF(renderer_, &rect);
		}
	}

	SDL_RenderPresent(renderer_);
}
