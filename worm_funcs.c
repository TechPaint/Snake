#include "worm.h"

/*worKING*/


/*WORM BEHAVIOR*/

/*Change's worm's direction based on user input.*/
void InputRespond (char *wrm_headTurn)
{
	int c = getch();
	if (c == KEY_UP) {*wrm_headTurn = WORMUP;}
	else if (c == KEY_DOWN) {*wrm_headTurn = WORMDOWN;}
	else if (c == KEY_LEFT) {*wrm_headTurn = WORMLEFT;}
	else if (c == KEY_RIGHT) {*wrm_headTurn = WORMRIGHT;}
}

/*Checks, if worm ate his tail/hit wall. Returns 0 if game over.*/
int DidILose(
	int wrm_head_Y, int wrm_head_X, int wrm_allPos_Y[], 
	int wrm_allPos_X[], int wrm_len, int world_Y, int world_X)
{
	int edge_Y = world_Y - 1, edge_X = world_X - 1;
	// Top/Bottom edge of world breached.
	if (wrm_head_Y <= 0 || wrm_head_Y >= edge_Y)
	{return 0;}
	// Left/Right edge of world breached.
	if (wrm_head_X <= 0 || wrm_head_X >= edge_X)
	{return 0;}
	// Head-Segment collision.
	for (int idx = 0; idx < wrm_len; ++idx)
	{
		if (wrm_head_Y == wrm_allPos_Y[idx] && wrm_head_X == wrm_allPos_X[idx]) 
		{return 0;}
	}
	return 1;
}

/*Looks, if food item is located in front of head, if yes, expands worm.*/
void IsItFood(
	int *wrm_head_Y, int *wrm_head_X, 
	int wrm_allPos_Y[], int wrm_allPos_X[], int *wrm_len, int food_max,
	int *food_no, int worldSize, int food_YX[][worldSize], int world_Y, int world_X)
{
	for (int idx = 0; idx < food_max; ++idx) // Necessary to loop through all idx pos.
	{
		if(*wrm_head_Y == food_YX[0][idx] && *wrm_head_X == food_YX[1][idx])
		{
			int tailPos_Y, tailPos_X;
			tailPos_Y = wrm_allPos_Y[*wrm_len];
			tailPos_X = wrm_allPos_X[*wrm_len];
			// Move all segments on head's position.
			MoveSegments(
				*wrm_head_Y, *wrm_head_X, wrm_allPos_Y, wrm_allPos_X, *wrm_len);
			// Add segment to the former last position.
			AddSegment (tailPos_Y, tailPos_X, wrm_len, wrm_allPos_Y, wrm_allPos_X);
			// Clean up after old food, spawn new one if all consumed.
			food_YX[0][idx]= - 1;
			food_YX[1][idx]= - 1;
			*food_no -= 1;
			if (*food_no < 1)
			{
				SpawnFood(
					*wrm_head_Y, *wrm_head_X, wrm_allPos_Y, wrm_allPos_X, 
					*wrm_len, world_Y, world_X, food_max, food_no, worldSize, food_YX);
			}
			break;
		}
	}
}

/*Stores new Y, X coords of worm segments into their arrays, increments worm_len.*/
void AddSegment(int newLoc_Y, int newLoc_X, int *wrm_len, int wrm_allPos_Y[], int wrm_allPos_X[])
{
	*wrm_len += 1; // wormPositions are indexed from 0.
	wrm_allPos_Y[*wrm_len] = newLoc_Y;
	wrm_allPos_X[*wrm_len] = newLoc_X;
}

/*Moves worm's head along with segments.*/
void MoveWorm(
	int *wrm_head_Y, int *wrm_head_X, char *wrm_headTurn, 
	int wrm_allPos_Y[], int wrm_allPos_X[], int wrm_len)
{
	// We will move segments where head is now.
	MoveSegments(
			*wrm_head_Y, *wrm_head_X, wrm_allPos_Y, wrm_allPos_X, wrm_len);

	// Move head to the direction it's facing to.
	mvprintw (*wrm_head_Y,*wrm_head_X, "%c", WORLD_EMPTY_SPACE);
	if (*wrm_headTurn == WORMUP) {*wrm_head_Y -= 1;}
	else if (*wrm_headTurn == WORMDOWN) {*wrm_head_Y += 1;}
	else if (*wrm_headTurn == WORMLEFT) {*wrm_head_X -= 1;}
	else if (*wrm_headTurn == WORMRIGHT) {*wrm_head_X += 1;}
}

/*Moves all worm segments to defined position.*/
void MoveSegments(int pos_Y, int pos_X, int wrm_allPos_Y[], int wrm_allPos_X[], int wrm_len)
{
	//Move all segments (except 1st one) to X, Y of neigbour segment. 
	int segmentLocX, segmentLocY, nextSegment_Y, nextSegment_X;
	for (int idx = wrm_len; idx >= 1; --idx)
	{
		segmentLocY = wrm_allPos_Y[idx];
		segmentLocX = wrm_allPos_X[idx];
		nextSegment_Y = wrm_allPos_Y[idx - 1];
		nextSegment_X = wrm_allPos_X[idx - 1];
		//Move segment to the position of the neighbour segment.
		mvprintw (segmentLocY,segmentLocX, "%c", WORLD_EMPTY_SPACE); //Wipe former position.
		wrm_allPos_Y[idx] = nextSegment_Y;
		wrm_allPos_X[idx] = nextSegment_X;
	}
	if (wrm_len == 0) {mvprintw (wrm_allPos_Y[0],wrm_allPos_X[0], "%c", WORLD_EMPTY_SPACE);}
	//Put first segment on defined position, show it in world.
	wrm_allPos_Y[0] = pos_Y;
	wrm_allPos_X[0] = pos_X;
}


