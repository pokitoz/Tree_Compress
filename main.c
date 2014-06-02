#include <stdio.h>
#include <stdlib.h>

#define SIZE 512
#define MAX_LEVEL 9

#define VARIANCE_LEVEL 100

#define NO_VALUE 256

// ------------------------------------------------
// STRUCTURES

typedef struct Node4 Node4;

struct Node4{

	Node4* children[4]; //NW, NE, SW, SE
	Node4* parent;
	int value;
    int level;

};


// ------------------------------------------------
// GLOBAL VARIABLES

const char namePGM[100] = "baboon.pgm";

// ------------------------------------------------
// FUNCTION DECLARATIONS

void printArray(int array[SIZE][SIZE]);
void writeArray(int array[SIZE][SIZE], const char name[]);
void fillArray(int array_picture[SIZE][SIZE]);
int isUnicolored(int array[SIZE][SIZE] , int x1, int y1, int x2, int y2);
void TREE_fillTree(Node4* root, int array_picture[SIZE][SIZE]);
int power(int level);
Node4* buildChildren(int array_picture[SIZE][SIZE], Node4* parent, int x, int y);
void buildArray(int array[SIZE][SIZE], Node4 *node, int x, int y);

void cleanArray(int array[SIZE][SIZE]);
void freeTree(Node4* node);

double computeEsperance2(int values[], int size);
double computeVariance2(int values[], int size);
double computeEsperance(Node4* node);
double computeVariance(Node4* node);

void TREE_compressTree(Node4* node);
void TREE_rotateTree90(Node4* node);
void TREE_rotateTree(Node4* node, int times);
		
int main(){

    int array_picture[SIZE][SIZE]; 		//For windows, if the heap is too small, do calloc(512*512, sizeof(int))
    cleanArray(array_picture);

    fillArray(array_picture);
	
    Node4 root;

    TREE_fillTree(&root, array_picture);

    cleanArray(array_picture);

    buildArray(array_picture, &root, 0, 0);
	writeArray(array_picture, "resultfromTree.pgm");
    cleanArray(array_picture);

    TREE_compressTree(&root);
    buildArray(array_picture, &root, 0, 0);
    writeArray(array_picture, "resultfromCompressedTree1.pgm");

	
	TREE_rotateTree(&root, 2);
	buildArray(array_picture, &root, 0, 0);
    writeArray(array_picture, "resultfromCompressedTree2.pgm");

    freeTree(&root);

    return 0;
}


void cleanArray(int array[SIZE][SIZE]){

    int i, j;

    for (i = 0; i < SIZE; ++i) {
        for (j = 0; j < SIZE; ++j) {
            array[i][j] = 0;
        }
    }

}

void TREE_fillTree(Node4* root, int array_picture[SIZE][SIZE]){

    printf("[Building the tree]\n");

    //The root has no parents
    root->parent = NULL;
    //The level initial is 0
    root->level = 0;

    int color = isUnicolored(array_picture, 0, 0, SIZE, SIZE);
    if(color != NO_VALUE){
        //The picture has the same color in each cell. (Picture black or white)
        //No child since all the quarters are unicolored
        root->children[0] = NULL;
        root->children[1] = NULL;
        root->children[2] = NULL;
        root->children[3] = NULL;

        //The root is a leaf with value the color of the pixel.
        root->value = color;
        return;
    }

    else{
        //A 256 value means that the node is not a leaf. It's a node and has children.
        root->value = NO_VALUE;

        //Build children of the first part

        //Part A
        root->children[0] = buildChildren(array_picture, root, 0, 0);
        //Part B
        root->children[1] = buildChildren(array_picture, root, SIZE/2, 0);
        //Part C
        root->children[2] = buildChildren(array_picture, root, 0, SIZE/2);
        //Part D
        root->children[3] = buildChildren(array_picture, root, SIZE/2, SIZE/2);


        //Can use the function buildChildren bellow to make the same thing..

    }


}


int power(int level){
   return (1<<(MAX_LEVEL-level));
}


