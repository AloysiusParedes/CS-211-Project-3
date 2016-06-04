/****HEADER
  AUTHOR: Aloysius Paredes
  CLASS: CS211
  PROGRAM: Project 3
	A C program that will find its way through a maze using the depth-first search algorithm. This program takes input from a file where the filename is specified in the command line argument that contains two integer values per line of input.
*/
#include <stdio.h>
#include <stdlib.h>

//GLOBAL CONSTANTS
int const TRUE = 1;
int const FALSE = 0;
int debugMode = 0;

//****STRUCT DEFINITIONS****
typedef struct mazeStruct{
	char **arr;  		/* allows for a maze of any size */
	int **visited;		//tracks positions visited
	int xsize, ysize; 	//tracks size of maze
	int xstart, ystart; //tracks starting position
	int xend, yend; 	//tracks ending position
} maze;

typedef struct stackStruct{
	int xpos, ypos;				//tracks current position in stack
	struct stackStruct *next;	//pointer to next node
}stack;

//****FUNCTION PROTOTYPES****
void readFromFile(FILE *src,int *xsize, int *ysize, int *xstart, int *ystart, int *xend, int*yend);
void checkValid(int xsize, int ysize, int xstart, int ystart, int xend, int yend);
void printMazeInfo(int xsize, int ysize, int xstart, int ystart, int xend, int yend);
void initializeMaze(maze *maze, int xsize, int ysize, int xstart, int ystart, int xend, int yend);
void markBlocked(FILE *src, maze *maze, int *xpos, int *ypos, int xstart, int ystart, int xend, int yend, int xsize, int ysize);
void printMaze(maze *maze);
void push(stack **head, int xpos, int ypos);
void pop(stack **head, int debugMode);
int checkEmpty(stack *head);
void displayStack(stack *head);
void freeMaze(maze *maze);

int main (int argc, char **argv){
	//Variables for maze
	int xpos, ypos, xsize = -1, ysize = -1, xstart = -1, ystart = -1, xend = -1, yend = -1;
	//Variables for loops
	int i, j;
	char **string; //to hold argv
	
	/* initialize maze */
	maze m1;
	
	/* initialize stack */
	stack *stackHead = NULL;

	FILE *src;

	//sets debug mode
	for(i = 0; i < argc; i++){
		if(strcmp(argv[i], "-d") == 0){
			printf("\nDEBUG MODE ON\n");
			debugMode = TRUE;
		}
		else 
			string = argv[i];
	}
	argv[1] = string;
	
	/* verify the proper number of command line arguments were given */
	if(argc < 2) {
		printf("Usage: %s <input file name>\n", argv[0]);
		exit(-1);
	}

	/* Try to open the input file. */
	if ( ( src = fopen( argv[1], "r" )) == NULL ){
		printf ( "Can't open input file: %s\n", argv[1] );
		exit(-1);
	}
	
	/* read in the size, starting and ending positions in the maze */
	readFromFile(src, &xsize, &ysize, &xstart, &ystart, &xend, &yend);
	
	/*check to see if values are valid */
	checkValid(xsize, ysize, xstart, ystart, xend, yend);
	
	/* print them out to verify the input */	
	printMazeInfo(xsize, ysize, xstart, ystart, xend, yend);
	
	/*initialize maze*/
	initializeMaze(&m1, xsize, ysize, xstart, ystart, xend, yend);

	/* mark the blocked positions in the maze with *'s */
	markBlocked(src, &m1, &xpos, &ypos, xstart, ystart, xend, yend, xsize, ysize);
	
	/* print out the initial maze */
	printf("\t ====\n\t|MAZE|\n\t ====\n");
	printMaze(&m1);
	
	/* initialize stack to start positions */
	push(&stackHead, xstart, ystart);

	//variables to keep track of current positions
	int xcurr = xstart, ycurr = ystart;
	/* loop while stack is not empty and end has not been found */
	while(checkEmpty(stackHead) != 1){
		//check if top of the stack is the end position
		if(stackHead->xpos == m1.xend && stackHead->ypos == m1.yend)
			break;
		
		//check if top of the stack has unvisited and unblocked neighbor
		if(m1.visited[xcurr][ycurr + 1] == 0){ //check right
			//set new ycurr
			ycurr++;
			//push coordinates of unvisited neighbor on the stack
			push(&stackHead, xcurr, ycurr);
			//mark the unvisited neighbor as visited
			m1.visited[xcurr][ycurr] = 1;
			m1.arr[xcurr][ycurr] = '>';
		}
		else if(m1.visited[xcurr + 1][ycurr] == 0){ //check down
			//set new xcurr
			xcurr++;
			//push coordinates of unvisited neighbor on the stack
			push(&stackHead, xcurr, ycurr);
			//mark the unvisited neighbor as visited
			m1.visited[xcurr][ycurr] = 1;
			m1.arr[xcurr][ycurr] = '-';
		}
		else if(m1.visited[xcurr][ycurr - 1] == 0){ //check left
			//set new ycurr
			ycurr--;
			//push coordinates of unvisited neighbor on the stack
			push(&stackHead, xcurr, ycurr);
			//mark the unvisited neighbor as visited
			m1.visited[xcurr][ycurr] = 1;
			m1.arr[xcurr][ycurr] = '<';
		}
		else if(m1.visited[xcurr - 1][ycurr] == 0){ //check up
			//set new xcurr
			xcurr--;
			//push coordinates of unvisited neighbor on the stack
			push(&stackHead, xcurr, ycurr);
			//mark the unvisited neighbor as visited
			m1.visited[xcurr][ycurr] = 1;
			m1.arr[xcurr][ycurr] = '+';
		}
		
		else{ //no positions available. go back to previous
			pop(&stackHead, debugMode); 
			if(checkEmpty(stackHead)) //check for no solution
				continue;
			xcurr = stackHead->xpos;
			ycurr = stackHead->ypos;
		}
	}
	
	//check if stack is empty
	if(checkEmpty(stackHead)){
		printf("\nMAZE HAS NO SOLUTION\n\n");
		exit(1);
	}
	else{ //the items on the stack contain the coordinates of the solution from the end of the maze to start of the maze
		printf("\t ========\n\t|SOLUTION|\n\t ========\n");
		printMaze(&m1);
		printf("\n");
		displayStack(stackHead);
		exit(1);
	}
	
	/* deallocate memory */
	freeMaze(&m1);
	
}//end main

