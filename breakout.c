#include <hf-risc.h>
#include "vga_drv.h"


#define brick_height 8
#define brick_widht 12

#define brick_columns 5

#define paddleHeight 5
#define paddleWidht 50  //size of paddle
#define paddleY 211		

int lives = 3;
int totalbricks = brick_columns * 23;


struct ball_s {
	unsigned int ballx, bally;
	unsigned int last_ballx, last_bally;
	int dx, dy;
};


struct paddle_p{
    unsigned int paddlex;
    int dx; 
};


struct brick_b{
	unsigned int brickx, bricky;
    uint16_t color;
};


void init_display()
{
	display_background(BLACK);
	display_frectangle(0, 6, VGA_WIDTH, 1, WHITE);
	update_livesdisplay();
}

void init_ball(struct ball_s *ball, int x, int y, int dx, int dy)
{
	ball->ballx = x;
	ball->bally = y;
	ball->last_ballx = ball->ballx;
	ball->last_bally = ball->bally;
	ball->dx = dx;
	ball->dy = dy;
}

void init_paddle(struct paddle_p *paddle, int x, int dx)
{
    paddle->paddlex = x;
	paddle->dx = dx;
}

void init_input()
{
	/* configure GPIOB pins 8 .. 12 as inputs */
	GPIOB->DDR &= ~(MASK_P8 | MASK_P9 | MASK_P10 | MASK_P11 | MASK_P12);
}



void test_limits(char *limits, struct ball_s *ball)
{
	unsigned int ballx, bally;
	
	ballx = ball->ballx;
	bally = ball->bally;
	
	limits[0] = display_getpixel(ballx-2, bally-2);
	limits[1] = display_getpixel(ballx, bally-1);
	limits[2] = display_getpixel(ballx+2, bally-2);
	limits[3] = display_getpixel(ballx+1, bally);
	limits[4] = display_getpixel(ballx+2, bally+2);
	limits[5] = display_getpixel(ballx, bally+1);
	limits[6] = display_getpixel(ballx-2, bally+2);
	limits[7] = display_getpixel(ballx-1, bally);
	limits[8] = display_getpixel(ballx, bally);
}

char test_collision(char *limits, struct ball_s *ball, struct brick_b *bricks)
{
	char hit = 0;
	int i;
	
	if ((ball->ballx < VGA_WIDTH-1) && (ball->ballx > 0) && (ball->bally < VGA_HEIGHT-1) && (ball->bally > 0)) {
		for (i = 0; i < 9; i++) {
			if (limits[i]) {
				hit = i;
				break;
			}
		}
		
		if (limits[0]) {
			ball->dx = -ball->dx;
			ball->dy = -ball->dy;
		}
		if (limits[2]) {
			ball->dx = -ball->dx;
			ball->dy = -ball->dy;
		}
		if (limits[4]) {
			ball->dx = -ball->dx;
			ball->dy = -ball->dy;
		}
		if (limits[6]) {
			ball->dx = -ball->dx;
			ball->dy = -ball->dy;
		}
		
		if (limits[1]) ball->dy = -ball->dy;
		if (limits[3]) ball->dx = -ball->dx;
		if (limits[5]) ball->dy = -ball->dy;
		if (limits[7]) ball->dx = -ball->dx;
	} else {
		if ((ball->ballx + ball->dx > VGA_WIDTH-1) || (ball->ballx + ball->dx < 1))
			ball->dx = -ball->dx;
		if ((ball->bally + ball->dy > VGA_HEIGHT-1) || (ball->bally + ball->dy < 1))
			ball->dy = -ball->dy;
	}

	if(limits[hit] != 7){
		test_blockHit(limits, ball, bricks, hit);
	}

	
	return hit;
}


/*
void test_limits(char *limits, struct ball_s *ball)
{
	unsigned int ballx, bally;
	
	ballx = ball->ballx;
	bally = ball->bally;
	
	//display_pixel(ball->last_ballx, ball->last_bally, BLACK);
	limits[0] = display_getpixel(ballx-1, bally-1);		// upper Pixels
	limits[1] = display_getpixel(ballx, bally-1);
	limits[2] = display_getpixel(ballx+1, bally-1);
	limits[3] = display_getpixel(ballx-1, bally);		// middle Pixels
	limits[4] = display_getpixel(ballx, bally);			// ball pixel
	limits[5] = display_getpixel(ballx+1, bally);
	limits[6] = display_getpixel(ballx-1, bally+1);		// lower Pixels
	limits[7] = display_getpixel(ballx, bally+1);
	limits[8] = display_getpixel(ballx+1, bally+1);
	//display_pixel(ball->last_ballx, ball->last_bally, WHITE);
}

char test_collision(char *limits, struct ball_s *ball, struct brick_b *bricks)
{
	char hit = 0;
	int i;
	
	if ((ball->ballx < VGA_WIDTH-1) && (ball->ballx > 0) && (ball->bally < VGA_HEIGHT-1) && (ball->bally > 0)) {
		for (i = 0; i < 9; i++) {
			if (limits[i]) {
				hit = i;
				break;
			}
		}
		
		if (limits[0]) {
			ball->dx = -ball->dx;
			ball->dy = -ball->dy;
		}

		if (limits[1]) ball->dy = -ball->dy;

		if (limits[2]) {
			ball->dx = -ball->dx;
			ball->dy = -ball->dy;
		}

		if (limits[3]) ball->dx = -ball->dx;

		//if (limits[4])  // pixel representing the ball does not collide with itself

		if (limits[5]) ball->dy = -ball->dx;
		
		if (limits[6]) {
			ball->dx = -ball->dx;
			ball->dy = -ball->dy;
		}
		
		if (limits[7]) ball->dx = -ball->dy;

		if (limits[8]) {
			ball->dx = -ball->dx;
			ball->dy = -ball->dy;
		}

	} else {
		if ((ball->ballx + ball->dx > VGA_WIDTH-1) || (ball->ballx + ball->dx < 1))
			ball->dx = -ball->dx;
		if (ball->bally + ball->dy < 1)
			ball->dy = -ball->dy;
	}

	if(limits[hit] != 7){
		test_blockHit(limits, ball, bricks);
	}
	
	return hit;
}
*/


