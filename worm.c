#include "worm.h"

int main ()
{
	// Initialize screen.
	initscr ();
	int scrnMax_Y, scrnMax_X;
	getmaxyx (stdscr, scrnMax_Y, scrnMax_X);

	// Color pairs.
	start_color ();
	init_pair (1, COLOR_GREEN, COLOR_BLACK); // Intro
	init_pair (2, COLOR_RED, COLOR_BLACK); // DificultyPick 
	init_pair (3, COLOR_BLACK, COLOR_WHITE); // Game
	init_pair (4, COLOR_BLUE, COLOR_BLACK); // Score

	srand (time(NULL)); //Initialize random number generator.

	// NCurses tweaks.
	keypad (stdscr, TRUE); // Collect input.
	cbreak (); // Disable line buffering.
	noecho (); // Don't print getch() chars.
	curs_set (0); // Hide blinking cursor position.
	nodelay (stdscr, TRUE); // Don't wait for EOF/newline on getch().


	// Show intro, ask for  Dificulty, World Size.
	if (SHOW_INTRO == TRUE) {ShowIntro (scrnMax_Y, scrnMax_X);}

	int dificulty = 0;
	int wrm_step_len = 0;

	InputDificulty (&dificulty, &wrm_step_len, scrnMax_Y, scrnMax_X);

	int food_no = 0; // Current no. of food items in world.
	int food_max = 0; // Max no. of food items in world.
	int score = 0;
	int score_Y = 0, score_X = 0; // Scoreboard coords.
	int world_Y = 0, world_X = 0;
	WorldSetDims(
		&food_max, &score_Y, &score_X, 
		&world_Y, &world_X, scrnMax_Y, scrnMax_X);


	// Allocate memory for, create world array. Score is drawn separately from world.
	char *world[world_Y];
	for (int i = 0; i < world_Y; ++i)
	{
		world[i] = calloc (world_X, sizeof (char));
	}
	WorldArrayCreate (world_Y, world_X, world);

	char wrm_headTurn = '0';
	int wrm_head_X = 0, wrm_head_Y = 0, wrm_len = 0;
	wrm_head_Y = 1;
	wrm_head_X = 1;
	wrm_len = -1; //No. of worm segments (head is not segment).


	// ! Coord of every asset is stored in array located in pointer array. !
	
	int *food_YX[2];
	food_YX[0] = calloc (food_max, sizeof (int));
	food_YX[1] = calloc (food_max, sizeof (int));
	
	int *wrm_allPos_YX[2];
	int worldSize = world_Y * world_X;
	// Segments are indexed, stored by wrmLength.
	wrm_allPos_YX[0] = calloc (worldSize, sizeof (int));
	wrm_allPos_YX[1] = calloc (worldSize, sizeof (int));

	// Start the game
	int gameActive = TRUE;
	attron (COLOR_PAIR(3));

	// Draw World. Only updated positions are refreshed during game.
	ArrayDraw_Char_Ptr (world_Y, world_X, world);

	WormPlace(
		&wrm_head_Y, &wrm_head_X, &wrm_headTurn, 
		wrm_allPos_YX, &wrm_len, world_Y, world_X);
	FoodSpawn(
		wrm_head_Y, wrm_head_X, wrm_allPos_YX, wrm_len, 
		world_Y, world_X, food_max, &food_no, food_YX);
		DrawAssets(
			wrm_head_Y, wrm_head_X, wrm_headTurn, 
			wrm_allPos_YX, wrm_len, food_no, food_YX);

	while (gameActive)
	{
		IsItFood(
			&wrm_head_Y, &wrm_head_X, wrm_allPos_YX, &wrm_len, 
			food_max, &food_no, &score, score_Y, score_X, food_YX, world_Y, world_X);
		InputRespond (&wrm_headTurn);
		WormMove (&wrm_head_Y, &wrm_head_X, &wrm_headTurn, wrm_allPos_YX, wrm_len);
		DrawAssets(
			wrm_head_Y, wrm_head_X, wrm_headTurn, 
			wrm_allPos_YX, wrm_len, food_no, food_YX);
		refresh ();

		gameActive = IsGameOver (wrm_head_Y, wrm_head_X, wrm_allPos_YX, wrm_len, world_Y, world_X);
		usleep (wrm_step_len);
	}

	// Free all allocated memory, end game.
	free (food_YX[0]);
	free (food_YX[1]);
	free (wrm_allPos_YX[0]);
	free (wrm_allPos_YX[1]);
	for (int i = 0; i < world_Y; ++i) {free (world[i]);}

	attroff (COLOR_PAIR(3));
	endwin ();	// End curses.
	printf ("Final length: %d \n", wrm_len + 1);
	return 0;
}