//****FUNCTION DEFINITIONS****
void readFromFile(FILE *src, int *xsize, int *ysize, int *xstart, int *ystart, int *xend, int*yend){
	//check xsize and ysize
	while(fscanf (src, "%d %d", &*xsize, &*ysize) != EOF){
		if(*xsize > 0 && *ysize > 0)
			break;
		else{
			printf("Invalid Maze Size: %d, %d\n", *xsize, *ysize);
			*xsize = -1;
			*ysize = -1;
		}
	}
	//check xstart and ystart
	while(fscanf (src, "%d %d", &*xstart, &*ystart) != EOF){
		if(*xstart <= *xsize && *ystart <= *ysize)
			break;
		else{
			printf("Invalid Maze Start Positions: %d, %d\n", *xstart, *ystart);
			*xstart = -1;
			*ystart = -1;
		}
	}
	//check xend and yend
	while(fscanf (src, "%d %d", &*xend, &*yend) != EOF){
		if(*xend <= *xsize && *yend <= *ysize)
			break;
		else{
			printf("Invalid Maze End Positions: %d, %d\n", *xend, *yend);
			*xend = -1;
			*yend = -1;
		}
	}
}//end readFromFile(...)

void checkValid(int xsize, int ysize, int xstart, int ystart, int xend, int yend){
	//check xsize and ysize
	if(xsize == -1 || ysize == -1){
		printf("\nReached end of file without finding valid size, start, and end\n");
		exit(1);
	}
	//check xstart and ystart
	if(xstart == -1 || ystart == -1){
		printf("\nReached end of file without finding valid size, start, and end\n");
		exit(1);
	}
	//check xend and yend
	if(xend == -1 || yend == -1){
		printf("\nReached end of file without finding valid size, start, and end\n");
		exit(1);
	}
}//end checkValid(...)

void printMazeInfo(int xsize, int ysize, int xstart, int ystart, int xend, int yend){
	printf ("\nsize: %d, %d\n", xsize, ysize);
	printf ("start: %d, %d\n", xstart, ystart);
	printf ("end: %d, %d\n\n", xend, yend);
}//end printMazeInfo(...)

