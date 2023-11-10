#include <hf-risc.h>
#include "vga_drv.h"


#define brick_height 15
#define brick_widht 30

#define brick_columns 5

#define paddleHeight 10
#define paddleWidht 50  //size of paddle
#define paddleY 20		

int lives = 3;
int totalbricks = 50;


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
	//display_frectangle(100, 100, 25, 35, GREEN);
	//display_fcircle(250, 50, 25, YELLOW);
	//display_ftriangle(30, 170, 75, 195, 50, 150, RED);
	//display_frectangle(230, 180, 25, 10, CYAN);
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
	
	display_pixel(ball->last_ballx, ball->last_bally, BLACK);
	limits[0] = display_getpixel(ballx-2, bally-2);
	limits[1] = display_getpixel(ballx, bally-1);
	limits[2] = display_getpixel(ballx+2, bally-2);
	limits[3] = display_getpixel(ballx+1, bally);
	limits[4] = display_getpixel(ballx+2, bally+2);
	limits[5] = display_getpixel(ballx, bally+1);
	limits[6] = display_getpixel(ballx-2, bally+2);
	limits[7] = display_getpixel(ballx-1, bally);
	limits[8] = display_getpixel(ballx, bally);
	display_pixel(ball->last_ballx, ball->last_bally, WHITE);
}

char test_collision(char *limits, struct ball_s *ball)
{
	char hit = 0;
	int i;
	
	if ((ball->ballx < VGA_WIDTH-1) && (ball->ballx > 0) && (ball->bally < VGA_HEIGHT-1) && (ball->bally > 0)) {
		for (i = 0; i < 9; i++) {
			if (limits[i]) {
				hit = 1;
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
	
	return hit;
}


void update_ball(struct ball_s *ball)
{
	display_pixel(ball->last_ballx, ball->last_bally, BLACK);
	display_pixel(ball->ballx, ball->bally, WHITE);

	ball->last_ballx = ball->ballx;
	ball->last_bally = ball->bally;
	ball->ballx = ball->ballx + ball->dx;
	ball->bally = ball->bally + ball->dy;
}


void update_paddle(struct paddle_p *paddle)
{
	if(paddle->dx == 0)
		return;

	paddle->paddlex = paddle->paddlex + paddle->dx;

	if(paddle->dx > 0)
	display_frectangle(paddle->paddlex - paddle->dx, paddleY, paddle->dx, paddleHeight, BLACK);
	else
	display_frectangle(paddle->paddlex + paddleWidht, paddleY, paddle->dx, paddleHeight, BLACK);

    display_frectangle(paddle->paddlex, paddleY, paddleWidht, paddleHeight, WHITE);
}


void get_input(struct paddle_p *paddle)
{
	paddle->dx = 0;
	if (!(GPIOB->IN & MASK_P10 && GPIOB->IN & MASK_P11)){
		if (GPIOB->IN & MASK_P11)
			paddle->dx = 2;
		if (GPIOB->IN & MASK_P10)
			paddle->dx = -2;
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
	init_ball(pball, 150, 80, 0, -1);
	init_input();
	init_paddle(paddle, 125, 0);

	int startBrickY = 0;
	int brickIndex = 0;
	for(int i = 1; i < brick_columns + 1; i++){
		for(int j = 0; j <= VGA_WIDTH - brick_widht; j += brick_widht){
			bricks[brickIndex].brickx = j;
			bricks[brickIndex].bricky = startBrickY;
			bricks[brickIndex].color = i;
			display_frectangle(j, startBrickY, brick_widht, brick_height, i);
			brickIndex++;
		}
		startBrickY += brick_height;
	}

	while (lives > 0 && totalbricks > 0) {
		test_limits(limits, pball);
		test_collision(limits, pball);
		delay_ms(10);
		update_ball(pball);
		get_input(paddle);
		update_paddle(paddle);
	}

	return 0;
}