/* FUNCTIONS
 * .........*/

/*GAME STATUS*/

/*Checks, if worm ate his tail/hit wall. Returns 0 if game over.*/
int IsGameOver(
	int wrm_head_Y, int wrm_head_X, int *wrm_allPos_YX[], int wrm_len, int world_Y, int world_X)
{
	int edge_Y = world_Y - 1;
	int edge_X = world_X - 1;

	// Top/Bottom edge of world breached.
	if (wrm_head_Y <= 0 || wrm_head_Y >= edge_Y) {return 0;}
	// Left/Right edge of world breached.
	if (wrm_head_X <= 0 || wrm_head_X >= edge_X) {return 0;}

	// Head-Segment collision.
	for (int idx = 0; idx < wrm_len; ++idx)
	{
		if (wrm_head_Y == wrm_allPos_YX[0][idx] && wrm_head_X == wrm_allPos_YX[1][idx]) 
		{
			return 0;
		}
	}
	return 1;
}

/*Looks, if food item is located in front of head, if yes, expands worm.*/
void IsItFood(
	int *wrm_head_Y, int *wrm_head_X, int *wrm_allPos_YX[], 
	int *wrm_len, int food_max, int *food_no, int *score, 
	int score_Y, int score_X, int *food_YX[], int world_Y, int world_X)
{
	for (int idx = 0; idx < food_max; ++idx) // Necessary to loop through all idx pos.
	{
		if(*wrm_head_Y == food_YX[0][idx] && *wrm_head_X == food_YX[1][idx])
		{
			// Increment, draw score.
			*score += 1;
			mvprintw (score_Y, score_X, SCORE_MSG, *score); // Score

			int tailPos_Y = 0, tailPos_X = 0;
			tailPos_Y = wrm_allPos_YX[0][*wrm_len];
			tailPos_X = wrm_allPos_YX[1][*wrm_len];

			// Move all segments on head's position.
			SegmentsMove(
				*wrm_head_Y, *wrm_head_X, wrm_allPos_YX, *wrm_len);
			// Add segment to the former last position.
			SegmentAdd (tailPos_Y, tailPos_X, wrm_len, wrm_allPos_YX);
			// Clean up after old food, spawn new one if all consumed.
			food_YX[0][idx]= - 1;
			food_YX[1][idx]= - 1;
			*food_no -= 1;
			if (*food_no < 1)
			{
				FoodSpawn(
					*wrm_head_Y, *wrm_head_X, wrm_allPos_YX, 
					*wrm_len, world_Y, world_X, food_max, food_no, food_YX);
			}
			break;
		}
	}
}

/*Asks user to select dificulty.*/
void InputDificulty (int *dificulty, int *wrm_step_len, int scrnMax_Y, int scrnMax_X)
{
	int scrnCenter_Y = scrnMax_Y / 2;
	int scrnCenter_X = scrnMax_X / 2;
	attron (COLOR_PAIR(2));
	mvprintw (0, scrnCenter_X / 2, "Please, pick your dificulty (KeyArrows): ");
	mvprintw (scrnCenter_Y, 0, "Easy	<--");
	mvprintw (scrnCenter_Y, scrnMax_X - 14, "->	Medium");
	mvprintw (scrnMax_Y - 1, scrnCenter_X - 4, "v Hard");
	refresh ();

	while (*dificulty == 0) // Wait until dificutly is chosen.
	{
		int c = getch();
		if (c == KEY_LEFT) 
		{
			*dificulty = DIFICULTY_EASY; 
			*wrm_step_len = SECOND * 0.4;
		}
		else if (c == KEY_RIGHT) 
		{
			*dificulty = DIFICULTY_MEDIUM; 
			*wrm_step_len = SECOND * 0.2;
		}
		else if (c == KEY_DOWN) 
		{
			*dificulty = DIFICULTY_HARD;  
			*wrm_step_len = SECOND * 0.1;
		}
	}
	clear ();
	refresh ();
	attroff (COLOR_PAIR(2));
}


/*WORM BEHAVIOR*/

/*Changes head's direction based on user input.*/
void InputRespond (char *wrm_headTurn)
{
	int c = getch();
	if (c == KEY_UP || c == 'w') {*wrm_headTurn = WORMUP;}
	else if (c == KEY_DOWN || c == 's') {*wrm_headTurn = WORMDOWN;}
	else if (c == KEY_LEFT || c == 'a') {*wrm_headTurn = WORMLEFT;}
	else if (c == KEY_RIGHT || c == 'd') {*wrm_headTurn = WORMRIGHT;}
}


