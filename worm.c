#include "worm.h"


int main ()
{
	initscr (); // Initialize screen (stdscr).
	int scrnMax_Y, scrnMax_X;
	getmaxyx (stdscr, scrnMax_Y, scrnMax_X);
	// Color(Pair)s.
	start_color ();
	init_pair (1, COLOR_GREEN, COLOR_BLACK); // Intro
	init_pair (2, COLOR_RED, COLOR_BLACK); // DificultyPick 
	init_pair (3, COLOR_WHITE, COLOR_BLACK); // WorldPick
	init_pair (4, COLOR_BLACK, COLOR_WHITE); // Game

	cbreak (); // Disable line buffering.
	curs_set (0); // Hide blinking cursor position.
	nodelay (stdscr, TRUE); // Don't wait for EOF/newline on getch().
	noecho (); // Don't print getch() chars.
	keypad (stdscr, TRUE); // Collect input.
	srand (time(NULL)); //Initialize random number generator.

	// Intro, pick Dificulty & World.
	if (SHOW_INTRO == TRUE) {ShowIntro (scrnMax_Y, scrnMax_X);}
	int dificulty = 0, wrm_step_len = 0;
	GetDificulty (&dificulty, &wrm_step_len, scrnMax_Y, scrnMax_X);
	int gameActive = TRUE;
	int world_Y = 0, world_X = 0;
	int food_max = 0; // Max no. of food items in the map.
	int food_no = 0; // Current no. of food items in the map.

	// Create game world/w scoreboard.
	// World is allocated array of pointers. (Each pointer is pointing to world row.)
	GetWorldSize (&gameActive, &food_max, &world_Y, &world_X, scrnMax_Y, scrnMax_X);
	if (gameActive == FALSE) {return 1;} // Resolution error. Quit the game.
	char *world[world_Y];
	for (int i = 0; i < world_Y; ++i)
	{
		world[i] = calloc (world_X, sizeof (char));
	}
	CreateWorld (world_Y, world_X, world);
	// Score is drawn separately from world.
	int score = 0, score_Y = world_Y / 2, score_X = world_X + 2; 

	char wrm_headTurn;
	int wrm_head_X, wrm_head_Y, wrm_len;
	wrm_head_Y = 1;
	wrm_head_X = 1;
	wrm_len = -1; //No. of worm segments (head is not segment).

	// Y, X coords of the food (pointers to arrays holding them).
	int *food_YX[2];
	food_YX[0] = calloc (food_max, sizeof (int));
	food_YX[1] = calloc (food_max, sizeof (int));
	
	// Arrays holding Y, X coords of segments (indexed by wrmLength).
	int *wrm_allPos_YX[2];
	wrm_allPos_YX[0] = calloc (1, sizeof (int));
	wrm_allPos_YX[1] = calloc (1, sizeof (int));

	// Start the game
	attron (COLOR_PAIR(4));
	CharArrayPtr_Draw (world_Y, world_X, world);
	PlaceWorm(
		&wrm_head_Y, &wrm_head_X, &wrm_headTurn, 
		wrm_allPos_YX, &wrm_len, world_Y, world_X);
	SpawnFood(
		wrm_head_Y, wrm_head_X, wrm_allPos_YX, wrm_len, 
		world_Y, world_X, food_max, &food_no, food_YX);
	DrawAssets(
		wrm_head_Y, wrm_head_X, wrm_headTurn, wrm_allPos_YX, wrm_len, food_no, food_YX);

	while (gameActive)
	{
		IsItFood(
			&wrm_head_Y, &wrm_head_X, wrm_allPos_YX, &wrm_len, 
			food_max, &food_no, &score, score_Y, score_X, food_YX, world_Y, world_X);
		InputRespond (&wrm_headTurn);
		MoveWorm (&wrm_head_Y, &wrm_head_X, &wrm_headTurn, wrm_allPos_YX, wrm_len);
		DrawAssets(
			wrm_head_Y, wrm_head_X, wrm_headTurn, wrm_allPos_YX, wrm_len, food_no, food_YX);
		refresh ();
		gameActive = DidILose (wrm_head_Y, wrm_head_X, wrm_allPos_YX, wrm_len, world_Y, world_X);
		usleep (wrm_step_len);
	}
	// Deallocate memory, end game.
	free (world);

	// ! ERROR ! - attempt to free non-heap object.
	/*
	free (food_YX);
	free (wrm_allPos_YX);
	*/

	attroff (COLOR_PAIR(4));
	endwin ();	// End curses.
	printf ("Final length: %d \n", wrm_len + 1);
	return 0;
}


