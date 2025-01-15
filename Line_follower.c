#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
//take constant values, dma sucks
//prototyping
#define SIZE1 180
#define SIZE2 120

#define ARBITRARY 700

#define LINE 

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

#define STACKSIZE 80

#define START_X 

#define YES 'y'
#define NO 'n'

/* since this sort of code is more famous for micromouse, it needs to be ported for line follower.
How to do that? I'm gonna assume that wherever the bot encounters an end of black-line in front = wall

Also, there's probably tons of mistakes that will surface as we edit this -- needs a desperate review*/

typedef struct Node {
	//data fields 
	short floodvalue;
	short row;
	short column;
	short visited;

	struct Node *west;
	struct Node *east;
	struct Node *north;
	struct Node *south;
} Node;

typedef struct Maze {
	Node *map[SIZE1][SIZE2];
} Maze;

typedef struct Stack {
	Node *stack[STACKSIZE];	//array of STACKSIZE no. of ptrs
} Stack;

//func protopypes

//stack funcs
Stack *new_Stack();
void delete_Stack(Stack **spp);
int is_empty_Stack(Stack *stack);
void pop(Stack *stack, Node **npp);
void push(Stack *stack, Node *node);

//node funcs
Node *new_Node(const short x, const short y);
void delete_Node(Node **npp);
void set_value(Node *node, const short value);
short get_nearest_neighbour(Node *node); //to determine next cell with the next smallest value

//Maze funcs
Maze *new_Maze();
void explore_build_maze(Maze *maze, Node *this_node, IR_Sensor_Data sensor_data );
void delete_maze(Maze **mpp); //is it required?
void print_maze(const Maze *this_maze); // to check grid, prolly not required

//flood fill funcs
void flood_fill(Node *node, Stack *stack, const short floodfill_flag);
void update_floodvalue (Node * this_node);
bool floodvalue_check(Node *this_node);

//Constructing data structures
Node *new_Node(const short x, const short y) 
{
    Node *this_node = (Node *) malloc(sizeof(Node));
    int goal[] = {SIZE1/2, SIZE2};

    this_node->row = y;
    this_node->column = x;
    this_node->visited = false;

    //flood values pre
    if (x < goal[0] && y < goal[1])
		this_node->floodvalue = (goal[0] - 1 - x) + (goal[1] - 1 - y);
	else if (x < goal[0] && y >= goal[1])
		this_node->floodvalue = (goal[0] - 1 - x) + (y - goal[1]);
	else if (x >= goal[0] && y < goal)
		this_node->floodvalue = (x - goal[0]) + (goal[1] - 1 - y);
	else
		this_node->floodvalue = (x - goal[0]) + (y - goal[1]);

    return this_node;
} 

Maze *new_Maze()
{
    Maze *this_maze = (Maze*)malloc(sizeof(Maze));
    short x,y;

    //Maze matrix, flooded with values without obstacle constraints
    for(x = 0; x < SIZE1; x++)
    {
		for(y = 0; y < SIZE2; y++)
        {
			this_maze->map[x][y] = new_Node(x, y);
        }
    }


    //establishing neighbour relations between nodes, because bhaichara baatne se badhta hai. Creating a gridmap of sorts
    for(x = 0; x < SIZE1; x++) {
		for(y = 0; y < SIZE2; y++) {
            //if first cell, then there is no south so set to null, similarly (too costly to use if else)
			this_maze->map[x][y]->south = (y == 0) ? NULL : (this_maze->map[x][y-1]);
			this_maze->map[x][y]->north = (y == SIZE2-1) ? NULL : (this_maze->map[x][y+1]);
			this_maze->map[x][y]->west = (x==0) ? NULL : (this_maze->map[x-1][y]);
			this_maze->map[x][y]->east = (x == SIZE1-1) ? NULL : (this_maze->map[x+1][y]);
		}
	}

    return this_maze;        
} //maze gridmap is ready with connections