void test_blockHit(char *limits, struct ball_s *ball, struct brick_b *bricks, char hit){
	if((brick_height + 1)*brick_columns + 9 >= ball->bally){
		int brick_y = brick_columns;
		while(1){
			int a = brick_y * 23 - 1;
			if(ball->bally > bricks[a].bricky){
				break;
			}
			else{
				brick_y -= 1;
			}
		}
		int brick_x = 0;
		while(1){
			int b = (brick_y * 23) - 24 + brick_x;
			if(bricks[b+1].brickx > ball->ballx >= bricks[b].brickx){
				break;
			}
			else{
				brick_x++;
			}
		}
		printf("brick_y %d\n", brick_y);
		printf("brick_x %d\n", brick_x);
		int id = brick_y * brick_x;
		printf("id %d\n", id);
		display_frectangle(bricks[id].brickx, bricks[id].bricky, brick_widht, brick_height, BLACK);

		/*int block_y = ball->bally / brick_height + 1; //coluna
		int block_x = ball->ballx / brick_widht + 1; //linha
		int id = block_y * block_x;
		printf("block_y %d\n", block_y);
		printf("block_x %d\n", block_x);
		printf("id %d\n", id);
		display_frectangle(bricks[id].brickx, bricks[id].bricky, brick_widht, brick_height, BLACK);
		 */
	}
}



void update_ball(struct ball_s *ball)
{
	ball->ballx = ball->ballx + ball->dx;
	ball->bally = ball->bally + ball->dy;

	display_pixel(ball->last_ballx, ball->last_bally, BLACK);
	display_pixel(ball->ballx, ball->bally, WHITE);
	ball->last_ballx = ball->ballx;
	ball->last_bally = ball->bally;

}


void update_paddle(struct paddle_p *paddle)
{
	display_frectangle(paddle->paddlex, paddleY, paddleWidht, paddleHeight, WHITE);

	if(paddle->dx == 0)
		return;

	paddle->paddlex = paddle->paddlex + paddle->dx;

	if(paddle->dx > 0){
		display_frectangle(paddle->paddlex - paddle->dx, paddleY, 1, paddleHeight, BLACK);
	}
	else{
		display_frectangle(paddle->paddlex + paddleWidht, paddleY, 1, paddleHeight, BLACK);
	} 
}


void get_input(struct paddle_p *paddle)
{
	paddle->dx = 0;
	if (!((GPIOB->IN & MASK_P10) && (GPIOB->IN & MASK_P11))){
		if (GPIOB->IN & MASK_P11 && paddle->paddlex < VGA_WIDTH-paddleWidht-1)
			paddle->dx = 1;
		if (GPIOB->IN & MASK_P10 && paddle->paddlex > 2)
			paddle->dx = -1;
	}
}



void test_Death(struct ball_s *ball){
	if(ball->bally + ball->dy > VGA_HEIGHT-1){
		lives--;
		ball->ballx = 150;
		ball->bally = 80;
		ball->dx = 0;
		ball->dy = 1;
		update_livesdisplay();
	}
}



void update_livesdisplay(void){
	display_frectangle(250, 1, 14, 4, BLACK);

	int x = 250;
	for(int i = 0; i < lives; i++){
		display_frectangle(x, 1, 2, 4, WHITE);
		x += 4;
	}
}



int main(void)
{
	struct ball_s ball;
	struct ball_s *pball = &ball;
	char limits[9];

	struct paddle_p Inpaddle;
	struct paddle_p *paddle = &Inpaddle;

	const int initialTotalBricks = totalbricks;
	struct brick_b bricks[initialTotalBricks];


	init_display();
	init_ball(pball, 150, 190, 0, 1);
	init_input();
	init_paddle(paddle, 125, 0);

	int startBrickY = 8;
	int brickIndex = 0;
	for(int i = 1; i <= brick_columns; i++){
		for(int j = 1; j <= VGA_WIDTH - brick_widht - 1; j += brick_widht + 1){
			bricks[brickIndex].brickx = j;
			bricks[brickIndex].bricky = startBrickY;
			bricks[brickIndex].color = i;
			display_frectangle(j, startBrickY, brick_widht, brick_height, i);
			brickIndex++;
		}
		startBrickY += brick_height + 1;
	}

	while (lives > 0 && totalbricks > 0) {
		test_Death(pball);
		test_limits(limits, pball);
		test_collision(limits, pball, bricks);
		delay_ms(10);
		update_ball(pball);
		get_input(paddle);
		update_paddle(paddle);
	}

	return 0;
}
