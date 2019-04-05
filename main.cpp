#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <sstream>
#include <time.h>

const int screen_width = 1022;
const int screen_height = 620;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

class texture
{
public:
	texture();
	~texture();
	bool loadFromFile(std::string path);
	void free();
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
	void render(int x, int y, SDL_Rect* clip = NULL);
	int getWidth();
	int getHeight();
private:
	SDL_Texture* mTexture;
	int mWidth;
	int mHeight;
};

texture card;
texture cardback;
texture background;
texture pokerf;
texture patches;
texture player;
texture john;
texture chip;
texture pot_img;
texture button[5];
texture plus;
texture minus;
texture raisebox;
texture infos;
SDL_Rect loc_carte[52];

void draw_map();

int deck[53], bani[5], m[17], c[11], show, h[9][9], w[9];
int fold[5], maxime[5], win[5], pot, blind, bet[9];
int simulare = 0, sim_maxime[5], sim_win[5];
int small_bet = 20, all_fold, tura, ord[5], prev_raise;
bool quit = false;

TTF_Font *gFont = NULL;

//Rendered texture
texture gTextTexture;

texture::texture()
{
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

texture::~texture()
{
	free();
}

bool texture::loadFromFile(std::string path)
{
	free();
	SDL_Texture *newtexture = NULL;
	SDL_Surface *loadedsurface = IMG_Load(path.c_str());
	if (loadedsurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
		SDL_Delay(15000);
	}
	else
	{
		SDL_SetColorKey(loadedsurface, SDL_TRUE, SDL_MapRGB(loadedsurface->format, 0, 0xFF, 0xFF));
		newtexture = SDL_CreateTextureFromSurface(gRenderer, loadedsurface);
		if (newtexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			mWidth = loadedsurface->w;
			mHeight = loadedsurface->h;
		}
		SDL_FreeSurface(loadedsurface);
	}
	mTexture = newtexture;
	return mTexture != NULL;
}

bool texture::loadFromRenderedText(std::string textureText, SDL_Color textColor)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
	if (textSurface == NULL)
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError()); SDL_Delay(15000);
	}
	else
	{
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError()); SDL_Delay(15000);
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}

	//Return success
	return mTexture != NULL;
}

void texture::free()
{
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void texture::render(int x, int y, SDL_Rect* clip)
{
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}
	SDL_RenderCopy(gRenderer, mTexture, clip, &renderQuad);
}

int texture::getWidth()
{
	return mWidth;
}

int texture::getHeight()
{
	return mHeight;
}

bool init()
{
	bool success = true;
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError()); SDL_Delay(10000);
		success = false;
	}
	else
	{
		if (TTF_Init() == -1)
		{
			printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError()); SDL_Delay(10000);
			success = false;
		}
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}
		//Create window
		gWindow = SDL_CreateWindow("Worst Poker App Made In The World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError()); SDL_Delay(10000);
			success = false;
		}
		else
		{
			SDL_Surface *gameIcon = IMG_Load("Pictures/icon.png");
			SDL_SetWindowIcon(gWindow, gameIcon);
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError()); SDL_Delay(10000);
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					SDL_Delay(10000);
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	bool success = true;

	if (!background.loadFromFile("Pictures/back.jpg"))
	{
		printf("Failed to load back texture!\n");
		success = false;
		SDL_Delay(15000);
	}
	if (!card.loadFromFile("Pictures/cards.png"))
	{
		printf("Failed to load back texture!\n");
		success = false;
		SDL_Delay(15000);
	}
	else
	{
		for (int j = 0; j < 4; j++)
		{
			for (int i = 0; i <= 12; i++)
			{
				loc_carte[j * 13 + i].x = (73 * i) + 1;
				loc_carte[j * 13 + i].y = 1 + 98 * j;
				loc_carte[j * 13 + i].w = 71;
				loc_carte[j * 13 + i].h = 96;
			}
		}
	}
	if (!infos.loadFromFile("Pictures/informations.png"))
	{
		printf("No infos");
		success = false;
		SDL_Delay(15000);
	}
	if (!pokerf.loadFromFile("Pictures/pokerface.png"))
	{
		printf("No poker face sorry");
		success = false;
		SDL_Delay(15000);
	}
	if (!patches.loadFromFile("Pictures/Patches.png"))
	{
		printf("Never Patches");
		success = false;
		SDL_Delay(15000);
	}
	if (!cardback.loadFromFile("Pictures/cardback.png"))
	{
		printf("No star");
		success = false;
		SDL_Delay(15000);
	}
	if (!player.loadFromFile("Pictures/you.png"))
	{
		printf("I don't exist");
		success = false;
		SDL_Delay(15000);
	}
	if (!john.loadFromFile("Pictures/cena.png"))
	{
		printf("You can't see me");
		success = false;
		SDL_Delay(15000);
	}
	if (!chip.loadFromFile("Pictures/chip.png"))
	{
		printf("No money");
		success = false;
		SDL_Delay(15000);
	}
	if (!pot_img.loadFromFile("Pictures/pot.png"))
	{
		printf("No pot money");
		success = false;
		SDL_Delay(15000);
	}
	if (!button[1].loadFromFile("Pictures/bfold.png"))
	{
		printf("No buttons");
		success = false;
		SDL_Delay(15000);
	}
	if (!button[2].loadFromFile("Pictures/bcheck.png"))
	{
		printf("No buttons");
		success = false;
		SDL_Delay(15000);
	}
	if (!button[3].loadFromFile("Pictures/bcall.png"))
	{
		printf("No buttons");
		success = false;
		SDL_Delay(15000);
	}
	if (!button[4].loadFromFile("Pictures/braise.png"))
	{
		printf("No buttons");
		success = false;
		SDL_Delay(15000);
	}
	if (!plus.loadFromFile("Pictures/plus.png"))
	{
		printf("No plus");
		success = false;
		SDL_Delay(15000);
	}
	if (!minus.loadFromFile("Pictures/minus.png"))
	{
		printf("No minus");
		success = false;
		SDL_Delay(15000);
	}
	if (!raisebox.loadFromFile("Pictures/raisebox.png"))
	{
		printf("No box");
		success = false;
		SDL_Delay(15000);
	}
	gFont = TTF_OpenFont("Pictures/font.ttf", 12);
	if (gFont == NULL)
	{
		printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
		success = false; SDL_Delay(15000);
	}
	return success;
}

void close()
{
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;
	background.free();
	pokerf.free();
	patches.free();
	card.free();
	cardback.free();
	IMG_Quit();
	SDL_Quit();
}