Node4* buildChildren(int array_picture[SIZE][SIZE], Node4* parent, int x, int y){

    //Allocate space for the new node.
    Node4* child = malloc(sizeof(Node4));
    //Associate the parent to this node.
    child->parent = parent;
    //We are at a deeper level from the parent.
    child->level = parent->level +1;

    //Store the level in a variable.
    int level = child->level;
    //Currently, the new node has no child.
    child->value = NO_VALUE;

    //If we are at the lowest level. MAX_LEVEL is 9.
    if((MAX_LEVEL-level) == 0){
        //We know that the child value is the color contained in the ceil x,y
        child->value= array_picture[x][y];
        return child;
    }

    int color = isUnicolored(array_picture, x, y, x+power(level), y+power(level));
    //Else, we need to check if the subarray is unicolored.
    if(color != NO_VALUE){
        //The subarray is unicolored, there is no child.
        child->children[0] = NULL;
        child->children[1] = NULL;
        child->children[2] = NULL;
        child->children[3] = NULL;

        //The value of the leaf is contained in the ceil x,y
        child->value = array_picture[x][y];
        child->value = color;

    }else{
        //Otherwise we need to find the child for each of them.
        //Recursion with different arguments...
        child->children[0] = buildChildren(array_picture, child, x,y);
        child->children[1] = buildChildren(array_picture, child, x+power(level+1),y);
        child->children[2] = buildChildren(array_picture, child, x,y+power(level+1));
        child->children[3] = buildChildren(array_picture, child, x+power(level+1),y+power(level+1));
    }

    return child;

}


void fillArray(int array_picture[SIZE][SIZE]){

	FILE* picture = NULL;
    picture = fopen(namePGM, "r");


    if(picture == NULL){
        printf("Impossible to open the file. Abort");
        exit(0);
    }

    char format[5];
    fgets(format, 5, picture);	//Read the format of the picture

    int number_line = 1;

    // Ignore comments
    char c = fgetc(picture);
    char buffer[1000];
    while(c == '#' || c == '\n'){
        fgets (buffer, 1000, picture);
        c = fgetc(picture);
        number_line++;
    }

    fclose(picture);
    picture = fopen(namePGM, "r");
    if(picture == NULL){
        printf("Impossible to open the file. Abort\n");
        exit(0);
    }

    while(number_line != 0){
        fgets (buffer, 1000, picture);
        number_line--;
    }

    int size[2];
    fscanf(picture, "%d %d", &size[0], &size[1]);

    if(size[0] != size[1] || size[0] != SIZE){
        printf("Problem with the picture... Size is incorrect [%d or %d] not equal to SIZE\n", size[0], size[1]);
        exit(0);
    }

    int shade_of_grey = -1; 	//Lol
    fscanf(picture, "%d", &shade_of_grey);
    if(shade_of_grey != 255){
        printf("Problem with the number of grey levels\n");
        exit(0);
    }


    int row;
    int col;

    for(row = 0; row < SIZE; row++){
         for(col = 0; col < SIZE; col++){
            fscanf(picture, "%d", &array_picture[row][col]);

            //Some checking on the values of the image
            if(array_picture[row][col] < 0 || array_picture[row][col] > shade_of_grey){
                printf("Problem with a level of grey: row %d and col %d\n", row,col);
                exit(0);

            }
        }
    }
	fclose(picture);

}


void printArray(int array[SIZE][SIZE]){

	int row;
	int col;
    for(row = 0; row < SIZE; row++){
        for(col = 0; col < SIZE; col++){
			printf("%d ", array[row][col]);
		}

		printf("\n");

	}

}


void writeArray(int array[SIZE][SIZE], const char name[100]){

	FILE* newPicture = NULL;
    newPicture = fopen(name, "w+");

	if(newPicture == NULL){
        printf("Problem during the writing of the array\n");
		exit(0);
	}


	fprintf(newPicture, "P2\n512  512\n255\n");

	int row;
	int col;
    for(row = 0; row < SIZE; row++){
        for(col = 0; col < SIZE; col++){
            fprintf(newPicture, "%d ", array[row][col]);
		}

        fprintf(newPicture, "\n");
	}

	fclose(newPicture);

}