/*Stores new Y, X coords of worm segments into their arrays, increments worm_len.*/
void SegmentAdd(int newLoc_Y, int newLoc_X, int *wrm_len, int *wrm_allPos_YX[])
{
	*wrm_len += 1; // wormPositions are indexed from 0.
	wrm_allPos_YX[0][*wrm_len] = newLoc_Y;
	wrm_allPos_YX[1][*wrm_len] = newLoc_X;
}

/*Moves worm's head along with segments.*/
void WormMove(
	int *wrm_head_Y, int *wrm_head_X, char *wrm_headTurn, int *wrm_allPos_YX[], int wrm_len)
{
	// We will move segments where head is now.
	SegmentsMove(
			*wrm_head_Y, *wrm_head_X, wrm_allPos_YX, wrm_len);

	// Move head to the direction it's facing to.
	mvprintw (*wrm_head_Y,*wrm_head_X, "%c", WORLD_EMPTY_SPACE);
	if (*wrm_headTurn == WORMUP) {*wrm_head_Y -= 1;}
	else if (*wrm_headTurn == WORMDOWN) {*wrm_head_Y += 1;}
	else if (*wrm_headTurn == WORMLEFT) {*wrm_head_X -= 1;}
	else if (*wrm_headTurn == WORMRIGHT) {*wrm_head_X += 1;}
}

/*Moves all worm segments to defined position.*/
void SegmentsMove (int pos_Y, int pos_X, int *wrm_allPos_YX[], int wrm_len)
{
	//Move all segments (except 1st one) to X, Y of neigbour segment. 
	int segmentLocX = 0, segmentLocY = 0;
	int nextSegment_Y = 0, nextSegment_X = 0;
	for (int idx = wrm_len; idx >= 1; --idx)
	{
		segmentLocY = wrm_allPos_YX[0][idx];
		segmentLocX = wrm_allPos_YX[1][idx];
		nextSegment_Y = wrm_allPos_YX[0][idx - 1];
		nextSegment_X = wrm_allPos_YX[1][idx - 1];
		//Move segment to the position of the neighbour segment.
		mvprintw (segmentLocY,segmentLocX, "%c", WORLD_EMPTY_SPACE); //Wipe former position.
		wrm_allPos_YX[0][idx] = nextSegment_Y;
		wrm_allPos_YX[1][idx] = nextSegment_X;
	}
	if (wrm_len == 0) {mvprintw (wrm_allPos_YX[0][0], wrm_allPos_YX[1][0], "%c", WORLD_EMPTY_SPACE);}
	//Put first segment on defined position.
	wrm_allPos_YX[0][0] = pos_Y;
	wrm_allPos_YX[1][0] = pos_X;
}


/*WORLD_GEN*/

/*Sets food coords (1 - food_max) to random Empty position in world.*/
void FoodSpawn(
	int wrm_head_Y, int wrm_head_X, 
	int *wrm_allPos_YX[], int wrm_len, int world_Y, 
	int world_X, int food_max, int *food_no, int *food_YX[])
{
	int available_Y = world_Y - 2;
	int available_X = world_X - 2; // Walls take space.
	int noOfSpawns = rand() % (food_max) + 1; // Spawn 1 - food_max food.

	int foundIt = 0; 
	int ranPos_Y = 0, ranPos_X = 0;
	int notWall = 0, notFood = 0, notHead = 0, notSegment = 0;

	while (*food_no < noOfSpawns)
	{
		foundIt = FALSE; 
		while (foundIt != TRUE) // Find empty world position to place food in.
		{
			notWall = TRUE, notHead = TRUE, notFood = TRUE, notSegment = TRUE; 

			ranPos_Y = rand() % (available_Y - 1) + 1; 
			ranPos_X = rand() % (available_X - 1) + 1; 

			// Walls (vertical, horizontal world edge).
			if (ranPos_Y <= 0 || ranPos_Y >= world_Y) {notWall = FALSE; continue;}
			if (ranPos_X <= 0 || ranPos_X >= world_X) {notWall = FALSE; continue;}

			//Head coords.
			if (ranPos_Y == wrm_head_Y && ranPos_X == wrm_head_X) {notHead = FALSE; continue;}

			for (int idx = 0; idx < *food_no; ++idx) // Food coords.
			{
				if (ranPos_Y == food_YX[0][idx] && ranPos_X == food_YX[1][idx]) 
				{
					notFood = FALSE; 
					break;
				}
			}
			for (int idx = wrm_len; idx >= 1; --idx) // Segment coords.
			{
				if (ranPos_Y == wrm_allPos_YX[0][idx] && ranPos_X == wrm_allPos_YX[1][idx]) 
				{
					notSegment = FALSE; 
					break;
				}
			}

			if (notWall == TRUE && notHead == TRUE && notFood == TRUE && notSegment == TRUE) 
			{
				foundIt = TRUE; // Nothing occupies chosen position. Break loop.
			}
		}
		// Adequate position found, set food coords. 
		food_YX[0][*food_no] = ranPos_Y; 
		food_YX[1][*food_no] = ranPos_X; 
		*food_no += 1;
	}
}