void player_bet()
{
	SDL_Delay(2000);
	draw_map();
	button[1].render(358, 560);
	bool can_raise = false;
	int pariu;
	if (show == 0)
	{
		if (blind == small_bet)
		{
			if (bani[1] + bet[1] < blind * 2)
			{
				button[3].render(458, 560);
			}
			else if (bet[1] != blind)
			{
				button[3].render(458, 560);
				if (blind != pot)
				{
					button[4].render(558, 560);
					can_raise = true;
				}
			}
			else
			{
				button[2].render(458, 560);
				button[4].render(558, 560);
				can_raise = true;
			}
		}
		else if (blind == small_bet * 2)
		{
			button[3].render(458, 560);
		}
	}
	else
	{
		if (bet[1] + bet[2] + bet[3] + bet[4] != 0)
		{
			button[3].render(458, 560);
			if (bani[1] + bet[1] > blind)
			{
				button[4].render(558, 560);
				can_raise = true;
			}
		}
		else
		{
			button[2].render(458, 560);
			button[4].render(558, 560);
			can_raise = true;
		}
	}
	std::stringstream text;
	SDL_Color textColor = { 0, 255, 0 };
	if (can_raise)
	{
		text.str("");
		raisebox.render(685, 565);
		if (show == 0)
		{
			text << blind*2 << '$';
			pariu = blind * 2;
		}
		else
		{
			minus.render(670, 570);
			plus.render(742, 570);
			if (bet[1] + bet[2] + bet[3] + bet[4] == 0)
			{
				text.str("");
				pariu = small_bet;
				text << pariu << '$';
			}
			else
			{
				pariu = blind + 1;
				text << pariu << '$';
			}
		}
		if (!gTextTexture.loadFromRenderedText(text.str().c_str(), textColor))
		{
			printf("Something is wrong");
			SDL_Delay(2500);
		}
		else
		{
			gTextTexture.render(692, 568);
		}
	}
	SDL_RenderPresent(gRenderer);
	SDL_Event e;
	bool quit1 = false;
	while (quit == false && quit1 == false)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN)
			{
				int x, y;
				SDL_GetMouseState(&x, &y);
				bool inside = true;
				if (can_raise)
				{
					//Minus
					if (x > 670 && x < 683 && y>570 && y < 582)
					{
						int randbet = rand() % 9 + 1;
						if (!(pariu - randbet > blind && pariu - randbet >= small_bet))
							randbet = 1;
						if (pariu - randbet > blind && pariu - randbet >= small_bet)
						{
							pariu -= randbet;
							raisebox.render(685, 565);
							text.str("");
							text << pariu << '$';
							if (!gTextTexture.loadFromRenderedText(text.str().c_str(), textColor));
							else
							{
								gTextTexture.render(692, 568);
							}
						}
					}
					//Plus
					else if (x > 742 && x < 754 && y>570 && y < 582)
					{
						int randbet = rand() % 9 + 1;
						if (!(pariu + randbet <= pot&&pariu + randbet <= bani[1]))
							randbet = 1;
						if (pariu + randbet <= pot&&pariu + randbet <= bani[1])
						{
							pariu += randbet;
							raisebox.render(685, 565);
							text.str("");
							text << pariu << '$';
							if (!gTextTexture.loadFromRenderedText(text.str().c_str(), textColor));
							else
							{
								gTextTexture.render(692, 568);
							}
						}
					}
				}
				if (x < 358)
				{
					inside = false;
				}
				else if (x > 658)
				{
					inside = false;
				}
				else if (y < 560)
				{
					inside = false;
				}
				else if (y > 593)
				{
					inside = false;
				}
				if (inside)
				{
					if (x < 458)
					{
						fold[1] = 1;
						quit1 = true;
					}
					else if (x < 558)
					{
						if (blind != 0)
						{
							if (bani[1] + bet[1]<blind)
							{
								bet[1] += bani[1];
								bani[1] = 0;
							}
							else
							{
								bani[1] -= blind - bet[1];
								bet[1] = blind;
							}
						}
						quit1 = true;
					}
					else if (can_raise)
					{
						if (show == 0)
						{
							blind *= 2;
							bani[1] -= blind - bet[1];
							bet[1] = blind;
						}
						else
						{
							bani[1] -= pariu - bet[1];
							blind = pariu;
							bet[1] = pariu;
						}
						quit1 = true;
					}
					draw_map();
				}
			}
		}
		SDL_RenderPresent(gRenderer);
	}
}

void shuffle(int v[])
{
	int j, i, aux;
	for (i = 1; i <= 52; i++) {
		j = rand() % 52 + 1;
		aux = v[i];
		v[i] = v[j];
		v[j] = aux;
	}
}

void deck_setup()
{
	for (int i = 1; i <= 52; i++)
		deck[i] = i;
	shuffle(deck);
}

void deal_cards()
{
	for (int i = 1; i <= 8; i++)
	{
		m[i] = deck[i];
		if (i <= 5)
			c[i] = deck[i + 8];
	}
	show = 0;
}

void fill_h(int player)
{
	if (simulare == 0)
	{
		for (int i = 1; i <= 5; i++)
			h[player][i] = c[i];
	}
	else for (int i = 1, j = 6; i <= 5; i++, j++)
		h[player][i] = c[j];
	h[player][6] = m[2 * player - 1];
	h[player][7] = m[2 * player];
}

void sortare(int player)
{
	int aux;
	for (int i = 1; i < 7; i++)
		for (int j = i + 1; j <= 7; j++)
			if (h[player][i] > h[player][j])
			{
				aux = h[player][i]; h[player][i] = h[player][j]; h[player][j] = aux;
			}
}

void decode_h(int player)
{
	for (int i = 1; i <= 7; i++)
	{
		if (h[player][i] % 13 == 0)
			h[player][i] = 13;
		else if (h[player][i] % 13 == 1)
			h[player][i] = 14;
		else h[player][i] = h[player][i] % 13;
	}
	sortare(player);
}

bool culoare(int player)
{
	int nr1 = 0, nr2 = 0, nr3 = 0, nr4 = 0;
	for (int i = 1; i <= 7; i++)
	{
		if (h[player][i] >= 1 && h[player][i] <= 13)
			nr1++;
		if (h[player][i] >= 14 && h[player][i] <= 26)
			nr2++;
		if (h[player][i] >= 27 && h[player][i] <= 39)
			nr3++;
		if (h[player][i] >= 40 && h[player][i] <= 52)
			nr4++;
	}
	if (nr1 >= 5)
	{
		h[player][8] = 1; return true;
	}
	if (nr2 >= 5)
	{
		h[player][8] = 2; return true;
	}
	if (nr3 >= 5)
	{
		h[player][8] = 3; return true;
	}
	if (nr4 >= 5)
	{
		h[player][8] = 4; return true;
	}
	return false;
}