/*
 * FUNCTIONS
 * .........
 * */
 
/*Reallocates int array to specified memory size.*/ 
void IntArray_Realloc ( int **arr, int realloc_size)
{
	int *tmp = 0;
	tmp = realloc (arr, realloc_size);
	// Handle memory realloc failure.
	if (tmp == NULL) 
	{
		free (arr);
		printf ("\n\t Error - realloc() failed.");
		exit (1);
	} 
	*arr = tmp;
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

/*Checks, if worm ate his tail/hit wall. Returns 0 if game over.*/
int DidILose(
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
	int *wrm_head_Y, int *wrm_head_X, int *wrm_allPos_YX[], int *wrm_len, int food_max, 
	int *food_no, int *score, int score_Y, int score_X, int *food_YX[], int world_Y, int world_X)
{
	for (int idx = 0; idx < food_max; ++idx) // Necessary to loop through all idx pos.
	{
		if(*wrm_head_Y == food_YX[0][idx] && *wrm_head_X == food_YX[1][idx])
		{
			// Increment, draw score.
			*score += 1;
			mvprintw (score_Y, score_X, "Score: %d", *score);

			int tailPos_Y, tailPos_X;
			tailPos_Y = wrm_allPos_YX[0][*wrm_len];
			tailPos_X = wrm_allPos_YX[1][*wrm_len];
			// Move all segments on head's position.
			MoveSegments(
				*wrm_head_Y, *wrm_head_X, wrm_allPos_YX, *wrm_len);
			// Add segment to the former last position.
			AddSegment (tailPos_Y, tailPos_X, wrm_len, wrm_allPos_YX);
			// Clean up after old food, spawn new one if all consumed.
			food_YX[0][idx]= - 1;
			food_YX[1][idx]= - 1;
			*food_no -= 1;
			if (*food_no < 1)
			{
				SpawnFood(
					*wrm_head_Y, *wrm_head_X, wrm_allPos_YX, 
					*wrm_len, world_Y, world_X, food_max, food_no, food_YX);
			}
			break;
		}
	}
}

/*Stores new Y, X coords of worm segments into their arrays, increments worm_len.*/
void AddSegment(int newLoc_Y, int newLoc_X, int *wrm_len, int *wrm_allPos_YX[])
{
	*wrm_len += 1; // wormPositions are indexed from 0.
	IntArray_Realloc (&wrm_allPos_YX[0], *wrm_len);
	IntArray_Realloc (&wrm_allPos_YX[1], *wrm_len);
	wrm_allPos_YX[0][*wrm_len] = newLoc_Y;
	wrm_allPos_YX[1][*wrm_len] = newLoc_X;
}

/*Moves worm's head along with segments.*/
void MoveWorm(
	int *wrm_head_Y, int *wrm_head_X, char *wrm_headTurn, int *wrm_allPos_YX[], int wrm_len)
{
	// We will move segments where head is now.
	MoveSegments(
			*wrm_head_Y, *wrm_head_X, wrm_allPos_YX, wrm_len);

	// Move head to the direction it's facing to.
	mvprintw (*wrm_head_Y,*wrm_head_X, "%c", WORLD_EMPTY_SPACE);
	if (*wrm_headTurn == WORMUP) {*wrm_head_Y -= 1;}
	else if (*wrm_headTurn == WORMDOWN) {*wrm_head_Y += 1;}
	else if (*wrm_headTurn == WORMLEFT) {*wrm_head_X -= 1;}
	else if (*wrm_headTurn == WORMRIGHT) {*wrm_head_X += 1;}
}

/*Moves all worm segments to defined position.*/
void MoveSegments(int pos_Y, int pos_X, int *wrm_allPos_YX[], int wrm_len)
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
void SpawnFood(
	int wrm_head_Y, int wrm_head_X, 
	int *wrm_allPos_YX[], int wrm_len, int world_Y, 
	int world_X, int food_max, int *food_no, int *food_YX[])
{
	int available_Y = world_Y - 2;
	int available_X = world_X - 2; // Walls take space.
	int noOfSpawns = rand() % (food_max) + 1; // Spawn 1 - Max food_max.
	int foundIt, ranPos_Y, ranPos_X, notFood, notHead, notSegment;
	while (*food_no < noOfSpawns)
	{
		foundIt = FALSE; //Only empty positions can be food coords.
		while (foundIt != TRUE)
		{
			notHead = notFood = notSegment = TRUE; 
			ranPos_Y = rand() % (available_Y - 1) + 1; 
			ranPos_X = rand() % (available_X - 1) + 1; 
			//Check if Head/Food/Segment doesn't occupy given position.
			if (ranPos_Y == wrm_head_Y && ranPos_X == wrm_head_X) {notHead = FALSE; continue;}
			for (int idx = 0; idx < *food_no; ++idx) // Food
			{
				if (ranPos_Y == wrm_allPos_YX[0][idx] && ranPos_X == wrm_allPos_YX[1][idx]) 
				{
					notFood = FALSE; 
					break;
				}
			}
			for (int idx = wrm_len; idx >= 1; --idx)
			{
				if (ranPos_Y == wrm_allPos_YX[0][idx] && ranPos_X == wrm_allPos_YX[1][idx]) 
				{
					notSegment = FALSE; 
					break;
				}
			}
			if (notHead == TRUE && notFood == TRUE && notSegment == TRUE) 
			{
				foundIt = TRUE;
			} // Nothing occupies chosen position. Break loop.
		}
		food_YX[0][*food_no] = ranPos_Y; 
		food_YX[1][*food_no] = ranPos_X; 
		*food_no += 1;
	}
}

/*Places worm +- near the world center, turns him into random direction.*/
void PlaceWorm(
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
	//Update position data.
	*wrm_head_Y = worldCenter_Y;
	*wrm_head_X = worldCenter_X;
	AddSegment (segment_Y, segment_X, wrm_len, wrm_allPos_YX);
}

/*Generates a game-world map of parameter-size.*/
void CreateWorld (int world_Y, int world_X, char *world[])
{
	//Arrays are indexed from '0'.
	int lastLine = world_Y - 1;
	int lastCol = world_X - 1;
	//Mark world with specific chars.
	world[0][0] = WORLD_CORNER;		//Upper-Left
	world[0][lastCol] = WORLD_CORNER;		//Upper-Right
	world[lastLine][0] = WORLD_CORNER;		//Lower-Left
	world[lastLine][lastCol] = WORLD_CORNER;		//Lower-Right
	CharArrayPtr_ColumnFill(0, 1, lastCol, world, WORLD_HORIZONTAL_WALL);
	CharArrayPtr_ColumnFill(lastLine, 1, lastCol, world, WORLD_HORIZONTAL_WALL);
	/*Loop over lines between upper walls, build side walls 
	(1st, Last column of a line), fill rest with empty spaces.*/
	for (int line_idx = 1; line_idx < lastLine; ++line_idx)
	{
		world[line_idx][0] = WORLD_VERTICAL_WALL;
		world[line_idx][lastCol] = WORLD_VERTICAL_WALL;
		CharArrayPtr_ColumnFill (line_idx, 1, lastCol, world, WORLD_EMPTY_SPACE);
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
	for (int idx = 0; idx <= food_no; ++idx) // Food
	{
		mvprintw (food_YX[0][idx], food_YX[1][idx], "%c", FOOD);
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
	CharArray2d_Draw (5, 43, logo); // Draw logo after 'intro snake' eats food.
	refresh ();
	usleep (SECOND * 2);
	clear ();
	attroff (COLOR_PAIR(1));
}

/*Asks user to select dificulty.*/
void GetDificulty (int *dificulty, int *wrm_step_len, int scrnMax_Y, int scrnMax_X)
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
			*wrm_step_len = SECOND * 0.5;
		}
		else if (c == KEY_RIGHT) 
		{
			*dificulty = DIFICULTY_MEDIUM; 
			*wrm_step_len = SECOND * 0.3;
		}
		else if (c == KEY_DOWN) 
		{
			*dificulty = DIFICULTY_HARD;  
			*wrm_step_len = SECOND * 0.2;
		}
	}
	clear ();
	attroff (COLOR_PAIR(2));
}

