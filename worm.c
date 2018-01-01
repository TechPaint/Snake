#include "worm_funcs.c"


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
	/*
	// Deallocate memory, end game.
	free (world);
	free (food_YX);
	free (wrm_allPos_YX);
	*/
	attroff (COLOR_PAIR(4));
	endwin ();	// End curses.
	printf ("Final length: %d \n", wrm_len + 1);
	return 0;
}