/*Places worm +- near the world center, turns him into random direction.*/
void WormPlace(
	int *wrm_head_Y, int *wrm_head_X, char *wrm_headTurn, 
	int *wrm_allPos_YX[], int *wrm_len, int world_Y, int world_X)
{
	//Will round to int correctly instead of truncating fractional part.
	int worldCenter_Y = (world_Y - 1) / 2 + 1;
	int worldCenter_X = (world_X - 1) / 2 + 1;
	//Pick one of 4 directions for worm's head, place 2nd segment in the opposite direction.
	int headDirectionNo = rand() % (5 - 1) + 1; //Pick 1 of 4 directions for the head.
	int segment_Y = worldCenter_Y;
	int segment_X = worldCenter_X;

	if (headDirectionNo == 1)
	{
		*wrm_headTurn = WORMUP;
		segment_Y += 1;
	}
	else if (headDirectionNo == 2)
	{
		*wrm_headTurn = WORMDOWN;
		segment_Y -= 1;
	}
	else if (headDirectionNo == 3)
	{
		*wrm_headTurn = WORMLEFT;
		segment_X += 1;
	}
	else if (headDirectionNo == 4)
	{
		*wrm_headTurn = WORMRIGHT;
		segment_X -= 1;
	}
	*wrm_head_Y = worldCenter_Y;
	*wrm_head_X = worldCenter_X;
	*wrm_len = 0;
	wrm_allPos_YX[0][0] = segment_Y;
	wrm_allPos_YX[1][0] = segment_X;
}

/*Generates a game-world map of parameter-size.*/
void WorldArrayCreate (int world_Y, int world_X, char *world[])
{
	//Arrays are indexed from '0'.
	int lastLine = world_Y - 1;
	int lastCol = world_X - 1;
	//Mark world with specific chars.
	world[0][0] = WORLD_CORNER;		//Upper-Left
	world[0][lastCol] = WORLD_CORNER;		//Upper-Right
	world[lastLine][0] = WORLD_CORNER;		//Lower-Left
	world[lastLine][lastCol] = WORLD_CORNER;		//Lower-Right
	ArrayFillCol_Char_Ptr(0, 1, lastCol, world, WORLD_HORIZONTAL_WALL);
	ArrayFillCol_Char_Ptr(lastLine, 1, lastCol, world, WORLD_HORIZONTAL_WALL);
	/*Loop over lines between upper walls, build side walls 
	(1st, Last column of a line), fill rest with empty spaces.*/
	for (int line_idx = 1; line_idx < lastLine; ++line_idx)
	{
		world[line_idx][0] = WORLD_VERTICAL_WALL;
		world[line_idx][lastCol] = WORLD_VERTICAL_WALL;
		ArrayFillCol_Char_Ptr (line_idx, 1, lastCol, world, WORLD_EMPTY_SPACE);
	}
}


/*GRAPHICS*/

/*CURSES! Draws all relevant assets (i.e. segments, head, food) at their positions.*/
void DrawAssets(
	int wrm_head_Y, int wrm_head_X, char wrm_headTurn, 
	int *wrm_allPos_YX[], int wrm_len, int food_no, int *food_YX[])
{
	mvprintw (wrm_head_Y, wrm_head_X, "%c", wrm_headTurn); // Head

	for (int idx = wrm_len; idx >= 0; --idx) // Segments
	{
		mvprintw (wrm_allPos_YX[0][idx], wrm_allPos_YX[1][idx], "%c", WORMSEGMENT);
	}

	for (int idx = 0; idx < food_no; ++idx) // Food
	{
		mvprintw (food_YX[0][idx], food_YX[1][idx], "%c", FOOD);
	}
}