/*Asks user to select world size*/
void GetWorldSize (int *gameActive, int *food_max, int *world_Y, int *world_X, int scrnMax_Y, int scrnMax_X)
{
	int scrnCenter_Y = scrnMax_Y / 2;
	int scrnCenter_X = scrnMax_X / 2;
	attron (COLOR_PAIR(3));
	mvprintw (0, scrnCenter_X / 2, "Please, pick world size (KeyArrows): ");
	mvprintw (scrnCenter_Y, 0, "Large	<--");
	mvprintw (scrnCenter_Y, scrnMax_X - 14, "->	Medium");
	mvprintw (scrnMax_Y - 1, scrnCenter_X - 4, "v Small");
	refresh ();
	while (*world_Y == 0) // Wait until user chooses.
	{
		int c = getch();
		if (c == KEY_LEFT) 
		{
			*food_max = 5;
			*world_Y = WORLD_LARGE_Y; 
			*world_X = WORLD_LARGE_X;
		}
		else if (c == KEY_RIGHT) 
		{
			*food_max = 3;
			*world_Y = WORLD_MEDIUM_Y; 
			*world_X = WORLD_MEDIUM_X;
		}
		else if (c == KEY_DOWN) 
		{
			*food_max = 2;
			*world_Y = WORLD_SMALL_Y; 
			*world_X = WORLD_SMALL_X;
		}
	}
	// Check if screen is big enough If not, match screen if it meets min. requirements.
	if (scrnMax_Y < *world_Y || scrnMax_X < *world_X) 
	{
		clear ();
		mvprintw (scrnCenter_Y, 0, "Screen size too small for this resolution.");
		if (scrnMax_Y >= 8 && scrnMax_X >= 10) 
		{
			*world_Y = scrnMax_Y; 
			*world_X = scrnMax_X;
		}
		else 
		{
			mvprintw (scrnCenter_Y + 1, 0, "Actually, for every resolution. Please, increase window size.");
			*gameActive = FALSE;
		}
	}
	clear ();
	attroff (COLOR_PAIR(3));
}

