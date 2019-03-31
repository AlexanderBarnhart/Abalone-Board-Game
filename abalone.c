/*
*	Alexander Barnhart
* Abalone Board Game in OpenGL
* ~~ I do not own the original game ~~
*/
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <GL/freeglut_std.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <stdio.h>
#include <math.h>

typedef GLubyte Pixel[3];

struct Space {
	int falloff;
	int hasBlackBall;
	int hasWhiteBall;
	int selected;
	float x;
	float z;
};
//material arrays
//----------------------------------------------------------
GLfloat white_amb[] = {1.0, 1.0, 1.0, 1.0};
GLfloat ball_brightness[] = {10.0};
GLfloat black_amb[] = {0.05, 0.05, 0.05, 1.0};
GLfloat selected_amb[] = {1.0, 0.0, 0.0, 0.5};
GLfloat selected_spec[] = {1.0, 0.0, 0.0, 0.5};
GLfloat green_amb[] = {0.3, 1.0, 0.3, 1.0};
GLfloat green_spec[] = {0.0, 1.0, 0.0, 1.0};
GLfloat ball_spec[] = {1.0, 1.0, 1.0, 1.0};
GLfloat table_diff[] = {0.3, 0.25, 0.1, 1.0};
GLfloat table_spec[] = {1.0, 1.0, 1.0, 1.0};
GLfloat table_amb[] = {0.4, 0.4, 0.4, 1.0};
GLfloat table_brightness[] = {20.0};
GLfloat board_amb[] = {0.53, 0.57, 0.59, 1.0};
GLfloat board_spec[] = {1.0, 1.0, 1.0, 1.0};
GLfloat board_brightness[] = {10.0};
GLfloat space_amb[] = {0.27, 0.28, 0.21, 1.0};
GLfloat space_brightness[] = {5.0};
GLfloat floor_diff[] = {0.5, 0.5, 0.5, 1.0};
GLfloat floor_spec[] = {1.0, 1.0, 1.0, 1.0};
GLfloat floor_brightness[] = {5.0};
GLfloat wall_diff[] = {1.0, 1.0, 1.0, 1.0};
GLfloat wall_spec[] = {1.0, 1.0, 1.0, 1.0};
GLfloat wall_brightness[] = {5.0};
//-----------------------------------------------------------
//piece arrays and game board
//-----------------------------------------------------------
struct Space spaces[11][11];
struct Space piece_buffer[6];
struct Space white_off[6];
struct Space black_off[6];
//------------------------------------------------------------
//texture elements
//------------------------------------------------------------
Pixel *Wood_Image;
Pixel *Board_Image;
Pixel *Floor_Image;
Pixel *Wall_Image;
char *file_wood = "./wood2.ppm";
char *file_floor = "./floor_tile.ppm";
char *file_wall = "./brick_wall.ppm";
//------------------------------------------------------------
float i = 0;
float inc = 5.0;
GLUquadric* quadObj;
int count = 0;
int swtch = -1;
int s_y = 9;	
int s_x = 5;	
int b = 0;
int w = 0;
int turn = 1;	//1 = black's turn, -1 = white's turn
int temp;
int temp2;
int counter;
int white_score = 0;
int black_score = 0;
int rotating = 0;
float rot = 90.0;
int Height;
int Width;
float pan = 150.0;
int invalid = -1;
int delay = 30;
int instruction = -1;
int showText = 1;

//prints current board on terminal, used for debugging.
void displayBoard(){
	printf("Board:\n");
	for(int i = 0; i < 11; i++){
		for(int j = 0; j < 11; j++){
			if(spaces[i][j].hasBlackBall == 1)
				printf("[B]");
			else if(spaces[i][j].hasWhiteBall == 1)
				printf("[W]");
			else if(spaces[i][j].falloff == 1)
				printf("[X]");
			else
				printf("[ ]");
		}
		printf("\n");
	}
}

int allowedSize(int x){
	int r;
	r = 1;
	while(r < x) r=(r<<1);
	if(r==x) return r;	
	else return r>>1;
}
Pixel* readImage(char *Filename){
	Pixel* Image;
	int w,h,max;
	int i,j;
	unsigned int r,g,b;
	int k;
	char ch;
	FILE *fp;
	fp = fopen(Filename,"r");
    fscanf(fp, "P%c\n", &ch);
	if (ch != '3') {
		fprintf(stderr, "Only ascii mode 3 channel PPM files");
		exit(-1);
	}
	ch = getc(fp);
	while (ch == '#') {
      do {
		  ch = getc(fp);
      }
	  while (ch != '\n');
      ch = getc(fp); 
    }
	ungetc(ch, fp);
	fscanf(fp,"%d",&w);
	fscanf(fp,"%d",&h);
	fscanf(fp,"%d",&max);
	Width = allowedSize(w);
	Height = allowedSize(h);
	Image = (Pixel *)malloc(Width*Height*sizeof(Pixel));
	unsigned int rc, gc, bc;
	for(i=0;i<Height;++i){
		for(j=0;j<Width;++j) {
			fscanf(fp,"%d %d %d",&rc,&gc,&bc);
			k = i*Width+j; 
			(*(Image+k))[0] = (GLubyte)rc;
			(*(Image+k))[1] = (GLubyte)gc;
			(*(Image+k))[2] = (GLubyte)bc;
		}
		for(j=Width; j<w; ++j) fscanf(fp,"%d %d %d",&rc,&gc,&bc);
	}
	fclose(fp);
	return Image;
}

