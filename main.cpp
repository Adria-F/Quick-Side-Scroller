// ----------------------------------------------------------------
// Quick Side Scroller (https://github.com/d0n3val/Quick-Side-Scroller)
// Simplistic side scroller made with SDL for educational purposes.
//
// Installation
// Project files are made for VS 2015. Download the code, compile it. There is no formal installation process.
//
// Credits
// Ricard Pillosu
//
// License
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non - commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain.We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors.We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>
// 
// ----------------------------------------------------------------

#include "SDL\include\SDL.h"
#include "SDL_image\include\SDL_image.h"
#include "SDL_mixer\include\SDL_mixer.h"
#include <iostream>
#include <time.h>

#pragma comment( lib, "SDL/libx86/SDL2.lib" )
#pragma comment( lib, "SDL/libx86/SDL2main.lib" )
#pragma comment( lib, "SDL_image/libx86/SDL2_image.lib" )
#pragma comment( lib, "SDL_mixer/libx86/SDL2_mixer.lib" )

// Globals --------------------------------------------------------
#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 480
#define SCROLL_SPEED 5
#define SHIP_SPEED 3
#define NUM_SHOTS 15
#define SHOT_SPEED 5
#define Spawn_Delay 0.5
#define Max_Enemies 3
#define Enemy_Speed 5
#define Max_Lives 3

struct projectile
{
	int x, y;
	bool alive;
};

struct globals
{
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* background = nullptr;
	SDL_Texture* ship = nullptr;
	SDL_Texture* shot = nullptr;
	SDL_Texture* sprites_texture = nullptr;
	SDL_Texture* texture_lives = nullptr;
	SDL_Texture* gameover = nullptr;
	SDL_Texture* ammo = nullptr;
	SDL_Texture* tutorial = nullptr;
	SDL_Rect bullet;
	int background_width = 0;
	int ship_x = 0;
	int ship_y = 0;
	int last_shot = 0;
	bool fire, up, down, left, right;
	Mix_Music* music = nullptr;
	Mix_Chunk* fx_shoot = nullptr;
	int scroll = 0;
	projectile shots[NUM_SHOTS];
	float delay = 0;
	float expl_delay = 0;
	char c_points[7];
	int points = 0;
	char score[43] = "QSS - Quick Side Scroller - Points: ";
	int lives = Max_Lives;
	bool dead = false;
	int enemy_counter = 0;
	int enemy_lives[Max_Enemies];
	int enemy_type[Max_Enemies];
	bool explode[Max_Enemies];
	bool expl = false;
	bool start = false;
} g; // automatically create an insteance called "g"

// ----------------------------------------------------------------
void Start()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	// Create window & renderer
	g.window = SDL_CreateWindow("QSS - Quick Side Scroller - Points: 0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	g.renderer = SDL_CreateRenderer(g.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	// Load image lib --
	IMG_Init(IMG_INIT_PNG);
	g.background = SDL_CreateTextureFromSurface(g.renderer, IMG_Load("assets/background.png"));
	g.ship = SDL_CreateTextureFromSurface(g.renderer, IMG_Load("assets/ship.png"));
	g.shot = SDL_CreateTextureFromSurface(g.renderer, IMG_Load("assets/shot.png"));
	g.sprites_texture = SDL_CreateTextureFromSurface(g.renderer, IMG_Load("assets/sprites.png"));
	g.texture_lives = SDL_CreateTextureFromSurface(g.renderer, IMG_Load("assets/heart.png"));
	g.gameover = SDL_CreateTextureFromSurface(g.renderer, IMG_Load("assets/gameover.png"));
	g.ammo = SDL_CreateTextureFromSurface(g.renderer, IMG_Load("assets/ammo.png"));
	g.tutorial = SDL_CreateTextureFromSurface(g.renderer, IMG_Load("assets/tutorial.png"));
	SDL_QueryTexture(g.background, nullptr, nullptr, &g.background_width, nullptr);

	// Create mixer --
	Mix_Init(MIX_INIT_OGG);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	g.music = Mix_LoadMUS("assets/music.ogg");
	Mix_PlayMusic(g.music, -1);
	g.fx_shoot = Mix_LoadWAV("assets/laser.wav");

	// Init other vars --
	g.ship_x = 100;
	g.ship_y = SCREEN_HEIGHT / 2;
	g.fire = g.up = g.down = g.left = g.right = false;

	for (int i = 0; i < Max_Enemies; i++)
	{
		g.enemy_lives[i] = 1;
		g.enemy_type[i] = 1;
		g.explode[i] = false;
	}
}