/*CURSES! Draws ENTIRE world (array[rows][cols]) to the screen using 'printw()'.*/
void CharArray2d_Draw (int arr_Y, int arr_X, char arr[][arr_X])
{
	for (int line_idx = 0; line_idx < arr_Y; ++line_idx)
	{
		for (int col_idx = 0; col_idx < arr_X; ++col_idx)
		{
			mvprintw (line_idx, col_idx, "%c", arr[line_idx][col_idx]);
		}
	}
}

/*CURSES! Draws ENTIRE world (*array[rows]) to the screen using 'printw()'.*/
void CharArrayPtr_Draw (int arr_Y, int arr_X, char *arr[arr_Y])
{
	for (int line_idx = 0; line_idx < arr_Y; ++line_idx)
	{
		for (int col_idx = 0; col_idx < arr_X; ++col_idx)
		{
			mvprintw (line_idx, col_idx, "%c", arr[line_idx][col_idx]);
		}
	}
}

/*Fills a single column of pointer array with given char.*/
void CharArrayPtr_ColumnFill(
	int lineNo, int startCol, int endCol, char *Array_2D[], char fillWithMe)
{
	int idx = startCol;
	for (; idx < endCol; ++idx)
	{
		Array_2D[lineNo][idx] = fillWithMe;
	}
}

//Fills array positions with specified character.
void IntArray_Fill (int array[], int array_length, int fill_with_this)
{
	for ( int i = 0; i < array_length ; ++i ) 
	{
		array[i] = fill_with_this;
	}
}

/*Fills a single column of 2D array with given integer.*/
void IntArray2d_ColumnFill(
	int lineNo, int startCol, int endCol, 
	int total_cols, int Array_2D[][total_cols], int fillWithMe)
{
	int idx = startCol;
	for (; idx < endCol; ++idx)
	{
		Array_2D[lineNo][idx] = fillWithMe;
	}
}