void init(){
	quadObj = gluNewQuadric();
	gluQuadricNormals(quadObj, GLU_SMOOTH);	
	
	GLfloat light_pos[] = {1.0, 1.0, 1.0, 0.0};
	GLfloat diffuse_color[] = {0.9, 1.0, 0.8, 1.0};
	GLfloat ambient_color[] = {0.4, 0.4, 0.4, 1.0};
	GLfloat specular_color[] = {1.0, 1.0, 1.0, 1.0};
	
	//begin lighting control
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_color);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_color);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular_color);
	//end lighting control

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glEnable(GL_TEXTURE_2D);

	Wood_Image = readImage(file_wood);
	Wall_Image = readImage(file_wall);
	Floor_Image = readImage(file_floor);

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		
	//initialize arrays for knocked off pieces
	for(int i = 0; i < 6; i++){
		white_off[i].hasWhiteBall;
		white_off[i].x = 27.0;
		white_off[i].z = -15.0+(5*i);
		black_off[i].hasBlackBall;
		black_off[i].x = -27.0;
		black_off[i].z = -15.0+(5*i);
	}
	//initialize game board array	
	for(int i = 0; i < 11; i++){
		for(int j = 0; j < 11; j++){
			spaces[i][j].falloff = 0;
			spaces[i][j].hasBlackBall = 0;
			spaces[i][j].hasWhiteBall = 0;
			spaces[i][j].x = 0.0;
			spaces[i][j].z = 0.0;
			spaces[i][j].selected = 0.0;
		}
	}
	//setting "falloff" spaces
	for(int i = 0; i < 11; i++){
		spaces[i][10].falloff = 1;
		spaces[i][0].falloff = 1;
	}
	for(int i = 0; i < 11; i++){
		spaces[0][i].falloff = 1;
		spaces[10][i].falloff = 1;
	}
	for(int i = 1; i < 5; i++){
		spaces[1][10-i].falloff = 1; spaces[9][i].falloff = 1;
	}
	for(int i = 1; i < 4; i++){
		spaces[2][10-i].falloff = 1; spaces[8][i].falloff = 1;
	}
	for(int i = 1; i < 3; i++){
		spaces[3][10-i].falloff = 1; spaces[7][i].falloff = 1;
	}
	for(int i = 1; i < 2; i++){
		spaces[4][10-i].falloff = 1; spaces[6][i].falloff = 1;
	}

	//setting initial white and black ball positions
	for(int i = 5; i < 10; i++){
		spaces[9][i].hasBlackBall = 1; spaces[1][10-i].hasWhiteBall = 1;
	}
	for(int i = 4; i < 10; i++){
		spaces[8][i].hasBlackBall = 1; spaces[2][10-i].hasWhiteBall = 1;
	}
	for(int i = 5; i < 8; i++){
		spaces[7][i].hasBlackBall = 1; spaces[3][10-i].hasWhiteBall = 1;
	}

	//set (x,z) coordinates of all spaces
	float right = 0;
	for(int i = 5; i < 10; i++){
		spaces[1][i-4].x = 20.0;
		spaces[1][i-4].z = -10+right;
		spaces[9][i].x = -20.0;
		spaces[9][i].z = -10+right;
		right += inc;
	}
	right = 0;
	for(int i = 4; i < 10; i++){
		spaces[2][i-3].x = 15.0;
		spaces[2][i-3].z = -12.5+right;
		spaces[8][i].x = -15.0;
		spaces[8][i].z = -12.5+right;
		right += inc;
	}
	right = 0;
	for(int i = 3; i < 10; i++){
		spaces[3][i-2].x = 10.0;
		spaces[3][i-2].z = -15.0+right;
		spaces[7][i].x = -10.0;
		spaces[7][i].z = -15.0+right;
		right += inc;
	}
	right = 0;
	for(int i = 2; i < 10; i++){
		spaces[4][i-1].x = 5.0;
		spaces[4][i-1].z = -17.5+right;
		spaces[6][i].x = -5.0;
		spaces[6][i].z = -17.5+right;
		right += inc;
	}
	right = 0;
	for(int i = 1; i < 10; i++){
		spaces[5][i].x = 0.0;
		spaces[5][i].z = -20.0+right;
		right += inc;
	}
	//set first selected piece
	spaces[s_y][s_x].selected = 1.0;
}
void drawBall(int color){
	//draw a ball
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ball_spec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, ball_brightness);
	//white ball
	if(color == 0){
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white_amb);
		gluSphere(quadObj, 2.0, 60.0, 60.0);
	}
	//black ball
	else if(color == 1){
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black_amb);
		gluSphere(quadObj, 2.0, 60.0, 60.0);
	}
	//selection ball
	else if(color == 2){
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, selected_amb);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, selected_spec);
		gluSphere(quadObj, 2.05, 60.0, 60.0);
	}
	else if(color == 3){
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, green_amb);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, green_spec);
		gluSphere(quadObj, 2.05, 60.0, 60.0);
	}
}
void drawBoard(){
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Board_Image);
	//draw octagonal game board
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, board_amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, board_brightness);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, board_spec);
	
	glNormal3f(0,1,0);
	glBegin(GL_POLYGON);
		glVertex3f(0, 0.1, -24); 
		glVertex3f(24, 0.1, -12); 
		glVertex3f(24, 0.1, 12); 
		glVertex3f(0, 0.1, 24);
		glVertex3f(-24, 0.1, 12); 
		glVertex3f(-24, 0.1, -12);
	glEnd();
	
	glFrontFace(GL_CCW);
	//loop through the board and draw all pieces that need to be drawn
	for(int i = 0; i < 11; i++){
		for(int j = 0; j < 11; j++){

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, space_amb);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, space_brightness);
		glNormal3f(0,1,0);
		glBegin(GL_POLYGON);	
		for(double x = 0; x < 3.14159254*2; x += .1)
			glVertex3f((cos(x)*2)+spaces[i][j].x, 0.11,(-sin(x)*2)+spaces[i][j].z);
		glEnd();

			if(spaces[i][j].selected == 1 && swtch > 0 && invalid < 0){
				glPushMatrix();
				glTranslatef(spaces[i][j].x, 2, spaces[i][j].z);
				drawBall(3);
				glPopMatrix();
			}	
			if(spaces[i][j].selected == 1 && invalid > 0){
				glPushMatrix();
				glTranslatef(spaces[i][j].x, 2, spaces[i][j].z);
				drawBall(2);
				glPopMatrix();
			}	
			else if(spaces[i][j].hasWhiteBall == 1){
				glPushMatrix();
				glTranslatef(spaces[i][j].x, 2, spaces[i][j].z);
				drawBall(0);
				glPopMatrix();
			}	
			else if(spaces[i][j].hasBlackBall == 1){
				glPushMatrix();
				glTranslatef(spaces[i][j].x, 2, spaces[i][j].z);
				drawBall(1);
				glPopMatrix();
			}	
		}
	}
}
void drawOutLines(){
	//draw black and white balls that are out
	for(int i = 0; i < black_score; i++){
		glPushMatrix();
		glTranslatef(white_off[i].x, 2, white_off[i].z);
		drawBall(0);
		glPopMatrix();
	}
	for(int i = 0; i < white_score; i++){
		glPushMatrix();
		glTranslatef(black_off[i].x, 2, black_off[i].z);
		drawBall(1);
		glPopMatrix();
	}
}