bool chintaroy(int x, int player)
{
	if ((h[player][x] >= 10 && h[player][x] <= 13) || (h[player][x] >= 23 && h[player][x] <= 26) ||
		(h[player][x] >= 36 && h[player][x] <= 39) || (h[player][x] >= 49 && h[player][x] <= 52))
		return false;
	if (h[player][x] >= 1 && h[player][x] <= 9)
		for (int i = x; i < x + 4; i++)
			if (h[player][i] + 1 != h[player][i + 1])
				return false;
	if (h[player][x] >= 14 && h[player][x] <= 22)
		for (int i = x; i < x + 4; i++)
			if (h[player][i] + 1 != h[player][i + 1])
				return false;
	if (h[player][x] >= 27 && h[player][x] <= 35)
		for (int i = x; i < x + 4; i++)
			if (h[player][i] + 1 != h[player][i + 1])
				return false;
	if (h[player][x] >= 40 && h[player][x] <= 48)
		for (int i = x; i < x + 4; i++)
			if (h[player][i] + 1 != h[player][i + 1])
				return false;
	h[player][7] = h[player][x + 4]; return true;
}

void clear_perechi(int &x, int player)
{
	for (int i = 1; i<x; i++)
		if (h[player][i] == h[player][i + 1])
		{
			for (int j = i + 1; j<x; j++)
				h[player][j] = h[player][j + 1];
			i--;
			x--;
		}

}

bool chinta(int x, int player)
{
	int k = 7;
	clear_perechi(k, player);
	if (k<5)
	{
		fill_h(player); decode_h(player); return false;
	}
	if (h[player][1] == 2 && h[player][2] == 3 && h[player][3] == 4 && h[player][4] == 5 && h[player][k] == 14)
	{
		h[player][7] = 5; return true;
	}
	if (h[player][x] >= 2 && h[player][x] <= 10)
	{
		int nr = 0;
		for (int i = x; i < x + 4; i++)
		{
			if (h[player][i] + 1 != h[player][i + 1])
			{
				fill_h(player); decode_h(player); return false;
			}
			nr++;
		}
		if (nr == 4)
		{
			h[player][7] = h[player][x + 4]; return true;
		}
		else {
			fill_h(player); decode_h(player);
			return false;
		}
	}
	else return false;
}

int perechi(int player)
{
	int v[5], x, y, z;
	v[1] = 0; v[2] = 0; v[3] = 0; v[4] = 0;
	int nr;
	for (int i = 1; i<7; i++)
	{
		nr = 1;
		if (h[player][i] != -1)
			for (int j = i + 1; j <= 7; j++)
				if (h[player][i] == h[player][j])
				{
					nr++; h[player][j] = -1;
				}
		if (nr == 2)
		{
			if (h[player][i] > v[1])
			{
				v[2] = v[1]; v[1] = h[player][i];
			}
			else if (h[player][i] > v[2])
				v[2] = h[player][i];
		}
		if (nr == 3)
		{
			if (h[player][i]>v[3])
				v[3] = h[player][i];
		}
		if (nr == 4)
		{
			v[4] = h[player][i]; fill_h(player); decode_h(player);
			for (int g = 7; g >= 1; g--)
				if (h[player][g] != v[4])
				{
					h[player][1] = h[player][g]; break;
				}
			h[player][5] = h[player][4] = h[player][3] = h[player][2] = v[4];
			return 8;
		}
		h[player][i] = -1;
	}
	if (v[1] > 0 && v[3] > 0)
	{
		h[player][5] = h[player][4] = h[player][3] = v[3];
		h[player][1] = h[player][2] = v[1];
		return 7;
	}
	if (v[3] > 0)
	{
		fill_h(player); decode_h(player);
		for (int g = 7; g >= 1; g--)
			if (h[player][g] != v[3])
			{
				x = h[player][g]; break;
			}
		for (int g = 7; g >= 1; g--)
			if (h[player][g] != v[3] && h[player][g] != x)
			{
				y = h[player][g]; break;
			}
		h[player][5] = h[player][4] = h[player][3] = v[3]; h[player][2] = x; h[player][1] = y;
		return 4;
	}
	if (v[2]>0)
	{
		fill_h(player); decode_h(player);
		for (int g = 7; g >= 1; g--)
			if (h[player][g] != v[2] && h[player][g] != v[1])
			{
				h[player][1] = h[player][g]; break;
			}
		h[player][5] = h[player][4] = v[1];
		h[player][3] = h[player][2] = v[2];
		return 3;
	}
	if (v[1] > 0)
	{
		fill_h(player); decode_h(player);
		for (int g = 7; g >= 1; g--)
			if (h[player][g] != v[1])
			{
				x = h[player][g]; break;
			}
		for (int g = 7; g >= 1; g--)
			if (h[player][g] != v[1] && h[player][g] != x)
			{
				y = h[player][g]; break;
			}
		for (int g = 7; g >= 1; g--)
			if (h[player][g] != v[1] && h[player][g] != x && h[player][g] != y)
			{
				z = h[player][g]; break;
			}
		h[player][5] = h[player][4] = v[1]; h[player][3] = x; h[player][2] = y; h[player][1] = z;
		return 2;
	}
	fill_h(player); decode_h(player);
	return 1;
}

int royala(int player)
{
	for (int k = 2; k <= 4; k++)
		if (h[player][k] == 10 && h[player][k + 1] == 11 && h[player][k + 2] == 12 && h[player][k + 3] == 13)
			if (h[player][1] == 1)
				return 10;
	for (int k = 2; k <= 4; k++)
		if (h[player][k] == 23 && h[player][k + 1] == 24 && h[player][k + 2] == 25 && h[player][k + 3] == 26)
			if (h[player][1] == 14 || h[player][2] == 14 || h[player][3] == 14)
				return 10;
	for (int k = 2; k <= 4; k++)
		if (h[player][k] == 36 && h[player][k + 1] == 37 && h[player][k + 2] == 38 && h[player][k + 3] == 39)
			if (h[player][1] == 27 || h[player][2] == 27 || h[player][3] == 27)
				return 10;
	if (h[player][4] == 49 && h[player][5] == 50 && h[player][6] == 51 && h[player][7] == 52)
		if (h[player][1] == 40 || h[player][2] == 40 || h[player][3] == 40)
			return 10;
	if (chintaroy(3, player) || chintaroy(2, player) || chintaroy(1, player))
		return 9;
	else return 0;
}

void determine_hand(int player)
{
	sortare(player);
	int roy = royala(player);
	if (roy == 10)
		w[player] = 10;
	else if (roy == 9)
		w[player] = 9;
	else if (culoare(player))
		w[player] = 6;
	if (w[player] == 0)
	{
		decode_h(player);
		if (chinta(3, player) || chinta(2, player) || chinta(1, player))
			w[player] = 5;
		else w[player] = perechi(player);
	}
}

void clear_maxime()
{
	for (int i = 1; i <= 4; i++)
		maxime[i] = 0;
}