void initializeMaze(maze *maze, int xsize, int ysize, int xstart, int ystart, int xend, int yend){
	int i, j;
	
	//initialize maze size, start, and end positions
	maze->xsize = xsize;
	maze->ysize = ysize;
	maze->xstart = xstart;
	maze->ystart = ystart;
	maze->xend = xend;
	maze->yend = yend;
	
	//allocate arr and visited arrays based on xsize and ysize
	maze->arr = (char **) malloc(sizeof(char *) * (xsize + 2));
	maze->visited = (int **) malloc(sizeof(int *) * (xsize + 2));
	for(i = 0; i < xsize + 2; i++){
		maze->arr[i] = (char *) malloc(sizeof(char) * (ysize + 2));
		maze->visited[i] = (int *) malloc(sizeof(int) * (ysize + 2));
	}
		
	/* initialize the maze to empty and visited to UNVISITED*/
	for (i = 0; i < maze->xsize + 2; i++){
		for (j = 0; j < maze->ysize + 2; j++){
			maze->arr[i][j] = '.';
			maze->visited[i][j] = 0;
		}
	}
		
	/* mark the borders of the maze with *'s and blocked positions as VISITED*/
	for (i=  0; i < maze->xsize + 2; i++){
		maze->arr[i][0] = '*';
		maze->arr[i][maze->ysize + 1] = '*';
		maze->visited[i][0] = 1;
		maze->visited[i][maze->ysize + 1] = 1;
	}
	for (i = 0; i < maze->ysize + 2; i++){
		maze->arr[0][i] = '*';
		maze->arr[maze->xsize + 1][i] = '*';
		maze->visited[0][i] = 1;
		maze->visited[maze->xsize + 1][i] = 1;
	}	
	/* mark the starting and ending positions in the maze */
	maze->arr[maze->xstart][maze->ystart] = 'S';
	maze->arr[maze->xend][maze->yend] = 'E';
	
	/* mark the start position as visited */
	maze->visited[maze->xstart][maze->ystart] = 1;
}//end initializeMaze(...)

void markBlocked(FILE *src, maze *maze, int *xpos, int *ypos, int xstart, int ystart, int xend, int yend, int xsize, int ysize){
	//loop to accept blocked positions
	while (fscanf (src, "%d %d", &*xpos, &*ypos) != EOF){
		//check if blocking start position
 		if(*xpos == xstart && *ypos == ystart){
			printf("Invalid Blocking Start Position: %d, %d\n", *xpos, *ypos);
			continue;
		}
		//check if blocking end position
		if(*xpos == xend && *ypos == yend){
			printf("Invalid Blocking End Position: %d, %d\n", *xpos, *ypos);
			continue; 
		}
		//check if blocking outside of maze
		if(*xpos > xsize || *ypos > ysize){
			printf("Invalid Blocking Outside Of Maze: %d, %d\n", *xpos, *ypos);
			continue;
		}
		//check if blocking non-positive index of array
		if(*xpos < 1 || *ypos < 1){
			printf("Invalid Blocking Of Maze: %d, %d\n", *xpos, *ypos);
			continue;
		}
		maze->arr[*xpos][*ypos] = '*';
		maze->visited[*xpos][*ypos] = 1;
	}
}//end markBlocked(...)

void printMaze(maze *maze){
	int i, j;

	for (i = 0; i < maze->xsize + 2; i++){
		for (j = 0; j < maze->ysize + 2; j++)
			printf ("%c", maze->arr[i][j]);
		printf("\t");
		for (j = 0; j < maze->ysize + 2; j++)
			printf ("%d", maze->visited[i][j]);
		printf("\n");
	}	
}//end printMaze(...)

void push(stack **head, int xpos, int ypos){
	//create new node
	stack *temp = (stack *) malloc(sizeof(stack));
	//set node values
	temp->xpos = xpos;
	temp->ypos = ypos;
	//set next pointer
	temp->next = *head;
	//set new head
	*head = temp;
}//end push(...)

void pop(stack **head, int debugMode){
	//printf("\nPOPPED\n");
	//set temp head
	stack *temp = *head;
	if(debugMode == TRUE)
		printf("Popping off stack: (%d, %d)\n", temp->xpos, temp->ypos);
	//check to see if list is empty
	if(*head != NULL){
		//remove from list
		*head = (*head)->next;
		free(temp);
		return;
	}
	//empty
	printf("\nCannot pop from empty stack\n");
	return;
}//end pop(...)

int checkEmpty(stack *head){
	if(head == NULL)
		return 1;
	return 0;
}//end checkEmpty(...)

void displayStack(stack *head){
	stack *temp = head;
	//check to see if stack is empty
	if(checkEmpty(head)){
		printf("\nSTACK EMPTY\n\n");
		return;
	}
	
	while(temp->next != NULL){
		printf("(%d,%d)<-", temp->xpos, temp->ypos);
		temp = temp->next;
	}
	printf("(%d,%d)\n", temp->xpos, temp->ypos);
}//end displayStack(...)

void freeMaze(maze *maze){
	int i;
	for(i = 0; i < maze->xsize + 2; i++){
		free(maze->arr[i]);
		free(maze->visited[i]);
	}
	free(maze->arr);
	free(maze->visited);
}//end freeMaze(...)