void drawWalls(){
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, wall_diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, wall_brightness);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, wall_spec);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Wall_Image);
	
	glNormal3f(1,0,0);
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(-200,-50,-200); 
		glTexCoord2f(3.0,0.0); glVertex3f(-200,-50,200); 
		glTexCoord2f(3.0,3.0); glVertex3f(-200,100,200); 
		glTexCoord2f(0.0,3.0); glVertex3f(-200,100,-200);
	glEnd();

	glNormal3f(0,0,-1);	
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(-200,-50,200); 
		glTexCoord2f(3.0,0.0); glVertex3f(200,-50,200); 
		glTexCoord2f(3.0,3.0); glVertex3f(200,100,200); 
		glTexCoord2f(0.0,3.0); glVertex3f(-200,100,200);
	glEnd();

	glNormal3f(-1,0,0);	
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(200,-50,200); 
		glTexCoord2f(3.0,0.0); glVertex3f(200,-50,-200); 
		glTexCoord2f(3.0,3.0); glVertex3f(200,100,-200); 
		glTexCoord2f(0.0,3.0); glVertex3f(200,100,200);
	glEnd();

	glNormal3f(0,0,1);	
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(200,-50,-200); 
		glTexCoord2f(3.0,0.0); glVertex3f(-200,-50,-200); 
		glTexCoord2f(3.0,3.0); glVertex3f(-200,100,-200); 
		glTexCoord2f(0.0,3.0); glVertex3f(200,100,-200);
	glEnd();
}

void drawFloor(){
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, floor_diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, floor_spec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, floor_brightness);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Floor_Image);
	
	glNormal3f(0,1,0);
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(-200,-50,-200); 
		glTexCoord2f(3.0,0.0); glVertex3f(200,-50,-200); 
		glTexCoord2f(3.0,3.0); glVertex3f(200,-50,200); 
		glTexCoord2f(0.0,3.0); glVertex3f(-200,-50,200);
	glEnd();
}

void drawTableLeg(){
	glNormal3f(1,0,0);
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(2.5,-5,2.5); 
		glTexCoord2f(0.5,0.0); glVertex3f(2.5,-5,-2.5); 
		glTexCoord2f(0.5,0.5); glVertex3f(2.5,-50,-2.5); 
		glTexCoord2f(0.0,0.5); glVertex3f(2.5,-50,2.5);
	glEnd();

	glNormal3f(0,0,1);
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(-2.5,-5,2.5); 
		glTexCoord2f(0.5,0.0); glVertex3f(2.5,-5,2.5); 
		glTexCoord2f(0.5,0.5); glVertex3f(2.5,-50,2.5); 
		glTexCoord2f(0.0,0.5); glVertex3f(-2.5,-50,2.5);
	glEnd();

	glNormal3f(-1,0,0);
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(-2.5,-5,-2.5); 
		glTexCoord2f(0.5,0.0); glVertex3f(-2.5,-5,2.5); 
		glTexCoord2f(0.5,0.5); glVertex3f(-2.5,-50,2.5); 
		glTexCoord2f(0.0,0.5); glVertex3f(-2.5,-50,-2.5);
	glEnd();

	glNormal3f(0,0,-1);
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(2.5,-5,-2.5); 
		glTexCoord2f(0.5,0.0); glVertex3f(-2.5,-5,-2.5); 
		glTexCoord2f(0.5,0.5); glVertex3f(-2.5,-50,-2.5); 
		glTexCoord2f(0.0,0.5); glVertex3f(2.5,-50,-2.5);
	glEnd();

}

void drawTable(){
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, table_amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, table_diff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, table_spec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, table_brightness);
	
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, Wood_Image);
	
	glFrontFace(GL_CCW);
	glNormal3f(-1,0,0);
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(-55,0,55); 
		glTexCoord2f(1.0,0.0); glVertex3f(-55,0,-55); 
		glTexCoord2f(1.0,1.0); glVertex3f(-55,-5,-55); 
		glTexCoord2f(0.0,1.0); glVertex3f(-55,-5,55);
	glEnd();

	glNormal3f(0,0,-1);
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(-55,0,-55); 
		glTexCoord2f(1.0,0.0); glVertex3f(55,0,-55); 
		glTexCoord2f(1.0,1.0); glVertex3f(55,-5,-55); 
		glTexCoord2f(0.0,1.0); glVertex3f(-55,-5,-55);
	glEnd();
	
	glNormal3f(1,0,0);
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(55,0,-55); 
		glTexCoord2f(1.0,0.0); glVertex3f(55,0,55); 
		glTexCoord2f(1.0,1.0); glVertex3f(55,-5,55); 
		glTexCoord2f(0.0,1.0); glVertex3f(55,-5,-55);
	glEnd();

	glNormal3f(0,0,1);
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(55,0,55); 
		glTexCoord2f(1.0,0.0); glVertex3f(-55,0,55); 
		glTexCoord2f(1.0,1.0); glVertex3f(-55,-5,55); 
		glTexCoord2f(0.0,1.0); glVertex3f(55,-5,55);
	glEnd();

	glFrontFace(GL_CW);
	glNormal3f(0,1,0);
	glBegin(GL_POLYGON);
		glTexCoord2f(0.0,0.0); glVertex3f(-55,0,-55); 
		glTexCoord2f(1.0,0.0); glVertex3f(55,0,-55); 
		glTexCoord2f(1.0,1.0); glVertex3f(55,0,55); 
		glTexCoord2f(0.0,1.0); glVertex3f(-55,0,55);
	glEnd();

	//table legs
	glPushMatrix();
	glTranslatef(-45,0,45);
	glRotatef(45,0,0,0);
	drawTableLeg();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(45,0,45);
	glRotatef(45,0,0,0);
	drawTableLeg();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-45,0,-45);
	glRotatef(45,0,0,0);
	drawTableLeg();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(45,0,-45);
	glRotatef(45,0,0,0);
	drawTableLeg();
	glPopMatrix();
}
void printtext(){
 /*
	* code for printing text to the screen found here:
	* http://programmingexamples.net/wiki/OpenGL/Text
	*/	
		char *StringW = "WHITE SCORE: ";
		char *StringB = "BLACK SCORE: ";
		char *StringInstruct = "[Z] for instructions";
		char *StringInstructNo = "[Z] to close instructions";
		char *TurnW = "WHITE'S TURN";
		char *TurnB = "BLACK'S TURN";
		char *InvalidMessage = "INVALID MOVE";
		//instruction strings
		char *StringI = "[H,U,I,K,M,n] = movement";
		char *StringII = "[W,A,S,D] = move piece selection";
		char *StringIII = "[Q] to quit";
		char *StringIV = "Objective: push the enemy pieces off the board.";
		char *StringV = "If you knock of 6 enemy pieces, you win.";
		char *StringVI = "You cannot move more than 3 of your pieces at a time.";
		char *StringVII = "You cannot push a row of enemy pieces larger than yours.";
		//end instruction strings
		char *ScoreW;
		char *ScoreB;

		switch(white_score){
			case 0:
				ScoreW = "0"; break;
			case 1:
				ScoreW = "1"; break;
			case 2:
				ScoreW = "2"; break;
			case 3:
				ScoreW = "3"; break;
			case 4:
				ScoreW = "4"; break;
			case 5:
				ScoreW = "5"; break;
			case 6:
				ScoreW = "VICTORY"; break;
		}	
		switch(black_score){
			case 0:
				ScoreB = "0"; break;
			case 1:
				ScoreB = "1"; break;
			case 2:
				ScoreB = "2"; break;
			case 3:
				ScoreB = "3"; break;
			case 4:
				ScoreB = "4"; break;
			case 5:
				ScoreB = "5"; break;
			case 6:
				ScoreB = "VICTORY"; break;
		}		
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 700, 0, 700, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
		glDisable(GL_LIGHTING);
		glColor3f(1.0, 1.0, 1.0);
    glRasterPos2i(1,5);
    for (int i=0; i<14; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, StringW[i]);
    glRasterPos2i(140,5);
    for (int i=0; i<1; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ScoreW[i]);
		glRasterPos2i(1,50);
    for (int i=0; i<14; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, StringB[i]);
    glRasterPos2i(140,50);
    for (int i=0; i<1; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ScoreB[i]);
    glColor3f(1.0, 0.0, 0.0);
		if(invalid > 0){
    	glRasterPos2i(1,100);
			for (int i=0; i<12; i++)
		      glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, InvalidMessage[i]);
		}

		glColor3f(0.0, 0.0, 0.0);
		if(instruction < 0){
			glRasterPos2i(1,680);
		  for (int i=0; i<20; i++)
		      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, StringInstruct[i]);
			glRasterPos2i(260,650);
			if(turn > 0){		  
				for (int i=0; i<12; i++)
				    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, TurnB[i]);
			}
			else{
				for (int i=0; i<12; i++)
				    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, TurnW[i]);
			}
		}
		else{
			glRasterPos2i(1,680);
		  for (int i=0; i<24; i++)
		      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, StringI[i]);
			glRasterPos2i(1,650);
		  for (int i=0; i<32; i++)
		      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, StringII[i]);
			glRasterPos2i(1,620);
		  for (int i=0; i<11; i++)
		      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, StringIII[i]);
			glRasterPos2i(1,590);
		  for (int i=0; i<47; i++)
		      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, StringIV[i]);
			glRasterPos2i(1,560);
		  for (int i=0; i<40; i++)
		      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, StringV[i]);
			glRasterPos2i(1,530);
		  for (int i=0; i<53; i++)
		      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, StringVI[i]);
			glRasterPos2i(1,500);
		  for (int i=0; i<57; i++)
		      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, StringVII[i]);
			glRasterPos2i(450,680);
		  for (int i=0; i<25; i++)
		      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, StringInstructNo[i]);
		}
		glEnable(GL_LIGHTING);
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
void display(){	
	glEnable(GL_NORMALIZE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_MULTISAMPLE);

	gluLookAt(0.0, 70.0+pan, 70.0,		
		      	0.0, 0.0, 0.0,					
		     	 	0.0, 1.0, 0.0);		
	
	glRotatef(rot, 0, 1, 0);
	//rot is based on whose turn it is
	
	drawWalls();
	drawFloor();
	drawTable();
	drawBoard();
	drawOutLines();
	if(showText > 0)
		printtext();
	glFlush();
	glutSwapBuffers();
}