void maxime4()
{
	maxime[1] = maxime[2] = maxime[3] = maxime[4] = 0;
	win[1] = win[2] = win[3] = win[4] = 0;
	maxime[1] = 1; int k = 1;
	for (int i = 2; i <= 4; i++)
		if (w[i] > w[maxime[1]])
		{
			clear_maxime();
			maxime[1] = i;
			k = 1;
		}
		else if (w[i] == w[maxime[1]])
			maxime[++k] = i;
}

void maxime3(int x, int y, int z)
{
	maxime[1] = maxime[2] = maxime[3] = maxime[4] = 0;
	win[1] = win[2] = win[3] = win[4] = 0;
	if (w[x] > w[y] && w[x] > w[z])
		maxime[1] = x;
	if (w[y] > w[x] && w[y] > w[z])
		maxime[1] = y;
	if (w[z] > w[y] && w[z] > w[x])
		maxime[1] = z;
	if (w[x] == w[y] && w[x] > w[z])
	{
		maxime[1] = x; maxime[2] = y;
	}
	if (w[x] == w[z] && w[x] > w[y])
	{
		maxime[1] = x; maxime[2] = z;
	}
	if (w[y] == w[z] && w[y] > w[x])
	{
		maxime[1] = y; maxime[2] = z;
	}
	if (w[x] == w[y] && w[y] == w[z])
	{
		maxime[1] = x; maxime[2] = y; maxime[3] = z;
	}
}

void maxime2(int x, int y)
{
	maxime[1] = maxime[2] = maxime[3] = maxime[4] = 0;
	win[1] = win[2] = win[3] = win[4] = 0;
	if (w[x] > w[y])
		maxime[1] = x;
	if (w[y] > w[x])
		maxime[1] = y;
	if (w[x] == w[y])
	{
		maxime[1] = x; maxime[2] = y;
	}
}

void best_hands()
{
	fill_h(1);
	fill_h(2);
	fill_h(3);
	fill_h(4);
	for (int i = 1; i <= 4; i++)
		w[i] = 0;
	if (fold[1] == 0)
		determine_hand(1);
	if (fold[2] == 0)
		determine_hand(2);
	if (fold[3] == 0)
		determine_hand(3);
	if (fold[4] == 0)
		determine_hand(4);
	if (fold[1] + fold[2] + fold[3] + fold[4] == 0)
		maxime4();
	if (fold[1] + fold[2] + fold[3] + fold[4] == 1)
	{
		if (fold[1] == 1)
			maxime3(2, 3, 4);
		if (fold[2] == 1)
			maxime3(1, 3, 4);
		if (fold[3] == 1)
			maxime3(1, 2, 4);
		if (fold[4] == 1)
			maxime3(1, 2, 3);
	}
	if (fold[1] + fold[2] + fold[3] + fold[4] == 2)
	{
		if (fold[1] == 1 && fold[2] == 1)
			maxime2(3, 4);
		if (fold[1] == 1 && fold[3] == 1)
			maxime2(2, 4);
		if (fold[1] == 1 && fold[4] == 1)
			maxime2(2, 3);
		if (fold[2] == 1 && fold[3] == 1)
			maxime2(1, 4);
		if (fold[2] == 1 && fold[4] == 1)
			maxime2(1, 3);
		if (fold[3] == 1 && fold[4] == 1)
			maxime2(1, 2);
	}
}

int compare(int x, int y)
{
	int k = 5;
	while (h[x][k] == h[y][k] && k != 0)
		k--;
	if (k == 0)
		return 0;
	if (h[x][k] > h[y][k])
		return 1;
	else return 2;
}

void castigator_i()
{
	int nr = 1;
	if (maxime[2] == 0)
		win[1] = maxime[1];
	else if (w[maxime[1]] == 1 || w[maxime[1]] == 2 || w[maxime[1]] == 3 || w[maxime[1]] == 4 || w[maxime[1]] == 7 || w[maxime[1]] == 8)
	{
		win[1] = maxime[1];
		for (int n = 1; n <= 3; n++)
		{
			if (maxime[n + 1] == 0)
				break;
			int x = compare(maxime[n], maxime[n + 1]);
			if (x == 2)
			{
				if (nr>1)
				{
					win[3] = win[2] = win[1] = 0; nr = 1;
				}
				win[1] = maxime[n + 1];
			}
			if (x == 1)
				maxime[n + 1] = maxime[n];
			if (x == 0)
				win[++nr] = maxime[n + 1];
		}
	}
	else if (w[maxime[1]] == 5 || w[maxime[1]] == 9)
	{
		win[1] = maxime[1];
		for (int n = 1; n <= 3; n++)
		{
			if (maxime[n + 1] == 0)
				break;
			if (h[maxime[n + 1]][7] > h[maxime[n]][7])
			{
				if (nr>1)
				{
					win[3] = win[2] = win[1] = 0; nr = 1;
				}
				win[1] = maxime[n + 1];
			}
			else if (h[maxime[n]][7] > h[maxime[n + 1]][7])
				maxime[n + 1] = maxime[n];
			else if (h[maxime[n + 1]][7] == h[maxime[n]][7])
				win[++nr] = maxime[n + 1];
		}
	}
	else if (w[maxime[1]] == 6)
	{
		win[1] = maxime[1];
		for (int n = 1; n <= 3; n++)
		{
			int x, y, start, finish;
			if (h[maxime[n]][8] == 1)
			{
				start = 1; finish = 13;
			}
			if (h[maxime[n]][8] == 2)
			{
				start = 14; finish = 26;
			}
			if (h[maxime[n]][8] == 3)
			{
				start = 27; finish = 39;
			}
			if (h[maxime[n]][8] == 4)
			{
				start = 40; finish = 52;
			}

			int k = 8, z = 8, ok = 1;
			while (k>0 && z>0 && ok == 1)
			{
				x = h[maxime[n]][--k];
				y = h[maxime[n]][--z];
				while (x<start || x>finish)
					x = h[maxime[n]][--k];
				while (y<start || y>finish)
					y = h[maxime[n]][--z];
				if (y>x)
				{
					if (nr>1)
						win[3] = win[2] = 0;
					win[1] = maxime[n + 1]; ok = 0;
				}
				if (x>y)
				{
					maxime[n + 1] = maxime[n]; ok = 0;
				}
			}
		}
	}
}

void sim_shuffle(int v[], int lg)
{
	int j, i, aux;
	for (i = 1; i <= lg; i++)
	{
		j = rand() % lg + 1;
		aux = v[i]; v[i] = v[j]; v[j] = aux;
	}
}

