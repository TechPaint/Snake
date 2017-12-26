#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>

#define FOOD		'x'
#define WORMUP			'^'
#define WORMDOWN 		'v'
#define WORMLEFT 		'<'
#define WORMRIGHT 		'>'
#define WORMSEGMENT 'o'

#define WORLD_CORNER 			'+'
#define WORLD_HORIZONTAL_WALL	'-'
#define WORLD_VERTICAL_WALL 	'|'
#define WORLD_EMPTY_SPACE		' '


/*WORM BEHAVIOR*/
void InputRespond (char *wrm_headTurn);
int DidILose(
	int wrm_head_Y, int wrm_head_X, int wrm_allPos_Y[], 
	int wrm_allPos_X[], int wrm_len, int world_Y, int world_X);
void IsItFood(
	int *wrm_head_Y, int *wrm_head_X, 
	int wrm_allPos_Y[], int wrm_allPos_X[], int *wrm_len, int food_max,
	int *food_no, int worldSize, int food_YX[][worldSize], int world_Y, int world_X);
void AddSegment(int newLoc_Y, int newLoc_X, int *wrm_len, int wrm_allPos_Y[], int wrm_allPos_X[]);
void MoveWorm(
	int *wrm_head_Y, int *wrm_head_X, char *wrm_headTurn, 
	int wrm_allPos_Y[], int wrm_allPos_X[], int wrm_len);
void MoveSegments(int pos_Y, int pos_X, int wrm_allPos_Y[], int wrm_allPos_X[], int wrm_len);

/*WORLD_GEN*/
void SpawnFood(
	int wrm_head_Y, int wrm_head_X, 
	int wrm_allPos_Y[], int wrm_allPos_X[], int wrm_len, int world_Y, 
	int world_X, int food_max, int *food_no, int worldSize, int food_YX[][worldSize]);
void PlaceWorm(
	int *wrm_head_Y, int *wrm_head_X, char *wrm_headTurn, 
	int wrm_allPos_Y[], int wrm_allPos_X[], int *wrm_len, int world_Y, int world_X);
void CreateWorld (int world_Y, int world_X, char world[][world_X]);

/*GRAPHICS*/
void DrawAssets(
	int wrm_head_Y, int wrm_head_X, char wrm_headTurn, int wrm_allPos_Y[], 
	int wrm_allPos_X[], int wrm_len, int food_no, int worldSize, int food_YX[][worldSize]);
void Draw2dArray (int arr_Y, int arr_X, char arr[][arr_X]);

void CharArrayColumnFill_2D(
	int lineNo, int startCol, int endCol, 
	int total_cols, char Array_2D[][total_cols], char fillWithMe);
void IntArrayColumnFill_2D(
	int lineNo, int startCol, int endCol, 
	int total_cols, int Array_2D[][total_cols], int fillWithMe);
void ArrayFill_Int (int array[], int array_length, int fill_with_this);