// ----------------------------------------------------------------
void Finish()
{
	Mix_FreeMusic(g.music);
	Mix_FreeChunk(g.fx_shoot);
	Mix_CloseAudio();
	Mix_Quit();
	SDL_DestroyTexture(g.shot);
	SDL_DestroyTexture(g.ship);
	SDL_DestroyTexture(g.background);
	IMG_Quit();
	SDL_DestroyRenderer(g.renderer);
	SDL_DestroyWindow(g.window);
	SDL_Quit();
}

// ----------------------------------------------------------------
bool CheckInput()
{
	bool ret = true;
	SDL_Event event;

	while(SDL_PollEvent(&event) != 0)
	{
		if(event.type == SDL_KEYUP)
		{
			switch(event.key.keysym.sym)
			{
				case SDLK_UP: g.up = false;	break;
				case SDLK_DOWN:	g.down = false;	break;
				case SDLK_LEFT:	g.left = false;	break;
				case SDLK_RIGHT: g.right = false; break;
			}
		}
		else if(event.type == SDL_KEYDOWN)
		{
			switch(event.key.keysym.sym)
			{
				case SDLK_UP: g.up = true; break;
				case SDLK_DOWN: g.down = true; break;
				case SDLK_LEFT: g.left = true; break;
				case SDLK_RIGHT: g.right = true; break;
				case SDLK_ESCAPE: ret = false; break;
				case SDLK_SPACE: g.fire = (event.key.repeat == 0); break;
				case SDLK_RETURN: g.start = true; break;
			}
		}
		else if (event.type == SDL_QUIT)
			ret = false;
	}

	return ret;
}

// ----------------------------------------------------------------
void MoveStuff()
{
	// Calc new ship position
	if(g.up && g.ship_y > 35) g.ship_y -= SHIP_SPEED;
	if(g.down && g.ship_y < 416) g.ship_y += SHIP_SPEED;
	if(g.left && g.ship_x > 0) g.ship_x -= SHIP_SPEED;
	if(g.right && g.ship_x < 656)	g.ship_x += SHIP_SPEED;

	if(g.fire)
	{
		for (int i = 0; i < NUM_SHOTS; i++)
		{
			if (g.shots[i].alive == false)
			{
				Mix_PlayChannel(-1, g.fx_shoot, 0);
				g.fire = false;

				if (g.last_shot == NUM_SHOTS)
					g.last_shot = 0;

				g.shots[g.last_shot].alive = true;
				g.shots[g.last_shot].x = g.ship_x + 32;
				g.shots[g.last_shot].y = g.ship_y;
				++g.last_shot;
				break;
			}
		}
	}

	for(int i = 0; i < NUM_SHOTS; ++i)
	{
		if(g.shots[i].alive)
		{
			if(g.shots[i].x < SCREEN_WIDTH)
				g.shots[i].x += SHOT_SPEED;
			else
				g.shots[i].alive = false;
		}
	}
}

// ----------------------------------------------------------------
void Draw(SDL_Rect* enemy, bool* free_enemy, float* enemy_pos, SDL_Rect* enemy_sprite, SDL_Rect heart[])
{
	SDL_Rect target;

	// Scroll and draw background
	g.scroll += SCROLL_SPEED;
	if(g.scroll >= g.background_width)
		g.scroll = 0;

	target = { -g.scroll, 0, g.background_width, SCREEN_HEIGHT };
	
	SDL_RenderCopy(g.renderer, g.background, nullptr, &target);
	target.x += g.background_width;
	SDL_RenderCopy(g.renderer, g.background, nullptr, &target);

	// Draw player's ship --
	target = { g.ship_x, g.ship_y, 64, 64 };
	SDL_RenderCopy(g.renderer, g.ship, nullptr, &target);

	// Draw lasers --
	for(int i = 0; i < NUM_SHOTS; ++i)
	{
		if(g.shots[i].alive)
		{
			target = { g.shots[i].x, g.shots[i].y, 64, 64 };
			SDL_RenderCopy(g.renderer, g.shot, nullptr, &target);
		}
	}

	// Draw enemies --
	for (int i = 0; i < Max_Enemies; i++)
	{
		if (free_enemy[i] == false)
		{
			if (g.enemy_type[i] == 1)
			{
				enemy_pos[i] -= Enemy_Speed;
			}
			else
			{
				enemy_pos[i] -= (Enemy_Speed * 0.75);
			}
			enemy[i].x = enemy_pos[i];
			SDL_RenderCopy(g.renderer, g.sprites_texture, &enemy_sprite[i], &enemy[i]);
		}
	}

	// Draw lives --
	for (int i = 0; i < g.lives; i++)
	{
		SDL_RenderCopy(g.renderer, g.texture_lives, NULL, &heart[i]);
	}

	// Draw ammo --
	for (int i = 0; i < NUM_SHOTS; i++)
	{
		if (g.shots[i].alive == false)
		{
			SDL_RenderCopy(g.renderer, g.ammo, NULL, &g.bullet);
			g.bullet.x += 15;
		}
	}

	// Finally swap buffers
	//SDL_RenderPresent(g.renderer);
}

