#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>

#define SECOND		1000000		// 1 second in NCurses.
#define SHOW_INTRO		TRUE
#define SCORE_MSG		"%d"

#define DIFICULTY_EASY		1
#define DIFICULTY_MEDIUM		2
#define DIFICULTY_HARD		3

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

// Larger dimensions cause stack overflows (malloc* knowledge needed).
#define WORLD_LARGE_Y		24
#define WORLD_LARGE_X		34
#define WORLD_MEDIUM_Y		20
#define WORLD_MEDIUM_X		24
#define WORLD_SMALL_Y		16
#define WORLD_SMALL_X		20


/*GAME STATUS*/
int IsGameOver(
	int wrm_head_Y, int wrm_head_X, int *wrm_allPos_YX[], int wrm_len, int world_Y, int world_X);

void IsItFood(
	int *wrm_head_Y, int *wrm_head_X, int *wrm_allPos_YX[], 
	int *wrm_len, int food_max, int *food_no, int *score, 
	int score_Y, int score_X, int *food_YX[], int world_Y, int world_X);

void InputDificulty (int *dificulty, int *wrm_step_len, int scrnMax_Y, int scrnMax_X);


/*WORM BEHAVIOR*/
void InputRespond (char *wrm_headTurn);

void SegmentAdd (int newLoc_Y, int newLoc_X, int *wrm_len, int *wrm_allPos_YX[]);

void WormMove(
	int *wrm_head_Y, int *wrm_head_X, char *wrm_headTurn, int *wrm_allPos_YX[], int wrm_len);

void SegmentsMove (int pos_Y, int pos_X, int *wrm_allPos_YX[], int wrm_len);


/*WORLD_GEN*/
void FoodSpawn(
	int wrm_head_Y, int wrm_head_X, 
	int *wrm_allPos_YX[], int wrm_len, int world_Y, 
	int world_X, int food_max, int *food_no, int *food_YX[]);

void WormPlace(
	int *wrm_head_Y, int *wrm_head_X, char *wrm_headTurn, 
	int *wrm_allPos_YX[], int *wrm_len, int world_Y, int world_X);

void WorldArrayCreate (int world_Y, int world_X, char *world[]);


/*GRAPHICS*/
void DrawAssets(
	int wrm_head_Y, int wrm_head_X, char wrm_headTurn, 
	int *wrm_allPos_YX[], int wrm_len, int food_no, int *food_YX[]);

void WorldSetDims(
	int *food_max,  int *score_Y, int *score_X, 
	int *world_Y, int *world_X, int scrnMax_Y, int scrnMax_X);

void ShowIntro (int scrnMax_Y, int scrnMax_X);


/*ARRAY MANIPULATION*/

void ArrayDraw_Char_2d (int arr_Y, int arr_X, char arr[][arr_Y]);

void ArrayDraw_Char_Ptr (int arr_Y, int arr_X, char *arr[arr_Y]);

void ArrayFill_Int (int array[], int array_length, int fill_with_this);

void ArrayFillCol_Int_2d(
	int lineNo, int startCol, int endCol, 
	int total_cols, int Array_2D[][total_cols], int fillWithMe);

void ArrayFillCol_Char_Ptr (int lineNo, int startCol, int endCol, char *Array_2D[], char fillWithMe);
