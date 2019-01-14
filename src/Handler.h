#ifndef HANDLER
#define HANDLER

#include <time.h>
#include <stdio.h>
#include <sys/socket.h>
#include <SDL2/SDL.h>

#include "View.h"
#include "Network.h"
#include "Server.h"
#include "Game.h"

#define DRAW_TILES 1
#define DRAW_MENU 2

void handle_mouse(SDL_MouseButtonEvent);

void handle_keyboard(SDL_KeyboardEvent);

#endif