/*Match world, factors to screen size.*/
void WorldSetDims(
	int *food_max,  int *score_Y, int *score_X, 
	int *world_Y, int *world_X, int scrnMax_Y, int scrnMax_X)
{

	// Check if screen is big enough If not, exit.
	if (scrnMax_Y >= 8 && scrnMax_X >= 10) 
	{
		// Score is placed in bottom-center of world.
		*world_Y = scrnMax_Y;
		*score_Y = scrnMax_Y - 1; 

		*world_X = scrnMax_X;
		*score_X = (*world_X / 2);

		*food_max = *world_Y / 4;
	}
	else 
	{
		mvprintw (scrnMax_Y / 2, 0, "Screen size too small for this resolution.");
		usleep (3 * SECOND);
		exit (1);
	}

}

/*Shows dynamic on screen intro.*/
void ShowIntro (int scrnMax_Y, int scrnMax_X)
{
	attron (COLOR_PAIR(1));
	int center_Y = scrnMax_Y / 2;
	int center_X = scrnMax_X / 2;
	
	char head = WORMRIGHT, seg = WORMSEGMENT, food = FOOD;
	int  headPos = center_X, segPos = headPos -1, foodPos = headPos + 5; // X coords

	while (headPos <= foodPos) // Move worm right till food is reached.
	{
		mvprintw (center_Y, headPos, "%c", head);
		mvprintw (center_Y, segPos, "%c", seg);
		mvprintw (center_Y, foodPos, "%c", food);
		refresh ();
		++headPos, ++segPos;
		usleep (SECOND * 0.5);
		clear ();
	}
	// Display worm along with extra segment, show logo.
	mvprintw (center_Y, headPos, "%c", head);
	mvprintw (center_Y, segPos, "%c", seg);
	mvprintw (center_Y, segPos - 1, "%c", seg);
	mvprintw (scrnMax_Y - 2, scrnMax_X - (scrnMax_X / 4), "%c", food); // Food aesthetics.
	mvprintw ((scrnMax_Y / 2) - 2, scrnMax_X - (scrnMax_X / 4) * 3, "%c", food);
	char logo[5][43] = 
	{
		{"    ===  |\\    |      /\\      |    /   |==="},
		{"   |     | \\   |     /  \\     |   /    |   "},
		{"   |     |  \\  |    /----\\    |==|     |==="},
		{"   |     |   \\ |   /      \\   |   \\    |   "},
		{"===      |    \\|  /        \\  |    \\   |==="}
	};
	ArrayDraw_Char_2d (5, 43, logo); // Draw logo after 'intro snake' eats food.
	refresh ();
	usleep (SECOND * 2);
	clear ();
	attroff (COLOR_PAIR(1));
}


/*ARRAY MANIPULATION*/

/*CURSES! Draws 2D array to the screen using 'mvprintw()'.*/
void ArrayDraw_Char_2d (int arr_Y, int arr_X, char arr[][arr_X])
{
	for (int line_idx = 0; line_idx < arr_Y; ++line_idx)
	{
		for (int col_idx = 0; col_idx < arr_X; ++col_idx)
		{
			mvprintw (line_idx, col_idx, "%c", arr[line_idx][col_idx]);
		}
	}
}

/*CURSES! *ptr[] array containing rows of chars to the screen using 'mvprintw()'.*/
void ArrayDraw_Char_Ptr (int arr_Y, int arr_X, char *arr[arr_Y])
{
	for (int line_idx = 0; line_idx < arr_Y; ++line_idx)
	{
		for (int col_idx = 0; col_idx < arr_X; ++col_idx)
		{
			mvprintw (line_idx, col_idx, "%c", arr[line_idx][col_idx]);
		}
	}
}

//Fills array positions with specified character.
void ArrayFill_Int (int array[], int array_length, int fill_with_this)
{
	for ( int i = 0; i < array_length ; ++i ) 
	{
		array[i] = fill_with_this;
	}
}

/*Fills a single column of 2D array with given integer.*/
void ArrayFillCol_Int_2d(
	int lineNo, int startCol, int endCol, 
	int total_cols, int Array_2D[][total_cols], int fillWithMe)
{
	int idx = startCol;
	for (; idx < endCol; ++idx)
	{
		Array_2D[lineNo][idx] = fillWithMe;
	}
}

/*Fills a single column of pointer array with given char.*/
void ArrayFillCol_Char_Ptr(
	int lineNo, int startCol, int endCol, char *Array_2D[], char fillWithMe)
{
	int idx = startCol;
	for (; idx < endCol; ++idx)
	{
		Array_2D[lineNo][idx] = fillWithMe;
	}
}