short get_nearest_neighbour (Node * this_node) 
{
   short nearest_neighbour = ARBITRARY; //temp value stored

   //decides who the nearest neighbour is using BFS

   //check west
   if (this_node->west != NULL && (this_node->west->east != NULL) && (this_node->west->floodvalue) < nearest_neighbour)
		nearest_neighbour = this_node->west->floodvalue;

	if (this_node->east != NULL && (this_node->east->west != NULL) && (this_node->east->floodvalue) < nearest_neighbour)
		nearest_neighbour = this_node->east->floodvalue;	

	if (this_node->north != NULL && (this_node->north->south != NULL) && (this_node->north->floodvalue) < nearest_neighbour)
		nearest_neighbour = this_node->north->floodvalue;

	if (this_node->south != NULL && (this_node->south->north != NULL) && (this_node->south->floodvalue) < nearest_neighbour)
		nearest_neighbour = this_node->south->floodvalue;

    return nearest_neighbour; 
 
}

short get_nearest_neighbour_dir(Node * this_node, const short preferred_dir) {

	Node* neighbour = get_nearest_neighbour(this_node);
    short path_count;

    path_count = 0; //count the number of paths available

    /*should I keep the comparison as between nodes 
    like (this_node->north == neighbour) [I think this] or 
    between floodvalues like (this_node->north->floodvalue == neighbour->floodvalue)*/

    if ((this_node->north != NULL) && (this_node->north->floodvalue == neighbour->floodvalue)) 
    	path_count++;
    
  	if ((this_node->east != NULL) && (this_node->east->floodvalue == neighbour->floodvalue)) 
    	path_count++;
  	
  	if ((this_node->south != NULL) && (this_node->south->floodvalue == neighbour->floodvalue)) 
    	path_count++;
    
  	if ((this_node->west != NULL) && (this_node->west->floodvalue == neighbour->floodvalue)) 
    	path_count++;

    //path preference    

    switch (preferred_dir){

    	case NORTH: 
    		if ((this_node->north != NULL) && (this_node->north->floodvalue == neighbour->floodvalue))
    			return NORTH;
    		break;
    	case EAST: 
    		if ((this_node->east != NULL) && (this_node->east->floodvalue == neighbour->floodvalue))
    			return EAST;
    		break;
    	case SOUTH: 
			if ((this_node->south != NULL) && (this_node->south->floodvalue == neighbour->floodvalue))
				return SOUTH;
    		break;
    	case WEST:  
    		if ((this_node->west != NULL) && (this_node->west->floodvalue == neighbour->floodvalue))
    			return WEST;
    		break;

    }



}

bool floodvalue_check(Node * this_node) { //to update or not to update, that is the question and this func will give the ans

	if (get_nearest_neighbor(this_node) + 1 == this_node->floodvalue)
		return true;

	return false;
}

void set_value (Node * this_node, const short value) {
	
	//set the flood value
	this_node->floodvalue = value;
}

void set_visited(Node *this_node) {
	
	//set visited flag flood value to TRUE
	this_node->visited = true;
}

Node* move_dir (Maze * this_maze, short * x, short * y) { 

    Node* this_node, *next_dir;   //this_node node 
 
    this_node = this_maze->map[(*x)][(*y)];
    next_dir = get_nearest_neighbour(this_node);

    if (next_dir == NORTH) 
      (*x) = (*x) - 1;
    else if (next_dir == EAST) 
      (*y) = (*y) + 1;
    else if (next_dir == SOUTH) 
      (*x) = (*x) + 1;
    else if (next_dir == WEST) 
      (*y) = (*y) - 1;

    return next_dir; //call this fcn on a dir variable 
    
}

void update_floodvalue (Node * this_node) {

	//set this node's value to 1 + min open adjascent cell 
	this_node->floodvalue = get_nearest_neighbour(this_node) + 1;

}