// ----------------------------------------------------------------
void timer(bool* spawn, bool* expl)
{
	g.delay += 1;

	if (g.delay > (Spawn_Delay * 175))
	{
		g.delay = 0;
		*spawn = true;
	}
	else
	{
		*spawn = false;
	}
	//-----------------------------------

	g.expl_delay += 1;

	if (g.expl_delay > (8))
	{
		g.expl_delay = 0;
		*expl = true;
	}
	else
	{
		*expl = false;
	}
}

//-----------------------------------------------------------------
void create_enemy(SDL_Rect* enemy, bool* free_enemy, float* enemy_pos, SDL_Rect* enemy_sprite)
{
	for (int i = 0; i < Max_Enemies; i++)
	{
		if (free_enemy[i])
		{
			if (g.enemy_counter == 3)
			{
				enemy[i].h = 64;
				enemy[i].w = 64;
				enemy_sprite[i].x = 129;
				enemy_sprite[i].y = 159;
				g.enemy_lives[i] = 2;
				g.enemy_counter = 0;
				g.enemy_type[i] = 2;
			}
			else
			{
				enemy[i].h = 50;
				enemy[i].w = 50;
				enemy_sprite[i].x = 98;
				enemy_sprite[i].y = 65;
				g.enemy_lives[i] = 1;
				g.enemy_counter++;
				g.enemy_type[i] = 1;
			}
			free_enemy[i] = false;
			enemy[i].x = 720;
			enemy[i].y = rand() % 400 + 30;
			enemy_pos[i] = enemy[i].x;
			break;
		}
	}
}

