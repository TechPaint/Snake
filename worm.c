#include "worm_funcs.c"


int main()
{
	initscr (); // Initialize screen (stdscr).
	int scrnMax_Y, scrnMax_X;
	scrnMax_Y = scrnMax_X = 0;
	getmaxyx( stdscr, scrnMax_Y, scrnMax_X );

	// Set color pairs.
	start_color ();
	init_pair (1, COLOR_GREEN, COLOR_BLACK);
	init_pair (2, COLOR_BLACK, COLOR_WHITE);
	
	cbreak (); // Disable line buffering.
	curs_set (0); // Hide blinking cursor position.
	nodelay (stdscr, TRUE); // Don't wait for EOF/newline on getch().
	noecho (); // Don't print getch() chars.
	keypad (stdscr, TRUE); // Collect input.
	srand (time(NULL)); //Initialize random number generator.

	if (SHOW_INTRO == TRUE) {ShowIntro (scrnMax_Y);}

	attron (COLOR_PAIR(2));
	int gameActive = TRUE;
	// Create a game world.
	int world_Y = 20;
	int world_X = 28;
	int worldSize = world_Y * world_X;
	char world[world_Y][world_X];
	CreateWorld (world_Y, world_X, world);
	// Initialize basic variables.
	int wrm_head_X, wrm_head_Y, wrm_len;
	wrm_head_Y = 1;
	wrm_head_X = 1;
	char wrm_headTurn;
	wrm_len = -1; //No. of worm segments (head is not segment).

	int food_max = 3; // Max no. of food items in the map.
	int food_no = 0; // Current no. of food items in the map.
	int food_YX[worldSize][worldSize]; // Y, X coords of the food.
	// Prepare coord array (-1 means empty position).
	IntArrayColumnFill_2D (0, 0, worldSize, worldSize, food_YX, -1);
	IntArrayColumnFill_2D (1, 0, worldSize, worldSize, food_YX, -1);

	//Arrays holding Y, X coords of segments (indexed by wrmLength).
	int wrm_allPos_Y[worldSize];
	int wrm_allPos_X[worldSize];
	ArrayFill_Int (wrm_allPos_Y, worldSize, -1);
	ArrayFill_Int (wrm_allPos_X, worldSize, -1);

	Draw2dArray (world_Y, world_X, world);
	PlaceWorm(
		&wrm_head_Y, &wrm_head_X, &wrm_headTurn, 
		wrm_allPos_Y, wrm_allPos_X, &wrm_len, world_Y, world_X);
	SpawnFood(
		wrm_head_Y, wrm_head_X, wrm_allPos_Y, wrm_allPos_X, 
		wrm_len, world_Y, world_X, food_max, &food_no, worldSize, food_YX);

	DrawAssets(
		wrm_head_Y, wrm_head_X, wrm_headTurn, wrm_allPos_Y, 
		wrm_allPos_X, wrm_len, food_no, worldSize, food_YX);

	while (gameActive)
	{
		IsItFood(
			&wrm_head_Y, &wrm_head_X, wrm_allPos_Y, wrm_allPos_X, 
			&wrm_len, food_max, &food_no, worldSize, food_YX, world_Y, world_X);

		InputRespond (&wrm_headTurn);
		MoveWorm (&wrm_head_Y, &wrm_head_X, &wrm_headTurn, wrm_allPos_Y, wrm_allPos_X, wrm_len);
		DrawAssets(
			wrm_head_Y, wrm_head_X, wrm_headTurn, wrm_allPos_Y, 
			wrm_allPos_X, wrm_len, food_no, worldSize, food_YX);
		refresh ();
		gameActive = DidILose (wrm_head_Y, wrm_head_X, wrm_allPos_Y, wrm_allPos_X, wrm_len, world_Y, world_X);
		usleep (400000);
	}
	endwin ();	// End curses.
	
	printf ("Final length: %d", wrm_len + 1);
	attroff (COLOR_PAIR(2));
	
	endwin ();	// End curses.
	return 0;
}