void reshape(int w, int h){
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 1.0, 1.0, 1000.0);
}
int validateLeft()		//check if left move is allowed
{
	//1 = valid, -1 = invalid
	b = 0;
	w = 0;
	if(spaces[s_y][s_x].hasWhiteBall != 1 && spaces[s_y][s_x].hasBlackBall != 1)
		return -1;	//can't move nothing
	if(turn == 1){
		if(spaces[s_y][s_x].hasWhiteBall == 1)
			return -1;	//black can't move white ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y][s_x-i].hasBlackBall == 1){
				b++;
				if(b > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y][s_x-i].hasWhiteBall != 1 && spaces[s_y][s_x-i].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y][s_x-i].hasWhiteBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y][s_x-j].hasWhiteBall == 1){
				w++;
				if(w == b)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y][s_x-j].hasWhiteBall != 1 && spaces[s_y][s_x-j].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y][s_x-j].hasBlackBall)
				return -1;
		}
	}
	else if(turn == -1){
		if(spaces[s_y][s_x].hasBlackBall == 1)
			return -1;	//white can't move black ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y][s_x-i].hasWhiteBall == 1){
				w++;
				if(w > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y][s_x-i].hasWhiteBall != 1 && spaces[s_y][s_x-i].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y][s_x-i].hasBlackBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y][s_x-j].hasBlackBall == 1){
				b++;
				if(b == w)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y][s_x-j].hasWhiteBall != 1 && spaces[s_y][s_x-j].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y][s_x-j].hasWhiteBall)
				return -1;
		}
	}
}
int validateRight()		//check if right move is allowed
{
	//1 = valid, -1 = invalid
	b = 0;
	w = 0;
	if(spaces[s_y][s_x].hasWhiteBall != 1 && spaces[s_y][s_x].hasBlackBall != 1)
		return -1;	//can't move nothing
	if(turn == 1){
		if(spaces[s_y][s_x].hasWhiteBall == 1)
			return -1;	//black can't move white ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y][s_x+i].hasBlackBall == 1){
				b++;
				if(b > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y][s_x+i].hasWhiteBall != 1 && spaces[s_y][s_x+i].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y][s_x+i].hasWhiteBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y][s_x+j].hasWhiteBall == 1){
				w++;
				if(w == b)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y][s_x+j].hasWhiteBall != 1 && spaces[s_y][s_x+j].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y][s_x+j].hasBlackBall)
				return -1;
		}
	}
	else if(turn == -1){
		if(spaces[s_y][s_x].hasBlackBall == 1)
			return -1;	//white can't move black ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y][s_x+i].hasWhiteBall == 1){
				w++;
				if(w > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y][s_x+i].hasWhiteBall != 1 && spaces[s_y][s_x+i].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y][s_x+i].hasBlackBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y][s_x+j].hasBlackBall == 1){
				b++;
				if(b == w)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y][s_x+j].hasWhiteBall != 1 && spaces[s_y][s_x+j].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y][s_x+j].hasWhiteBall)
				return -1;
		}
	}
}
int validateUpLeft()		//check if left move is allowed
{
	//1 = valid, -1 = invalid
	b = 0;
	w = 0;
	if(spaces[s_y][s_x].hasWhiteBall != 1 && spaces[s_y][s_x].hasBlackBall != 1)
		return -1;	//can't move nothing
	if(turn == 1){
		if(spaces[s_y][s_x].hasWhiteBall == 1)
			return -1;	//black can't move white ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y-i][s_x-i].hasBlackBall == 1){
				b++;
				if(b > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y-i][s_x-i].hasWhiteBall != 1 && spaces[s_y-i][s_x-i].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y-i][s_x-i].hasWhiteBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y-j][s_x-j].hasWhiteBall == 1){
				w++;
				if(w == b)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y-j][s_x-j].hasWhiteBall != 1 && spaces[s_y-j][s_x-j].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y-j][s_x-j].hasBlackBall)
				return -1;
		}
	}
	else if(turn == -1){
		if(spaces[s_y][s_x].hasBlackBall == 1)
			return -1;	//white can't move black ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y-i][s_x-i].hasWhiteBall == 1){
				w++;
				if(w > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y-i][s_x-i].hasWhiteBall != 1 && spaces[s_y-i][s_x-i].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y-i][s_x-i].hasBlackBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y-j][s_x-j].hasBlackBall == 1){
				b++;
				if(b == w)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y-j][s_x-j].hasWhiteBall != 1 && spaces[s_y-j][s_x-j].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y-j][s_x-j].hasWhiteBall)
				return -1;
		}
	}
}
int validateDownLeft()		//check if left move is allowed
{
	//1 = valid, -1 = invalid
	b = 0;
	w = 0;
	if(spaces[s_y][s_x].hasWhiteBall != 1 && spaces[s_y][s_x].hasBlackBall != 1)
		return -1;	//can't move nothing
	if(turn == 1){
		if(spaces[s_y][s_x].hasWhiteBall == 1)
			return -1;	//black can't move white ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y+i][s_x-i].hasBlackBall == 1){
				b++;
				if(b > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y+i][s_x-i].hasWhiteBall != 1 && spaces[s_y+i][s_x-i].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y+i][s_x-i].hasWhiteBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y+j][s_x-j].hasWhiteBall == 1){
				w++;
				if(w == b)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y+j][s_x-j].hasWhiteBall != 1 && spaces[s_y+j][s_x-j].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y+j][s_x-j].hasBlackBall)
				return -1;
		}
	}
	else if(turn == -1){
		if(spaces[s_y][s_x].hasBlackBall == 1)
			return -1;	//white can't move black ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y+i][s_x-i].hasWhiteBall == 1){
				w++;
				if(w > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y+i][s_x-i].hasWhiteBall != 1 && spaces[s_y+i][s_x-i].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y+i][s_x-i].hasBlackBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y+j][s_x-j].hasBlackBall == 1){
				b++;
				if(b == w)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y+j][s_x-j].hasWhiteBall != 1 && spaces[s_y+j][s_x-j].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y+j][s_x-j].hasWhiteBall)
				return -1;
		}
	}
}
int validateUpRight()		//check if left move is allowed
{
	//1 = valid, -1 = invalid
	b = 0;
	w = 0;
	if(spaces[s_y][s_x].hasWhiteBall != 1 && spaces[s_y][s_x].hasBlackBall != 1)
		return -1;	//can't move nothing
	if(turn == 1){
		if(spaces[s_y][s_x].hasWhiteBall == 1)
			return -1;	//black can't move white ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y-i][s_x].hasBlackBall == 1){
				b++;
				if(b > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y-i][s_x].hasWhiteBall != 1 && spaces[s_y-i][s_x].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y-i][s_x].hasWhiteBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y-j][s_x].hasWhiteBall == 1){
				w++;
				if(w == b)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y-j][s_x].hasWhiteBall != 1 && spaces[s_y-j][s_x].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y-j][s_x].hasBlackBall)
				return -1;
		}
	}
	else if(turn == -1){
		if(spaces[s_y][s_x].hasBlackBall == 1)
			return -1;	//white can't move black ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y-i][s_x].hasWhiteBall == 1){
				w++;
				if(w > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y-i][s_x].hasWhiteBall != 1 && spaces[s_y-i][s_x].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y-i][s_x].hasBlackBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y-j][s_x].hasBlackBall == 1){
				b++;
				if(b == w)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y-j][s_x].hasWhiteBall != 1 && spaces[s_y-j][s_x].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y-j][s_x].hasWhiteBall)
				return -1;
		}
	}
}
int validateDownRight()		//check if left move is allowed
{
	//1 = valid, -1 = invalid
	b = 0;
	w = 0;
	if(spaces[s_y][s_x].hasWhiteBall != 1 && spaces[s_y][s_x].hasBlackBall != 1)
		return -1;	//can't move nothing
	if(turn == 1){
		if(spaces[s_y][s_x].hasWhiteBall == 1)
			return -1;	//black can't move white ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y+i][s_x].hasBlackBall == 1){
				b++;
				if(b > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y+i][s_x].hasWhiteBall != 1 && spaces[s_y+i][s_x].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y+i][s_x].hasWhiteBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y+j][s_x].hasWhiteBall == 1){
				w++;
				if(w == b)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y+j][s_x].hasWhiteBall != 1 && spaces[s_y+j][s_x].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y+j][s_x].hasBlackBall)
				return -1;
		}
	}
	else if(turn == -1){
		if(spaces[s_y][s_x].hasBlackBall == 1)
			return -1;	//white can't move black ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y+i][s_x].hasWhiteBall == 1){
				w++;
				if(w > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y+i][s_x].hasWhiteBall != 1 && spaces[s_y+i][s_x].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y+i][s_x].hasBlackBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y+j][s_x].hasBlackBall == 1){
				b++;
				if(b == w)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y+j][s_x].hasWhiteBall != 1 && spaces[s_y+j][s_x].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y+j][s_x].hasWhiteBall)
				return -1;
		}
	}
}
int validateDownRightTwo()
{
	//1 = valid, -1 = invalid
	b = 0;
	w = 0;
	if(spaces[s_y][s_x].hasWhiteBall != 1 && spaces[s_y][s_x].hasBlackBall != 1)
		return -1;	//can't move nothing
	if(turn == 1){
		if(spaces[s_y][s_x].hasWhiteBall == 1)
			return -1;	//black can't move white ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y+i][s_x+i].hasBlackBall == 1){
				b++;
				if(b > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y+i][s_x+i].hasWhiteBall != 1 && spaces[s_y+i][s_x+i].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y+i][s_x+i].hasWhiteBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y+j][s_x+j].hasWhiteBall == 1){
				w++;
				if(w == b)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y+j][s_x+j].hasWhiteBall != 1 && spaces[s_y+j][s_x+j].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y+j][s_x+j].hasBlackBall)
				return -1;
		}
	}
	else if(turn == -1){
		if(spaces[s_y][s_x].hasBlackBall == 1)
			return -1;	//white can't move black ball
		for(int i = 0; i < 11; i++){
			if(spaces[s_y+i][s_x+i].hasWhiteBall == 1){
				w++;
				if(w > 3)
					return -1;	//moved too many balls
			}
			else if(spaces[s_y+i][s_x+i].hasWhiteBall != 1 && spaces[s_y+i][s_x+i].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y+i][s_x+i].hasBlackBall == 1){
				temp = i;
				break;
			}
		}
		for(int j = temp; j < 11; j++){
			if(spaces[s_y+j][s_x+j].hasBlackBall == 1){
				b++;
				if(b == w)
					return -1;	//whites equal blacks
			}
			else if(spaces[s_y+j][s_x+j].hasWhiteBall != 1 && spaces[s_y+j][s_x+j].hasBlackBall != 1)
				return 1;
			else if(spaces[s_y+j][s_x+j].hasWhiteBall)
				return -1;
		}
	}
}