int isUnicolored(int array[SIZE][SIZE], int x1, int y1, int x2, int y2){

    int color = array[x1][y1];
    int row;
    int col;
    for (row = x1; row < x2; ++row) {
        for (col = y1; col < y2; ++col) {

           if(color != array[row][col]){
                return NO_VALUE;
           }
        }
    }

    return color;
}



void buildArray(int array[SIZE][SIZE], Node4* node, int x, int y){

    if(node->value == NO_VALUE){

        buildArray(array, node->children[0], x, y);
        buildArray(array, node->children[1], x + power(node->level+1), y);
        buildArray(array, node->children[2], x, y + power(node->level+1));
        buildArray(array, node->children[3], x + power(node->level+1), y + power(node->level+1));

    }else{

        if(node->level == 9){
            array[x][y] = node->value;
        }else{

            int max_i = x + power(node->level);
            int max_j = y + power(node->level);

            int i;
            int j;

            for (i=x; i < max_i; ++i) {
                for (j=y; j < max_j; ++j) {
                    array[i][j] = node->value;
                }
            }


        }
    }
}

double computeEsperance(Node4* node){

    int values[4];
    int i;

    for (i = 0;  i < 4; i++) {
       values[i] = 0;
       values[i] = node->children[i]->value;
    }

    return computeEsperance2(values, 4);
}


double computeEsperance2(int values[], int size){
    // Esperance of x
    double E_x = 0;

    int i;

    for(i = 0; i < size; i++){
        E_x += values[i];
    }

    E_x = (1.0/size)*E_x;

    return E_x;
}

double computeVariance(Node4* node){

        int values[4];
        int i;

        for (i = 0;  i < 4; i++) {
           values[i] = node->children[i]->value;
        }

       return computeVariance2(values, 4);

}


double computeVariance2(int values[], int size){

    // Esperance of x²
    double E_x_square = 0;
    int i;

    for(i = 0; i < size; i++){
        E_x_square += values[i]*values[i];
    }

    E_x_square = (1.0/size)*E_x_square;

    double E_x = computeEsperance2(values, size);
    double variance = E_x_square - E_x*E_x;

    return variance;

}

void TREE_compressTree(Node4* node){

        if(node == NULL){
            return;
        }

        int i;
        if(node->value == NO_VALUE){
            for (i = 0; i < 4; ++i) {
                 TREE_compressTree(node->children[i]);
            }
        }

        if(node->value != NO_VALUE){
            return;
        }

        for (i = 0; i < 4; ++i) {
             if(node->children[i]->value == NO_VALUE){
                 return;
             }
        }

        //Now we need to try to merge the nodes.
        if(computeVariance(node) < VARIANCE_LEVEL){
                node->value = (int) computeEsperance(node);
                //Free all the children of the node.
                freeTree(node);
        }

}

void freeTree(Node4* node){
        int i;

        if(node != NULL){
            for(i=0; i<4; i++){
                if(node->children[i]->value == NO_VALUE){
                    freeTree(node->children[i]);
                    node->children[i] = NULL;
                }else{
                    free(node->children[i]);
                }
            }
        }
}


void TREE_TREE_rotateTree90(Node4* node){

	if(node == NULL || node->value!=NO_VALUE){
            return;
	}
    Node4* tmp = node->children[0];

	node->children[0] = node->children[2];
	node->children[2] = node->children[3];
	node->children[3] = node->children[1];
	node->children[1] = tmp;
	 
	
	TREE_TREE_rotateTree90(node->children[0]);
	TREE_TREE_rotateTree90(node->children[1]);
	TREE_TREE_rotateTree90(node->children[2]);
	TREE_TREE_rotateTree90(node->children[3]);
	
}


void TREE_rotateTree(Node4* node, int times){
	int i=0;
	for(i=times; i>0; i--){
		TREE_TREE_rotateTree90(node);
	}

}
