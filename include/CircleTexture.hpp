#ifndef CIRCLE_TEXTURE_HPP
#define CIRCLE_TEXTURE_HPP

#include <SDL2/SDL.h>

#include <iostream>
#include <cmath>

template <typename T>
class CircleTexture
{
private:
	SDL_Renderer* renderer_;
	SDL_Point center_;
	T radius_;
	SDL_Rect bbox_;
	SDL_Color color_;
	Uint32 pixel_color_;
	Uint32* pixels_;
	SDL_Texture* texture_;

public:
	CircleTexture(SDL_Renderer* renderer, SDL_Point center, T radius, SDL_Color color) : 
	renderer_(renderer), 
	center_(center), 
	radius_(radius), 
	color_(color), 
	pixel_color_(0), 
	texture_(nullptr)
    {
        bbox_.x = center_.x - radius_;
        bbox_.y = center_.y - radius_;
        bbox_.w = 2 * radius_;
        bbox_.h = 2 * radius_;

        #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        pixel_color_ = (color_.b << 24) + (color_.g << 16) + (color_.r << 8) + 255;
        #else
        pixel_color_ = (255 << 24) + (color_.r << 16) + (color_.g << 8) + color_.b;
        #endif

        pixels_ = new Uint32[bbox_.w * bbox_.h];
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, bbox_.w, bbox_.h);
        SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);

        CreateCircleBresenham();
    }

	~CircleTexture()
    {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;

        delete[] pixels_;
        pixels_ = nullptr;
    }

	void CreateCircleBresenham()
    {
        memset(pixels_, 0, bbox_.w * bbox_.h * sizeof(Uint32));

        int x = 0;
        int y = radius_;
        int d = 1 - radius_;

        while (x < y)
        {
            if (d < 0)
            {
                d = d + 2 * x + 3;
                ++x;
            }
            else
            {
                d = d + 2 * (x - y) + 5;
                ++x;
                --y;
            }

            pixels_[((radius_ - y) * bbox_.w) + (radius_ - x)] = pixel_color_;
            pixels_[((radius_ - y) * bbox_.w) + (radius_ - 1 + x)] = pixel_color_;
            pixels_[((radius_ - x) * bbox_.w) + (radius_ - y)] = pixel_color_;
            pixels_[((radius_ - x) * bbox_.w) + (radius_ - 1 + y)] = pixel_color_;
            pixels_[((radius_ - 1 + x) * bbox_.w) + (radius_ - y)] = pixel_color_;
            pixels_[((radius_ - 1 + x) * bbox_.w) + (radius_ - 1 + y)] = pixel_color_;
            pixels_[((radius_ - 1 + y) * bbox_.w) + (radius_ - x)] = pixel_color_;		
            pixels_[((radius_ - 1 + y) * bbox_.w) + (radius_ - 1 + x)] = pixel_color_;
        }

        SDL_UpdateTexture(texture_, nullptr, pixels_, bbox_.w * sizeof(Uint32));
    }

    void MoveTo(const SDL_Point& new_center)
    {
        center_ = new_center;
        bbox_.x = center_.x - radius_;
        bbox_.y = center_.y - radius_;
    }

	void Tick()
    {
    }
    
	void Render(const SDL_Point& center)
    {
        bbox_.x = center.x - radius_;
        bbox_.y = center.y - radius_;

        SDL_SetRenderDrawColor(renderer_, 0xff, 0xff, 0xff, 0xff);
	    SDL_RenderCopy(renderer_, texture_, nullptr, &bbox_);
    }
};

#endif