void sim_maxim()
{
	sim_maxime[1] = sim_maxime[2] = sim_maxime[3] = sim_maxime[4] = 0;
	sim_maxime[1] = 5; int k = 1;
	for (int i = 6; i <= 8; i++)
		if (w[i] > w[sim_maxime[1]])
		{
			sim_maxime[2] = sim_maxime[3] = sim_maxime[4] = 0;
			sim_maxime[1] = i;
			k = 1;
		}
		else if (w[i] == w[sim_maxime[1]])
			sim_maxime[++k] = i;
}

void sim_castigator()
{
	int nr = 1; sim_win[1] = sim_win[2] = sim_win[3] = sim_win[4] = 0;
	if (sim_maxime[2] == 0)
		sim_win[1] = sim_maxime[1];
	else if (w[sim_maxime[1]] == 1 || w[sim_maxime[1]] == 2 || w[sim_maxime[1]] == 3 || w[sim_maxime[1]] == 4 || w[sim_maxime[1]] == 7 || w[sim_maxime[1]] == 8)
	{
		sim_win[1] = sim_maxime[1];
		for (int n = 1; n <= 3; n++)
		{
			if (sim_maxime[n + 1] == 0)
				break;
			int x = compare(sim_maxime[n], sim_maxime[n + 1]);
			if (x == 2)
			{
				if (nr>1)
				{
					sim_win[3] = sim_win[2] = 0; nr = 1;
				}
				sim_win[1] = sim_maxime[n + 1];
			}
			if (x == 1)
				sim_maxime[n + 1] = sim_maxime[n];
			if (x == 0)
				sim_win[++nr] = sim_maxime[n + 1];
		}
	}
	else if (w[sim_maxime[1]] == 5 || w[sim_maxime[1]] == 9)
	{
		sim_win[1] = sim_maxime[1];
		for (int n = 1; n <= 3; n++)
		{
			if (sim_maxime[n + 1] == 0)
				break;
			if (h[sim_maxime[n + 1]][7] > h[sim_maxime[n]][7])
			{
				if (nr>1)
				{
					sim_win[3] = sim_win[2] = 0; nr = 1;
				}
				sim_win[1] = sim_maxime[n + 1];
			}
			else if (h[sim_maxime[n]][7] > h[sim_maxime[n + 1]][7])
				sim_maxime[n + 1] = sim_maxime[n];
			else if (h[sim_maxime[n + 1]][7] == h[sim_maxime[n]][7])
				sim_win[++nr] = sim_maxime[n + 1];
		}
	}
	else if (w[sim_maxime[1]] == 6)
	{
		sim_win[1] = sim_maxime[1];
		for (int n = 1; n <= 3; n++)
		{
			int x, y, start1=0, finish1=0;
			if (h[sim_maxime[n]][8] == 1)
			{
				start1 = 1; finish1 = 13;
			}
			if (h[sim_maxime[n]][8] == 2)
			{
				start1 = 14; finish1 = 26;
			}
			if (h[sim_maxime[n]][8] == 3)
			{
				start1 = 27; finish1 = 39;
			}
			if (h[sim_maxime[n]][8] == 4)
			{
				start1 = 40; finish1 = 52;
			}

			int k = 8, z = 8, ok = 1;
			while (k>0 && z>0 && ok == 1)
			{
				x = h[sim_maxime[n]][--k];
				y = h[sim_maxime[n]][--z];
				while (x<start1 || x>finish1)
					x = h[sim_maxime[n]][--k];
				while (y<start1 || y>finish1)
					y = h[sim_maxime[n]][--z];
				if (y>x)
				{
					if (nr>1)
						sim_win[3] = sim_win[2] = 0;
					sim_win[1] = sim_maxime[n + 1]; ok = 0;
				}
				if (x>y)
				{
					sim_maxime[n + 1] = sim_maxime[n]; ok = 0;
				}
			}
		}
	}

}

float simulari(int player)
{
	simulare = 1;
	int sim_deck[53], k = 0, scor = 0;
	m[9] = m[player * 2 - 1];
	m[10] = m[player * 2];
	if (show == 0)
		for (int i = 1; i <= 52; i++)
			if (i != m[9] && i != m[10])
				sim_deck[++k] = i;
	if (show == 1)
	{
		c[6] = c[1]; c[7] = c[2]; c[8] = c[3];
		for (int i = 1; i <= 52; i++)
		{
			if (i != c[6] && i != c[7] && i != c[8] && i != m[9] && i != m[10])
				sim_deck[++k] = i;
		}
	}
	if (show == 2)
	{
		c[6] = c[1]; c[7] = c[2]; c[8] = c[3]; c[9] = c[4];
		for (int i = 1; i <= 52; i++)
		{
			if (i != c[6] && i != c[7] && i != c[8] && i != c[9] && i != m[9] && i != m[10])
				sim_deck[++k] = i;
		}
	}
	if (show == 3)
	{
		c[6] = c[1]; c[7] = c[2]; c[8] = c[3]; c[9] = c[4]; c[10] = c[5];
		for (int i = 1; i <= 52; i++)
		{
			if (i != c[6] && i != c[7] && i != c[8] && i != c[9] && i != c[10] && i != m[9] && i != m[10])
				sim_deck[++k] = i;
		}
	}
	for (int i = 1; i <= 1000; i++)
	{
		sim_shuffle(sim_deck, k);
		m[11] = sim_deck[1];
		m[12] = sim_deck[2];
		m[13] = sim_deck[3];
		m[14] = sim_deck[4];
		m[15] = sim_deck[5];
		m[16] = sim_deck[6];
		if (show == 0)
		{
			c[6] = sim_deck[7];
			c[7] = sim_deck[8];
			c[8] = sim_deck[9];
			c[9] = sim_deck[10];
			c[10] = sim_deck[11];
		}
		if (show == 1)
		{
			c[9] = sim_deck[10];
			c[10] = sim_deck[11];
		}
		if (show == 2)
			c[10] = sim_deck[11];
		fill_h(5); fill_h(6); fill_h(7); fill_h(8);
		w[5] = w[6] = w[7] = w[8] = 0;
		determine_hand(5); determine_hand(6); determine_hand(7); determine_hand(8);
		sim_maxim();
		sim_castigator();
		if (sim_win[1] == 5)
			scor += 1;
	}
	return scor;
}

void fold_bet(int player)
{
	std::stringstream text;
	text.str("");
	if (bet[1] + bet[2] + bet[3] + bet[4] == 0)
	{
		SDL_Color textColor = { 0, 255, 0 };
		if (player == 2)
			text << "Patches ";
		else if (player == 3)
			text << "John Cena ";
		else if (player == 4)
			text << "Poker Face ";
		text << "checks";
		draw_map();
		if (!gTextTexture.loadFromRenderedText(text.str().c_str(), textColor));
		gTextTexture.render(10, 600);
		SDL_RenderPresent(gRenderer);
	}
	else
	{
		SDL_Color textColor = { 255, 0, 0 };
		fold[player] = 1;
		if (player == 2)
			text << "Patches ";
		else if (player == 3)
			text << "John Cena ";
		else if (player == 4)
			text << "Poker Face ";
		text << "folds";
		draw_map();
		if (!gTextTexture.loadFromRenderedText(text.str().c_str(), textColor));
		gTextTexture.render(10, 600);
		SDL_RenderPresent(gRenderer);
	}

}