void push_open_neighbors (Node * this_node, Stack * this_stack) {

	
	/* A NULL neighbor represents an obstacle/absence of line (disconnected). So assuming that there is a connection
	   if neighbor is accessible, push it onto stack! */
	if (this_node->west != NULL && this_node->west->east != NULL) 
		push (this_stack, this_node->west);

	if (this_node->east != NULL && this_node->east->west != NULL) 
		push (this_stack, this_node->east);

	if (this_node->north != NULL && this_node->north->south != NULL) 
		push (this_stack, this_node->north);

	if (this_node->south != NULL && this_node->south->north != NULL) 
		push (this_stack, this_node->south);

}

//update actual floodfill values
void flood_fill (Node * this_node, Stack * this_stack, const short flood_flag) {

	bool status;

    //to avoid flooding goal value -- ask senior -- check if this_node coordinates are "goal"


    status = floodvalue_check (this_node);
    if (!status) {
		update_floodvalue(this_node); 
		push_open_neighbors(this_node, this_stack); 
	}


} 

void flood_fill_dynamic(Node *this_node, Stack *stack, Maze *this_maze,IR_Sensor_Data sensor_data) {
    // Check if flood value needs updating.
    if (!floodvalue_check(this_node)) {
        update_floodvalue(this_node);
        explore_build_maze(this_maze, this_node, sensor_data); // Dynamically build neighbors.
        push_open_neighbors(this_node, stack); // Push neighbors onto the stack.
    }
}

//this will sever neighbourly ties
void set_wall (Node * this_node, const short dir) {

	switch (dir) {

		case NORTH :
			if (this_node->row != 0) {
				this_node->north = NULL;
                //neighbour, blocked by absence of line "wall"
			} break;

		case SOUTH :
			if (this_node->row != SIZE2 -1) {
				this_node->south = NULL;	
			} break; 

		case EAST : 
			if (this_node->column != SIZE1 - 1) {
				this_node->east = NULL;
			} break;

		case WEST :
			if (this_node->column != 0) { 
				this_node->west = NULL;	
			} break;
	}
}

//dynamic maze building -- suggested by GPT
void explore_build_maze(Maze *maze, Node *this_node, IR_Sensor_Data sensor_data /*placeholder*/) {
    // Create nodes dynamically based on sensor inputs.
    if (sensor_data.front == LINE) {
        if (this_node->north == NULL) {
            Node *new_node = new_Node(this_node->row + 1, this_node->column);
            this_node->north = new_node;
            new_node->south = this_node;
            maze->map[new_node->row][new_node->column] = new_node;
        }
    } else {
        set_wall(this_node, NORTH);
    }

    if (sensor_data.right == LINE) {
        if (this_node->east == NULL) {
            // Create a new node for the east path
            Node *new_node = new_Node(this_node->row, this_node->column + 1);
            this_node->east = new_node;
            new_node->west = this_node; 
            maze->map[new_node->row][new_node->column] = new_node; 
        }
    } else {
        // Set a wall if no path to the east
        set_wall(this_node, EAST);
    }

    // Check for a path to the SOUTH
    if (sensor_data.back == LINE) {
        if (this_node->south == NULL) {
            // Create a new node for the south path
            Node *new_node = new_Node(this_node->row + 1, this_node->column);
            this_node->south = new_node; 
            new_node->north = this_node; 
            maze->map[new_node->row][new_node->column] = new_node; 
        }
    } else {
        // Set a wall if no path to the south
        set_wall(this_node, SOUTH);
    }

    // Check for a path to the WEST
    if (sensor_data.left == LINE) {
        if (this_node->west == NULL) {
            // Create a new node for the west path
            Node *new_node = new_Node(this_node->row, this_node->column - 1); 
            this_node->west = new_node; 
            new_node->east = this_node; 
            maze->map[new_node->row][new_node->column] = new_node; 
        }
    } else {
        // Set a wall if no path to the west
        set_wall(this_node, WEST);
    }

}