//-----------------------------------------------------------------
void bullet_hit(SDL_Rect* enemy, bool* free_enemy, projectile bullet[])
{
	for (int i = 0; i < Max_Enemies; i++)
	{
		for (int j = 0; j < NUM_SHOTS; j++)
		{
			if (free_enemy[i] == false)
			{
				if (g.shots[j].alive && ((bullet[j].x + 50) > enemy[i].x && bullet[j].x < (enemy[i].x + 40)) && ((bullet[j].y + 25) > enemy[i].y && bullet[j].y < (enemy[i].y + 40)))
				{
					if (g.enemy_lives[i] == 1)
					{
						free_enemy[i] = true;
						g.shots[j].alive = false;
						if (g.enemy_type[i] == 1)
						{
							g.points += 5;
						}
						else
						{
							g.points += 15;
						}
						g.explode[i] = true;
					}
					else
					{
						g.shots[j].alive = false;
						g.enemy_lives[i]--;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------
void int_to_char(char* str, int num)
{
	int result;
	int length = 0;

	for (int i = 1; i < 7; i++)
	{
		result = num;
		result /= pow(10.0, i);
		if (result != 0)
		{
			length++;
		}
		else
		{
			break;
		}
	}
	length++;
	for (int i = 0; i < length; i++)
	{
		result = num;
		result /= pow(10.0, (length - 1 - i));
		str[i] = result + 48;
		num -= result * pow(10.0, (length - 1 - i));
	}
	str[length] = '\0';
}

//-----------------------------------------------------------------
void char_to_score(char* score, char points[])
{
	for (int i = 0; i < 7; i++)
	{
		score[36 + i] = points[i];
	}
}

//-----------------------------------------------------------------
void detect_end(SDL_Rect* enemy, bool* free_enemy)
{
	for (int i = 0; i < Max_Enemies; i++)
	{
		if (free_enemy[i] == false)
		{
			if (enemy[i].x <= 0)
			{
				free_enemy[i] = true;
				if (g.lives > 0)
				{
					g.lives -= 1;
				}
			}
		}
	}
}

//-----------------------------------------------------------------
void detect_lose()
{
	if (g.lives == 0)
	{
		g.dead = true;
	}
}

//-----------------------------------------------------------------
void draw_dead_screen()
{
	SDL_Rect target;
	g.scroll += SCROLL_SPEED;
	if (g.scroll >= g.background_width)
		g.scroll = 0;

	target = { -g.scroll, 0, g.background_width, SCREEN_HEIGHT };
	SDL_RenderCopy(g.renderer, g.background, nullptr, &target);
	target.x += g.background_width;
	SDL_RenderCopy(g.renderer, g.background, nullptr, &target);

	target = { 0, 0, 720, 480 };
	SDL_RenderCopy(g.renderer, g.gameover, NULL, &target);
	
	SDL_RenderPresent(g.renderer);
}

//-----------------------------------------------------------------
void draw_explosion(SDL_Rect* explosion, SDL_Rect* enemy)
{
	for (int i = 0; i < Max_Enemies; i++)
	{
		if (g.explode[i])
		{
 			if (explosion[i].x == 192)
			{
				if (g.expl)
				{
					explosion[i].x = 225;
				}
			}
			else
			{
				if (g.expl)
				{
					explosion[i].x = 192;
					g.explode[i] = false;
				}
			}
			SDL_RenderCopy(g.renderer, g.sprites_texture, &explosion[i], &enemy[i]);
		}
	}
}

int main(int argc, char* args[])
{
	Start();

	SDL_Rect enemy[Max_Enemies];
	bool* free_enemy = (bool*)malloc(Max_Enemies * sizeof(bool));
	SDL_Rect enemy_sprite[Max_Enemies];
	for (int i = 0; i < Max_Enemies; i++)
	{
		enemy[i].h = 50;
		enemy[i].w = 50;
		enemy[i].x = 0;
		enemy[i].y = 50;
		free_enemy[i] = true;

		enemy_sprite[i].h = 31;
		enemy_sprite[i].w = 31;
		enemy_sprite[i].x = 98;
		enemy_sprite[i].y = 65;
	}
	float* enemy_pos = (float*)calloc(Max_Enemies, sizeof(float));
	bool spawn = false;

	//-------------------------------------------------------------------

	SDL_Rect heart[Max_Lives];
	for (int i = 0; i < Max_Lives; i++)
	{
		heart[i].h = 30;
		heart[i].w = 30;
		heart[i].x = 0 + (i * 35);
		heart[i].y = 0;
	}
	//-------------------------------------------------------------------

	SDL_Rect explosion[Max_Enemies];
	for (int i = 0; i < Max_Enemies; i++)
	{
		explosion[i].h = 31;
		explosion[i].w = 31;
		explosion[i].x = 192;
		explosion[i].y = 225;
	}
	//-------------------------------------------------------------------
	g.bullet.h = 30;
	g.bullet.w = 22;

	SDL_Rect bckg;
	bckg.h = 480;
	bckg.w = 720;
	bckg.x = 0;
	bckg.y = 0;

	while(CheckInput())
	{
		if (g.start)
		{
			if (g.dead == false)
			{
				g.bullet.x = 0;
				g.bullet.y = 455;
				timer(&spawn, &g.expl); //comment to delete enemies
				if (spawn)
				{
					create_enemy(&enemy[0], free_enemy, &enemy_pos[0], &enemy_sprite[0]);
				}

				detect_end(&enemy[0], free_enemy);
				bullet_hit(&enemy[0], free_enemy, &g.shots[0]);

				MoveStuff();

				detect_lose();

				Draw(&enemy[0], free_enemy, &enemy_pos[0], &enemy_sprite[0], &heart[0]);
				draw_explosion(&explosion[0], &enemy[0]);

				SDL_RenderPresent(g.renderer);
			}
			else
			{
				draw_dead_screen();
			}
		}
		else
		{
			SDL_RenderCopy(g.renderer, g.tutorial, NULL, &bckg);

			SDL_RenderPresent(g.renderer);
		}
		int_to_char(&g.c_points[0], g.points);
		char_to_score(&g.score[0], g.c_points);
		SDL_SetWindowTitle(g.window, g.score);
	}

	Finish();

	return(0); // EXIT_SUCCESS
}