void call_bet(int player)
{
	std::stringstream text;
	SDL_Color textColor = { 0, 255, 0 };
	text.str("");
	if (bet[1] + bet[2] + bet[3] + bet[4] == 0)
	{
		if (simulari(player)>350 && bani[player] >= blind)
		{
			int x = pot - blind, y = rand() % x + 1; int p = blind + y;
			while (p>bani[player] || p<small_bet)
			{
				y = rand() % x + 1; p = blind + y;
			}
			bani[player] = bani[player] - (p - bet[player]);
			blind = p;
			bet[player] = blind;
			prev_raise++;
			if (player == 2)
				text << "Patches ";
			else if (player == 3)
				text << "John Cena ";
			else if (player == 4)
				text << "Poker Face ";
			text << "raises";
			draw_map();
			if (!gTextTexture.loadFromRenderedText(text.str().c_str(), textColor));
			gTextTexture.render(10, 600);
			SDL_RenderPresent(gRenderer);
		}
	}
	else if (bani[player] + bet[player]<blind)
	{
		bet[player] = bani[player] + bet[player];
		bani[player] = 0;
		if (player == 2)
			text << "Patches ";
		else if (player == 3)
			text << "John Cena ";
		else if (player == 4)
			text << "Poker Face ";
		text << "calls";
		draw_map();
		if (!gTextTexture.loadFromRenderedText(text.str().c_str(), textColor));
		gTextTexture.render(10, 600);
		SDL_RenderPresent(gRenderer);
	}
	else
	{
		bani[player] -= blind - bet[player];
		bet[player] = blind;
		if (player == 2)
			text << "Patches ";
		else if (player == 3)
			text << "John Cena ";
		else if (player == 4)
			text << "Poker Face ";
		text << "calls";
		draw_map();
		if (!gTextTexture.loadFromRenderedText(text.str().c_str(), textColor));
		gTextTexture.render(10, 600);
		SDL_RenderPresent(gRenderer);
	}
}

void raise_bet(int player)
{
	std::stringstream text;
	SDL_Color textColor = { 255, 255, 0 };
	text.str("");
	if (show == 0)
	{
		blind *= 2;
		bani[player] -= blind - bet[player];
		bet[player] = blind;
		if (player == 2)
			text << "Patches ";
		else if (player == 3)
			text << "John Cena ";
		else if (player == 4)
			text << "Poker Face ";
		text << "raises";
		draw_map();
		if (!gTextTexture.loadFromRenderedText(text.str().c_str(), textColor));
		gTextTexture.render(10, 600);
		SDL_RenderPresent(gRenderer);
	}
	else if (bani[player] <= blind)
	{
		if (simulari(player)>200)
			call_bet(player);
		else fold_bet(player);
	}
	else if (blind == pot)
		call_bet(player);
	else
	{
		int x = pot - blind, y = rand() % x + 1; int p = blind + y;
		while (p>bani[player] || p<small_bet)
		{
			y = rand() % x + 1; p = blind + y;
		}
		bani[player] = bani[player] - (p - bet[player]);
		blind = p;
		bet[player] = blind;
		prev_raise++;
		if (player == 2)
			text << "Patches ";
		else if (player == 3)
			text << "John Cena ";
		else if (player == 4)
			text << "Poker Face ";
		text << "raises";
		draw_map();
		if (!gTextTexture.loadFromRenderedText(text.str().c_str(), textColor));
		gTextTexture.render(10, 600);
		SDL_RenderPresent(gRenderer);
	}
}

void sim_hand(int player)
{
	float x = simulari(player) / 1000;
	if (show == 0)
	{
		if (x>0.2)
		{
			if (blind >= bani[player])
			{
				if (x>0.25)
					call_bet(player);
			}
			if (blind == small_bet)
			{
				int y = rand() % 100 + 1;
				if (y <= 70)
					call_bet(player);
				else raise_bet(player);
			}
			else call_bet(player);
		}
		else fold_bet(player);
	}
	else
	{
		int y = rand() % 100 + 1;
		if (x == 1)
			raise_bet(player);
		else if (x>0.8)
		{
			if (y <= 80 / prev_raise)
				raise_bet(player);
			else call_bet(player);
		}
		else if (x>0.6)
		{
			if (y <= 70 / prev_raise)
				raise_bet(player);
			else call_bet(player);
		}
		else if (x>0.4)
		{
			if (y <= 50 / prev_raise)
				raise_bet(player);
			else call_bet(player);
		}
		else if (x>0.2)
		{
			if (y <= 20 / prev_raise)
				raise_bet(player);
			else call_bet(player);
		}
		else if (x>0.1)
		{
			if (y <= 50)
				fold_bet(player);
			else if (y <= 80)
				call_bet(player);
			else raise_bet(player);
		}
		else fold_bet(player);
	}
}

int fold_sum()
{
	return fold[1] + fold[2] + fold[3] + fold[4];
}

void ordine()
{
	int ok = 1;
	bet[1] = bet[2] = bet[3] = bet[4] = 0;
	if (tura == 0 && bani[1]>0)
	{
		ord[1] = 1; ord[2] = 2; ord[3] = 3; ord[4] = 4;
	}
	else if (tura == 1 && bani[4]>0)
	{
		ord[1] = 4; ord[2] = 1; ord[3] = 2; ord[4] = 3;
	}
	else if (tura == 2 && bani[3]>0)
	{
		ord[1] = 3; ord[2] = 4; ord[3] = 1; ord[4] = 2;
	}
	else if (tura == 3 && bani[2]>0)
	{
		ord[1] = 2; ord[2] = 3; ord[3] = 4; ord[4] = 1;
	}
	else { tura = (tura + 1) % 4; ordine(); ok = 0; }
	if (ok == 1)
	{
		for (int i = 4; i >= 1; i--)
			if (bani[ord[i]] != 0)
			{
				if (bani[ord[i]] >= small_bet)
				{
					bet[ord[i]] = small_bet;
					bani[ord[i]] -= small_bet;
				}
				else { bet[ord[i]] = bani[ord[i]]; bani[ord[i]] = 0; }
				break;
			}
		for (int i = 4; i >= 1; i--)
			if (bani[ord[i]] != 0 && bet[ord[i]] == 0)
			{
				if (bani[ord[i]] >= small_bet / 2)
				{
					bet[ord[i]] = small_bet / 2;
					bani[ord[i]] -= small_bet / 2;
				}
				else { bet[ord[i]] = bani[ord[i]]; bani[ord[i]] = 0; }
				break;
			}
	}
}