/*WORLD_GEN*/

/*Sets food coords (1 - food_max) to random Empty position in world.*/
void SpawnFood(
	int wrm_head_Y, int wrm_head_X, 
	int wrm_allPos_Y[], int wrm_allPos_X[], int wrm_len, int world_Y, 
	int world_X, int food_max, int *food_no, int worldSize, int food_YX[][worldSize])
{
	int available_Y = world_Y - 2, available_X = world_X - 2; // Walls take space.
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
				if (ranPos_Y == wrm_allPos_Y[idx] && ranPos_X == wrm_allPos_X[idx]) 
					{notFood = FALSE; break;}
			}
			for (int idx = wrm_len; idx >= 1; --idx)
			{
				if (ranPos_Y == wrm_allPos_Y[idx] && ranPos_X == wrm_allPos_X[idx]) 
					{notSegment = FALSE; break;}
			}
			if (notHead == TRUE && notFood == TRUE && notSegment == TRUE) 
				{foundIt = TRUE;} // Nothing occupies chosen position. Break loop.
		}
		food_YX[0][*food_no] = ranPos_Y; 
		food_YX[1][*food_no] = ranPos_X; 
		*food_no += 1;
	}
}

/*Places worm +- near the world center, turns him into random direction.*/
void PlaceWorm(
	int *wrm_head_Y, int *wrm_head_X, char *wrm_headTurn, 
	int wrm_allPos_Y[], int wrm_allPos_X[], int *wrm_len, int world_Y, int world_X)
{
	//Will round to int correctly instead of truncating fractional part.
	int worldCenter_Y = (world_Y - 1) / 2 + 1;
	int worldCenter_X = (world_X - 1) / 2 + 1;
	//Pick one of 4 directions for worm's head, place 2nd segment in the opposite direction.
	int headDirectionNo = rand() % (5 - 1) + 1; //Pick 1 of 4 directions for the head.
	int segment_Y = worldCenter_Y, segment_X = worldCenter_X;

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
	AddSegment(
		segment_Y, segment_X, 
			wrm_len, wrm_allPos_Y, wrm_allPos_X);
}

/*Generates a game-world map of parameter-size.*/
void CreateWorld (int world_Y, int world_X, char world[][world_X])
{
	//Arrays are indexed from '0'.
	int lastLine = world_Y - 1;
	int lastCol = world_X - 1;
	//Mark world with specific chars.
	world[0][0] = WORLD_CORNER;		//Upper-Left
	world[0][lastCol] = WORLD_CORNER;		//Upper-Right
	world[lastLine][0] = WORLD_CORNER;		//Lower-Left
	world[lastLine][lastCol] = WORLD_CORNER;		//Lower-Right
	CharArrayColumnFill_2D(0, 1, lastCol, world_X, world, WORLD_HORIZONTAL_WALL);
	CharArrayColumnFill_2D(lastLine, 1, lastCol, world_X, world, WORLD_HORIZONTAL_WALL);
	/*Loop over lines between upper walls, build side walls 
	(1st, Last column of a line), fill rest with empty spaces.*/
	for (int line_idx = 1; line_idx < lastLine; ++line_idx)
	{
		world[line_idx][0] = WORLD_VERTICAL_WALL;
		world[line_idx][lastCol] = WORLD_VERTICAL_WALL;
		CharArrayColumnFill_2D (line_idx, 1, lastCol, world_X, world, WORLD_EMPTY_SPACE);
	}
}


/*GRAPHICS*/

/*CURSES! Draws all relevant assets (i.e. segments, head, food) at their positions.*/
void DrawAssets(
	int wrm_head_Y, int wrm_head_X, char wrm_headTurn, int wrm_allPos_Y[], 
	int wrm_allPos_X[], int wrm_len, int food_no, int worldSize, int food_YX[][worldSize])
{
	mvprintw (wrm_head_Y, wrm_head_X, "%c", wrm_headTurn); // Head
	for (int idx = wrm_len; idx >= 0; --idx) // Segments
	{
		mvprintw (wrm_allPos_Y[idx], wrm_allPos_X[idx], "%c", WORMSEGMENT);
	}
	for (int idx = 0; idx <= food_no; ++idx) // Food
	{
		mvprintw (food_YX[0][idx], food_YX[1][idx], "%c", FOOD);
	}
}

/*CURSES! Draws ENTIRE world (2D array) to the screen using 'printw()'.*/
void Draw2dArray (int arr_Y, int arr_X, char arr[][arr_X])
{
	for (int line_idx = 0; line_idx < arr_Y; ++line_idx)
	{
		for (int col_idx = 0; col_idx < arr_X; ++col_idx)
		{
			mvprintw (line_idx, col_idx, "%c", arr[line_idx][col_idx]);
		}
	}
}

/*Fills a single column of 2D array with given char.*/
void CharArrayColumnFill_2D(
	int lineNo, int startCol, int endCol, 
	int total_cols, char Array_2D[][total_cols], char fillWithMe)
{
	int idx = startCol;
	for (; idx < endCol; ++idx)
	{
		Array_2D[lineNo][idx] = fillWithMe;
	}
}

/*Fills a single column of 2D array with given integer.*/
void IntArrayColumnFill_2D(
	int lineNo, int startCol, int endCol, 
	int total_cols, int Array_2D[][total_cols], int fillWithMe)
{
	int idx = startCol;
	for (; idx < endCol; ++idx)
	{
		Array_2D[lineNo][idx] = fillWithMe;
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