void moveLeft(){
	//shift pieces on the board, handle scoring. 
	for(int i = 0; i < 6; i++){
		piece_buffer[i].hasWhiteBall = 0;
		piece_buffer[i].hasBlackBall = 0;
	}
	temp = s_x;
	counter = 0;
	spaces[s_y][temp].selected = 0;
	for(int i = 0; i < 6; i++){
		if(spaces[s_y][temp].hasBlackBall == 1){
			piece_buffer[5-i].hasWhiteBall = 0;
			piece_buffer[5-i].hasBlackBall = 1;
			counter++;
			temp--;
		}
		else if(spaces[s_y][temp].hasWhiteBall == 1){
			piece_buffer[5-i].hasWhiteBall = 1;
			piece_buffer[5-i].hasBlackBall = 0;
			counter++;
			temp--;
		}
		else 
			break;
	}
	for(int i = (6-counter); i < 6; i++){
		if(spaces[s_y][temp].falloff == 1){
			temp++;
			if(piece_buffer[i].hasBlackBall == 1)
				white_score++;
			else if(piece_buffer[i].hasWhiteBall == 1)
				black_score++;
		}
		else{
			if(piece_buffer[i].hasBlackBall == 1){
				spaces[s_y][temp].hasWhiteBall = 0;
				spaces[s_y][temp].hasBlackBall = 1;
				temp++;
			}
			else if(piece_buffer[i].hasWhiteBall == 1){
				spaces[s_y][temp].hasWhiteBall = 1;
				spaces[s_y][temp].hasBlackBall = 0;
				temp++;
			}
		}
		spaces[s_y][temp].hasWhiteBall = 0;
		spaces[s_y][temp].hasBlackBall = 0;
	}
}
void moveRight(){
	//shift pieces on the board, handle scoring. 
	for(int i = 0; i < 6; i++){
		piece_buffer[i].hasWhiteBall = 0;
		piece_buffer[i].hasBlackBall = 0;
	}
	temp = s_x;
	counter = 0;
	spaces[s_y][temp].selected = 0;
	for(int i = 0; i < 6; i++){
		if(spaces[s_y][temp].hasBlackBall == 1){
			piece_buffer[5-i].hasWhiteBall = 0;
			piece_buffer[5-i].hasBlackBall = 1;
			counter++;
			temp++;
		}
		else if(spaces[s_y][temp].hasWhiteBall == 1){
			piece_buffer[5-i].hasWhiteBall = 1;
			piece_buffer[5-i].hasBlackBall = 0;
			counter++;
			temp++;
		}
		else 
			break;
	}
	for(int i = (6-counter); i < 6; i++){
		if(spaces[s_y][temp].falloff == 1){
			temp--;
			if(piece_buffer[i].hasBlackBall == 1)
				white_score++;
			else if(piece_buffer[i].hasWhiteBall == 1)
				black_score++;
		}
		else{
			if(piece_buffer[i].hasBlackBall == 1){
				spaces[s_y][temp].hasWhiteBall = 0;
				spaces[s_y][temp].hasBlackBall = 1;
				temp--;
			}
			else if(piece_buffer[i].hasWhiteBall == 1){
				spaces[s_y][temp].hasWhiteBall = 1;
				spaces[s_y][temp].hasBlackBall = 0;
				temp--;
			}
		}
		spaces[s_y][temp].hasWhiteBall = 0;
		spaces[s_y][temp].hasBlackBall = 0;
	}
}
void moveUpLeft(){
	//shift pieces on the board, handle scoring. 
	for(int i = 0; i < 6; i++){
		piece_buffer[i].hasWhiteBall = 0;
		piece_buffer[i].hasBlackBall = 0;
	}
	temp = s_y;
	temp2 = s_x;
	counter = 0; 
	spaces[temp][temp2].selected = 0;
	for(int i = 0; i < 6; i++){
		if(spaces[temp][temp2].hasBlackBall == 1){
			piece_buffer[5-i].hasWhiteBall = 0;
			piece_buffer[5-i].hasBlackBall = 1;
			counter++;
			temp--;
			temp2--;
		}
		else if(spaces[temp][temp2].hasWhiteBall == 1){
			piece_buffer[5-i].hasWhiteBall = 1;
			piece_buffer[5-i].hasBlackBall = 0;
			counter++;
			temp--;
			temp2--;
		}
		else 
			break;
	}
	for(int i = (6-counter); i < 6; i++){
		if(spaces[temp][temp2].falloff == 1){
			temp++;
			temp2++;
			if(piece_buffer[i].hasBlackBall == 1)
				white_score++;
			else if(piece_buffer[i].hasWhiteBall == 1)
				black_score++;
		}
		else{
			if(piece_buffer[i].hasBlackBall == 1){
				spaces[temp][temp2].hasWhiteBall = 0;
				spaces[temp][temp2].hasBlackBall = 1;
				temp++;
				temp2++;
			}
			else if(piece_buffer[i].hasWhiteBall == 1){
				spaces[temp][temp2].hasWhiteBall = 1;
				spaces[temp][temp2].hasBlackBall = 0;
				temp++;
				temp2++;
			}
		}
		spaces[temp][temp2].hasWhiteBall = 0;
		spaces[temp][temp2].hasBlackBall = 0;
	}
}
void moveDownLeft(){
	//shift pieces on the board, handle scoring.
	for(int i = 0; i < 6; i++){
		piece_buffer[i].hasWhiteBall = 0;
		piece_buffer[i].hasBlackBall = 0;
	}
	temp = s_y;
	temp2 = s_x;
	counter = 0; 
	spaces[temp][temp2].selected = 0; 
	for(int i = 0; i < 6; i++){
		if(spaces[temp][temp2].hasBlackBall == 1){
			piece_buffer[5-i].hasWhiteBall = 0;
			piece_buffer[5-i].hasBlackBall = 1;
			counter++;
			temp++;
			temp2--;
		}
		else if(spaces[temp][temp2].hasWhiteBall == 1){
			piece_buffer[5-i].hasWhiteBall = 1;
			piece_buffer[5-i].hasBlackBall = 0;
			counter++;
			temp++;
			temp2--;
		}
		else 
			break;
	}
	for(int i = (6-counter); i < 6; i++){
		if(spaces[temp][temp2].falloff == 1){
			temp--;
			temp2++;
			if(piece_buffer[i].hasBlackBall == 1)
				white_score++;
			else if(piece_buffer[i].hasWhiteBall == 1)
				black_score++;
		}
		else{
			if(piece_buffer[i].hasBlackBall == 1){
				spaces[temp][temp2].hasWhiteBall = 0;
				spaces[temp][temp2].hasBlackBall = 1;
				temp--;
				temp2++;
			}
			else if(piece_buffer[i].hasWhiteBall == 1){
				spaces[temp][temp2].hasWhiteBall = 1;
				spaces[temp][temp2].hasBlackBall = 0;
				temp--;
				temp2++;
			}
		}
		spaces[temp][temp2].hasWhiteBall = 0;
		spaces[temp][temp2].hasBlackBall = 0;
	}
}
void moveUpRight(){
	//shift pieces on the board, handle scoring. 
	for(int i = 0; i < 6; i++){
		piece_buffer[i].hasWhiteBall = 0;
		piece_buffer[i].hasBlackBall = 0;
	}
	temp = s_y;
	temp2 = s_x;
	counter = 0; 
	spaces[temp][temp2].selected = 0;
	for(int i = 0; i < 6; i++){
		if(spaces[temp][temp2].hasBlackBall == 1){
			piece_buffer[5-i].hasWhiteBall = 0;
			piece_buffer[5-i].hasBlackBall = 1;
			counter++;
			temp--;
		}
		else if(spaces[temp][temp2].hasWhiteBall == 1){
			piece_buffer[5-i].hasWhiteBall = 1;
			piece_buffer[5-i].hasBlackBall = 0;
			counter++;
			temp--;
		}
		else 
			break;
	}
	for(int i = (6-counter); i < 6; i++){
		if(spaces[temp][temp2].falloff == 1){
			temp++;
			if(piece_buffer[i].hasBlackBall == 1)
				white_score++;
			else if(piece_buffer[i].hasWhiteBall == 1)
				black_score++;
		}
		else{
			if(piece_buffer[i].hasBlackBall == 1){
				spaces[temp][temp2].hasWhiteBall = 0;
				spaces[temp][temp2].hasBlackBall = 1;
				temp++;
			}
			else if(piece_buffer[i].hasWhiteBall == 1){
				spaces[temp][temp2].hasWhiteBall = 1;
				spaces[temp][temp2].hasBlackBall = 0;
				temp++;
			}
		}
		spaces[temp][temp2].hasWhiteBall = 0;
		spaces[temp][temp2].hasBlackBall = 0;
	}
}
void moveDownRight(){
	//shift pieces on the board, handle scoring.
	for(int i = 0; i < 6; i++){
		piece_buffer[i].hasWhiteBall = 0;
		piece_buffer[i].hasBlackBall = 0;
	}
	temp = s_y;
	temp2 = s_x;
	counter = 0; 
	spaces[temp][temp2].selected = 0; 
	for(int i = 0; i < 6; i++){
		if(spaces[temp][temp2].hasBlackBall == 1){
			piece_buffer[5-i].hasWhiteBall = 0;
			piece_buffer[5-i].hasBlackBall = 1;
			counter++;
			temp++;
		}
		else if(spaces[temp][temp2].hasWhiteBall == 1){
			piece_buffer[5-i].hasWhiteBall = 1;
			piece_buffer[5-i].hasBlackBall = 0;
			counter++;
			temp++;
		}
		else 
			break;
	}
	for(int i = (6-counter); i < 6; i++){
		if(spaces[temp][temp2].falloff == 1){
			temp--;
			if(piece_buffer[i].hasBlackBall == 1)
				white_score++;
			else if(piece_buffer[i].hasWhiteBall == 1)
				black_score++;
		}
		else{
			if(piece_buffer[i].hasBlackBall == 1){
				spaces[temp][temp2].hasWhiteBall = 0;
				spaces[temp][temp2].hasBlackBall = 1;
				temp--;
			}
			else if(piece_buffer[i].hasWhiteBall == 1){
				spaces[temp][temp2].hasWhiteBall = 1;
				spaces[temp][temp2].hasBlackBall = 0;
				temp--;
			}
		}
		spaces[temp][temp2].hasWhiteBall = 0;
		spaces[temp][temp2].hasBlackBall = 0;
	}
}
void moveDownRightTwo(){
	//shift pieces on the board, handle scoring.
	for(int i = 0; i < 6; i++){
		piece_buffer[i].hasWhiteBall = 0;
		piece_buffer[i].hasBlackBall = 0;
	}
	temp = s_y;
	temp2 = s_x;
	counter = 0; 
	spaces[temp][temp2].selected = 0; 
	for(int i = 0; i < 6; i++){
		if(spaces[temp][temp2].hasBlackBall == 1){
			piece_buffer[5-i].hasWhiteBall = 0;
			piece_buffer[5-i].hasBlackBall = 1;
			counter++;
			temp++;
			temp2++;
		}
		else if(spaces[temp][temp2].hasWhiteBall == 1){
			piece_buffer[5-i].hasWhiteBall = 1;
			piece_buffer[5-i].hasBlackBall = 0;
			counter++;
			temp++;
			temp2++;
		}
		else 
			break;
	}
	for(int i = (6-counter); i < 6; i++){
		if(spaces[temp][temp2].falloff == 1){
			temp--;
			temp2--;
			if(piece_buffer[i].hasBlackBall == 1)
				white_score++;
			else if(piece_buffer[i].hasWhiteBall == 1)
				black_score++;
		}
		else{
			if(piece_buffer[i].hasBlackBall == 1){
				spaces[temp][temp2].hasWhiteBall = 0;
				spaces[temp][temp2].hasBlackBall = 1;
				temp--;
				temp2--;
			}
			else if(piece_buffer[i].hasWhiteBall == 1){
				spaces[temp][temp2].hasWhiteBall = 1;
				spaces[temp][temp2].hasBlackBall = 0;
				temp--;
				temp2--;
			}
		}
		spaces[temp][temp2].hasWhiteBall = 0;
		spaces[temp][temp2].hasBlackBall = 0;
	}
}