void text_print(int n)
{
	std::stringstream timeText;
	SDL_Color textColor = { 255, 95, 0 };
	if (n == 1)
	{
		if (!gTextTexture.loadFromRenderedText("You", textColor));
		gTextTexture.render(498, 503);
	}
	else if (n == 2)
	{
		if (!gTextTexture.loadFromRenderedText("Patches", textColor));
		gTextTexture.render(948, 335);
	}
	else if (n == 3)
	{
		if (!gTextTexture.loadFromRenderedText("John Cena", textColor));
		gTextTexture.render(373, 115);
	}
	else if (n == 4)
	{
		if (!gTextTexture.loadFromRenderedText("Poker Face", textColor));
		gTextTexture.render(32, 335);
	}
	else if (n == 5)
	{
		if (gTextTexture.loadFromRenderedText("Pot : ", textColor));
		gTextTexture.render(410, 232);
	}
	textColor = { 0, 255, 0 };
	timeText.str("");
	if (n == 1)
	{
		timeText << bani[1] << '$';
		if (!gTextTexture.loadFromRenderedText(timeText.str().c_str(), textColor));
		gTextTexture.render(493, 520);
	}
	else if (n == 2)
	{
		timeText << bani[2] << '$';
		if (!gTextTexture.loadFromRenderedText(timeText.str().c_str(), textColor));
		gTextTexture.render(955, 350);
	}
	else if (n == 3)
	{
		timeText << bani[3] << '$';
		if (!gTextTexture.loadFromRenderedText(timeText.str().c_str(), textColor));
		gTextTexture.render(384, 130);
	}
	else if (n == 4)
	{
		timeText << bani[4] << '$';
		if (!gTextTexture.loadFromRenderedText(timeText.str().c_str(), textColor));
		gTextTexture.render(45, 350);
	}
	else if (n == 5)
	{
		timeText << pot << '$';
		if (!gTextTexture.loadFromRenderedText(timeText.str().c_str(), textColor));
		gTextTexture.render(465, 232);
	}
}

void draw_map()
{
	//Transparenta
	SDL_SetRenderDrawColor(gRenderer, 0, 0xFF, 0xFF, 0xFF);
	//Curatare ecran
	SDL_RenderClear(gRenderer);
	//Fundal
	background.render(0, 0);
	//Tabelul de informatii
	infos.render(0, 597);
	//Cartile tale
	card.render(455, 400, &loc_carte[m[1] - 1]);
	card.render(490, 400, &loc_carte[m[2] - 1]);
	if (show >= 1)
	{
		card.render(330, 257, &loc_carte[c[1] - 1]);
		card.render(405, 257, &loc_carte[c[2] - 1]);
		card.render(480, 257, &loc_carte[c[3] - 1]);
		if (show >= 2)
		{
			card.render(555, 257, &loc_carte[c[4] - 1]);
			if (show >= 3)
			{
				card.render(630, 257, &loc_carte[c[5] - 1]);
			}
		}
	}
	//Spatele cartilor
	if (show < 4)
	{
		//John Cena
		if (fold[3] != 1)
		{
			cardback.render(455, 108);
			cardback.render(490, 108);
		}
		//Pokerface
		if (fold[4] != 1)
		{
			cardback.render(150, 257);
			cardback.render(185, 257);
		}
		//Patches
		if (fold[2] != 1)
		{
			cardback.render(760, 257);
			cardback.render(795, 257);
		}
	}
	else
	//Cartile jucatorilor
	{
		//Patches
		if (fold[2] == 0)
		{
			card.render(760, 257, &loc_carte[m[3] - 1]);
			card.render(795, 257, &loc_carte[m[4] - 1]);
		}
		//John Cena
		if (fold[3] == 0)
		{
			card.render(455, 108, &loc_carte[m[5] - 1]);
			card.render(490, 108, &loc_carte[m[6] - 1]);
		}
		//Poker Face
		if (fold[4] == 0)
		{
			card.render(150, 257, &loc_carte[m[7] - 1]);
			card.render(185, 257, &loc_carte[m[8] - 1]);
		}
	}
	//Pozele jucatorilor
	player.render(458, 500);
	if (bani[2] == 0 && fold[2] == 1);
	else
	{
		patches.render(920, 230);
	}
	if (bani[3] == 0 && fold[3] == 1);
	else
	{
		john.render(350, 10);
	}
	if (bani[4] == 0 && fold[4] == 1);
	else
	{
		pokerf.render(10, 230);
	}
	//Potul in caz ca este
	if (pot > 0)
	{
		pot_img.render(405, 230);
		text_print(5);
	}
	//Pariurile
	if (bet[1] > 0)
	{
		chip.render(570, 450);//You
		std::stringstream timeText;
		SDL_Color textColor = { 0, 255, 0 };
		timeText << bet[1] << '$';
		if (!gTextTexture.loadFromRenderedText(timeText.str().c_str(), textColor));
		gTextTexture.render(596, 453);
	}
	if (bet[2] > 0)
	{
		chip.render(760, 360);//Patches
		std::stringstream timeText;
		SDL_Color textColor = { 0, 255, 0 };
		timeText << bet[2] << '$';
		if (!gTextTexture.loadFromRenderedText(timeText.str().c_str(), textColor));
		gTextTexture.render(786, 363);
	}
	if (bet[3] > 0)
	{
		chip.render(570, 170);//John
		std::stringstream timeText;
		SDL_Color textColor = { 0, 255, 0 };
		timeText << bet[3] << '$';
		if (!gTextTexture.loadFromRenderedText(timeText.str().c_str(), textColor));
		gTextTexture.render(596, 173);
	}
	if (bet[4] > 0)
	{
		chip.render(150, 360);//Poker Face
		std::stringstream timeText;
		SDL_Color textColor = { 0, 255, 0 };
		timeText << bet[4] << '$';
		if (!gTextTexture.loadFromRenderedText(timeText.str().c_str(), textColor));
		gTextTexture.render(176, 363);
	}
	//Numele si banii jucatorilor
	text_print(1);
	if (fold[2] == 1 && bani[2] == 0);
	else
	{
		text_print(2);
	}
	if (fold[3] == 1 && bani[3] == 0);
	else
	{
		text_print(3);
	}
	if (fold[4] == 1 && bani[4] == 0);
	else
	{
		text_print(4);
	}
	//Prezentarea
	SDL_RenderPresent(gRenderer);
}