void my_keyboard(unsigned char c, int x, int y){
	if(c == 'q' || c == 'Q')
		exit(0);
	if(c == ',')
		i -= 1;
	if(c == '.')
		i += 1;
	if(c == 't' || c == 'T')
		showText *= -1;
	if(c == 'z' || c == 'Z')
		instruction *= -1;
	//if white's or black's score is 6, don't move anything. The game is over.
	if(white_score != 6 && black_score != 6){
		if(c == 'w' || c == 'W')	//move selection up
		{
			spaces[s_y][s_x].selected = 0.0;
			s_y -= turn;
			if(spaces[s_y][s_x].falloff == 1.0)
				s_y += turn;
			spaces[s_y][s_x].selected = 1.0;
		}
		if(c == 'a' || c == 'A')	//move selection left
		{
			spaces[s_y][s_x].selected = 0.0;
			s_x -= turn;
			if(spaces[s_y][s_x].falloff == 1.0)
				s_x += turn;
			spaces[s_y][s_x].selected = 1.0;
		}
		if(c == 's' || c == 'S')	//move selection down
		{
			spaces[s_y][s_x].selected = 0.0;
			s_y += turn;
			if(spaces[s_y][s_x].falloff == 1.0)
				s_y -= turn;
			spaces[s_y][s_x].selected = 1.0;
		}
		if(c == 'd' || c == 'D')	//move selection right
		{
			spaces[s_y][s_x].selected = 0.0;
			s_x += turn;
			if(spaces[s_y][s_x].falloff == 1.0)
				s_x -= turn;
			spaces[s_y][s_x].selected = 1.0;
		}
		
		//movement keys for balls
		if(c == 'h' || c == 'H')	//moving selected piece to the left
		{
			if(turn > 0){
				if(validateLeft() == 1){
					moveLeft();
					spaces[s_y][s_x].selected = 0;
					s_y = 1;
					s_x = 5;
					spaces[s_y][s_x].selected = 1;
					turn *= -1;
					rotating = 1;
					delay = 20;
				}
				else {
					invalid = 1;
					count = 0;
				}
			}
			else if(turn < 0){
				if(validateRight() == 1){
					moveRight();
					spaces[s_y][s_x].selected = 0;
					s_y = 9;
					s_x = 5;
					spaces[s_y][s_x].selected = 1;
					turn *= -1;
					rotating = 1;
					delay = 20;
				}
				else {
					invalid = 1;
					count = 0;
				}
			}
		}
		if(c == 'k' || c == 'K')	//moving selected piece to the right
		{
			if(turn > 0){
				if(validateRight() == 1){
					moveRight();
					spaces[s_y][s_x].selected = 0;
					s_y = 1;
					s_x = 5;
					spaces[s_y][s_x].selected = 1;
					turn *= -1;
					rotating = 1;
					delay = 20;
				}
				else {
					invalid = 1;
					count = 0;
				}
			}
			else if(turn < 0){
				if(validateLeft() == 1){
					moveLeft();
					spaces[s_y][s_x].selected = 0;
					s_y = 9;
					s_x = 5;
					spaces[s_y][s_x].selected = 1;
					turn *= -1;
					rotating = 1;
					delay = 20;
				}
				else {
					invalid = 1;
					count = 0;
				}
			}
		}
		if(c == 'u' || c == 'U')	//moving selected piece to the left and up
		{
			if(turn > 0){
				if(validateUpLeft() == 1){
					moveUpLeft();
					spaces[s_y][s_x].selected = 0;
					s_y = 1;
					s_x = 5;
					spaces[s_y][s_x].selected = 1;
					turn *= -1;
					rotating = 1;
					delay = 20;
				}
				else {
					invalid = 1;
					count = 0;
				}
			}
			
			else if(turn < 0){
				if(validateDownRightTwo() == 1){
					moveDownRightTwo();
					spaces[s_y][s_x].selected = 0;
					s_y = 9;
					s_x = 5;
					spaces[s_y][s_x].selected = 1;
					turn *= -1;
					rotating = 1;
					delay = 20;
				}
				else {
					invalid = 1;
					count = 0;
				}
			}
		}
		if(c == 'n' || c == 'N')	//moving selected piece to the left and down
		{
			if(turn > 0){
				if(validateDownRight() == 1){
					moveDownRight();
					spaces[s_y][s_x].selected = 0;
					s_y = 1;
					s_x = 5;
					spaces[s_y][s_x].selected = 1;
					turn *= -1;
					rotating = 1;
					delay = 20;
				}
				else {
					invalid = 1;
					count = 0;
				}
			}
			
			else if(turn < 0){
				if(validateUpRight() == 1){
					moveUpRight();
					spaces[s_y][s_x].selected = 0;
					s_y = 9;
					s_x = 5;
					spaces[s_y][s_x].selected = 1;
					turn *= -1;
					rotating = 1;
					delay = 20;
				}
				else {
					invalid = 1;
					count = 0;
				}
			}
		}
		if(c == 'm' || c == 'M')	//moving selected piece to the right and down
		{
			if(turn > 0){
				if(validateDownRightTwo() == 1){
					moveDownRightTwo();
					spaces[s_y][s_x].selected = 0;
					s_y = 1;
					s_x = 5;
					spaces[s_y][s_x].selected = 1;
					turn *= -1;
					rotating = 1;
					delay = 20;
				}
				else {
					invalid = 1;
					count = 0;
				}
			}
			
			else if(turn < 0){
				if(validateUpLeft() == 1){
					moveUpLeft();
					spaces[s_y][s_x].selected = 0;
					s_y = 9;
					s_x = 5;
					spaces[s_y][s_x].selected = 1;
					turn *= -1;
					rotating = 1;
					delay = 20;
				}
				else {
					invalid = 1;
					count = 0;
				}
			}
		}
		if(c == 'i' || c == 'I')	//moving selected piece to the right and up
		{
			if(turn > 0){
				if(validateUpRight() == 1){
					moveUpRight();
					spaces[s_y][s_x].selected = 0;
					s_y = 1;
					s_x = 5;
					spaces[s_y][s_x].selected = 1;
					turn *= -1;
					rotating = 1;
					delay = 20;
				}
				else {
					invalid = 1;
					count = 0;
				}
			}
			
			else if(turn < 0){
				if(validateDownRight() == 1){
					moveDownRight();
					spaces[s_y][s_x].selected = 0;
					s_y = 9;
					s_x = 5;
					spaces[s_y][s_x].selected = 1;
					turn *= -1;
					rotating = 1;
					delay = 20;
				}
				else {
					invalid = 1;
					count = 0;
				}
			}
		}
	}
	glutPostRedisplay();
}
void idle(){
	count++;
	delay--; 
	if(count % 2 == 0 && pan > 0 && delay < 0)
		pan -= 10;
	if(count % 30 == 0 && invalid > 0)
		invalid = -1;
	if(count % 30 == 0)
		swtch = swtch * -1;
	if(rotating == 1 && count % 3 == 0 && delay < 0){
		if(turn == 1 && rot < 90.0)
			rot += 10.0;
		else if(turn == -1 && rot > -90.0)
			rot -= 10.0;
		else
			rotating = 0;
	}
	glutPostRedisplay();
}
int main(int argc, char **argv){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(700,700);
	glutInitWindowPosition(0,0);
	glutCreateWindow("ABALONE");
	glutKeyboardFunc(my_keyboard);
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	init();
	glutMainLoop();
	
	return 0;
}