void round1()
{
	pot = 0;
	all_fold = 0;
	blind = small_bet;
	draw_map();
	for (int i = 1; i <= 4; i++)
	{
		if (!quit)
		{
			if (fold_sum() == 3)
				all_fold = 1;
			if (all_fold == 0)
			{
				if (fold[ord[i]] == 0)
				{
					if (ord[i] == 1)
					{
						if (bani[1] > 0)
							player_bet();
					}
					else if (bani[ord[i]] > 0)
					{
						SDL_Delay(2000); sim_hand(ord[i]);
					}
				}
			}
		}
	}
	if (blind == small_bet * 2)
		while (((fold[1] == 0 && bet[1] != blind && bani[1]>0) || (fold[2] == 0 && bet[2] != blind && bani[2]>0) || (fold[3] == 0 && bet[3] != blind && bani[3]>0) || (fold[4] == 0 && bet[4] != blind && bani[4]>0)) && all_fold == 0)
		{
			for (int i = 1; i <= 4; i++)
			{
				if (fold_sum() == 3)
					all_fold = 1;
				if (all_fold == 0)
				{
					if (fold[ord[i]] == 0 && bet[ord[i]] != blind)
					{
						if (ord[i] == 1)
						{
							if (bani[1]>0)
								player_bet();
						}
						else if (bani[ord[i]]>0)
						{
							SDL_Delay(2000); sim_hand(ord[i]);
						}
					}
				}
			}
		}
	pot += bet[1]; pot += bet[2]; pot += bet[3]; pot += bet[4];
	SDL_Delay(2000);
}

void round234()
{
	prev_raise = 1;
	bet[1] = bet[2] = bet[3] = bet[4] = 0;
	blind = 0;
	draw_map();
	for (int i = 1; i <= 4; i++)
	{
		if (!quit)
		{
			if (fold_sum() == 3)
				all_fold = 1;
			if (all_fold == 0)
			{
				if (fold[ord[i]] == 0)
				{
					if (ord[i] == 1)
					{
						if (bani[1] > 0)
							player_bet();
					}
					else if (bani[ord[i]] > 0)
					{
						SDL_Delay(2000); sim_hand(ord[i]); 
					}
				}
			}
		 }
	}
	if (bet[1] + bet[2] + bet[3] + bet[4] != 0)
		while (((fold[1] == 0 && bet[1] != blind && bani[1] > 0) || (fold[2] == 0 && bet[2] != blind && bani[2] > 0) || (fold[3] == 0 && bet[3] != blind && bani[3] > 0) || (fold[4] == 0 && bet[4] != blind && bani[4] > 0)) && all_fold == 0 && !quit)
		{
			for (int i = 1; i <= 4; i++)
			{
				if (fold_sum() == 3)
					all_fold = 1;
				if (all_fold == 0)
				{
					if (fold[ord[i]] == 0 && bet[ord[i]] != blind)
					{
						if (ord[i] == 1)
						{
							if (bani[1]>0)
								player_bet();
						}
						else if (bani[ord[i]]>0)
						{
							SDL_Delay(2000); sim_hand(ord[i]);
						}
					}
				}
			}
		}
	SDL_Delay(2000);
	pot += bet[1]; pot += bet[2]; pot += bet[3]; pot += bet[4];
}

void partajare()
{
	std::stringstream text;
	SDL_Color textColor = { 0, 255, 0 };
	text.str("");
	if (fold_sum() == 3)
		all_fold = 1;
	if (all_fold == 0)
	{
		best_hands();
		castigator_i();
		show++;
		int nr = 0;
		for (int i = 1; i <= 4; i++)
			if (win[i] != 0)
				nr++;
		for (int i = 1; i <= nr; i++)
		{
			bani[win[i]] += pot / nr;
			if (win[i] == 1)
				text << "You";
			else if (win[i] == 3)
				text << "John Cena";
			else if (win[i] == 4)
				text << "Poker Face";
			else if (win[i] == 2)
				text << "Patches";
				if (i < nr)
					text << ',';
		}
		if (nr == 1)
		{
			if (win[1] == 1)
				text << " win ";
			else text << " wins ";
		}
		else text << " win ";
		text << pot / nr << " $";
		if (w[win[1]] == 1)
			text << " with high card";
		if (w[win[1]] == 2)
			text << " with a pair";
		if (w[win[1]] == 3)
			text << " with two pairs";
		if (w[win[1]] == 4)
			text << " with three of a kind";
		if (w[win[1]] == 5)
			text << " with a straight";
		if (w[win[1]] == 6)
			text << " with a flush";
		if (w[win[1]] == 7)
			text << "with a full house";
		if (w[win[1]] == 8)
			text << " with four of a kind";
		if (w[win[1]] == 9)
			text << " with a Royal Flush";
		if (w[win[1]] == 10)
			text << " with a ROYAL FLUSH !!!!!";
	}
	else
	{
		for (int i = 1; i <= 4; i++)
			if (fold[i] == 0)
			{
				system("cls"); draw_map();
				bani[i] += pot;
				if (i == 1)
					text << "You win " << pot;
				else if (i == 2)
					text << "Patches wins " << pot;
				else if (i == 3)
					text << "John Cena wins " << pot;
				else if (i == 4)
					text << "Poker Face wins " << pot;
				text << " before showdown";
			}
	}
	draw_map();
	if (!gTextTexture.loadFromRenderedText(text.str().c_str(), textColor));
	gTextTexture.render(10, 600);
	SDL_RenderPresent(gRenderer);
	SDL_Delay(15000);
}

int main(int argc, char* args[])
{
	printf("Jocul e plin de buguri asa ca TE ROG sa-l inchizi de aici");
	if (!init())
	{
		printf("Program could not initialise \n"); SDL_Delay(10000);
	}
	else
	{
		if (!loadMedia())
		{
			printf("Failed to load media!\n"); SDL_Delay(10000);
		}
		else
		{
			SDL_Event e;
			srand(time(NULL));
			deck_setup();
			tura = rand() % 4;
			bani[1] = bani[2] = bani[3] = bani[4] = 5000; pot = 0;
			show = 0;
			while (!quit)
			{
				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
				}
				fold[1] = 0;
				if (bani[2]>0)
					fold[2] = 0;
				if (bani[3]>0)
					fold[3] = 0;
				if (bani[4]>0)
					fold[4] = 0;
				shuffle(deck);
				deal_cards();
				ordine();
				round1();
				if (fold_sum() == 3)
					all_fold = 1;
				if (all_fold == 0 && !quit)
				{
					show++;
					round234();
				}
				if (fold_sum() == 3)
					all_fold = 1;
				if (all_fold == 0 && !quit)
				{
					show++; round234();
				}
				if (fold_sum() == 3)
					all_fold = 1;
				if (all_fold == 0 && !quit)
				{
					show++; round234();
				}
				if (!quit)
				partajare();
				tura = (tura + 1) % 4;
			}
		}
	}
	close();
	return 0;
}