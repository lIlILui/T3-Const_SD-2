#include <hf-risc.h>
#include "vga_drv.h"


#define brick_height 8
#define brick_widht 12

#define brick_columns 5
#define brick_per_columns 23

#define paddleHeight 5
#define paddleWidht 50  //size of paddle
#define paddleY 211		

int lives = 3;
int totalbricks = brick_columns * brick_per_columns;


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

char init_ball(struct ball_s *ball, int x, int y, int dx, int dy)
{

	static char init_game = 0;

	if (GPIOB->IN & MASK_P8){
		ball->ballx = x;
		ball->bally = y;
		ball->last_ballx = ball->ballx;
		ball->last_bally = ball->bally;
		ball->dx = dx;
		ball->dy = dy;
		init_game = 1;
	}

	return init_game;

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
	
	limits[0] = display_getpixel(ballx-1, bally-1);
	limits[1] = display_getpixel(ballx, bally-1);
	limits[2] = display_getpixel(ballx+1, bally-1);
	limits[3] = display_getpixel(ballx+1, bally);
	limits[4] = display_getpixel(ballx+1, bally+1);
	limits[5] = display_getpixel(ballx, bally+1);
	limits[6] = display_getpixel(ballx-1, bally+1);
	limits[7] = display_getpixel(ballx-1, bally);
	limits[8] = display_getpixel(ballx, bally);
}

char test_collision(char *limits, struct ball_s *ball, struct brick_b *bricks, struct paddle_p *paddle)
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
	else{
		if(ball->bally >= paddleY-1){
			if (ball->ballx <= paddle->paddlex + paddleWidht/2){
				ball->dx = -1;
			}
			else{
				if (paddle->paddlex + paddleWidht/2 < ball->ballx){
				ball->dx = 1;
				}
			}
		}
	}
	
	return hit;
}


void test_blockHit(char *limits, struct ball_s *ball, struct brick_b *bricks, char hit){
	if((brick_height + 1)*brick_columns + 9 >= ball->bally){
		int paddle_x =0, paddle_y = 8;
		for(int j = 1; j <= VGA_WIDTH - brick_widht - 1; j += brick_widht + 1){
			if((ball->ballx >= j-2) && (ball->ballx < (j+brick_widht)))  {
				paddle_x = j;
				printf("paddle_x: %d\n", paddle_x);
				break;		
			}
		}
		for(int i = 1; i <= brick_columns; i++){
			if((ball->bally >= paddle_y-1) && (ball->bally <= (paddle_y+8))){
				display_frectangle(paddle_x, paddle_y, brick_widht, brick_height, BLACK);
				totalbricks -= 1;
				printf("paddle_y = %d\n",paddle_y);
				return;
			}
			paddle_y += brick_height + 1;
		}
	} 	
}

/*void test_blockHit(char *limits, struct ball_s *ball, struct brick_b *bricks, char hit){
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
		while(brick_x < 22){
			if(bricks[brick_x+1].brickx > ball->ballx >= bricks[brick_x].brickx){
				break;
			}
			else{
				brick_x++;
			}
		}

		printf("brick_y %d\n", brick_y);
		printf("brick_x %d\n", brick_x);

		int id;
		if (brick_y > 0)
			id = brick_x + (23 * (brick_y-1));
		else 
			id = brick_x;

		printf("id %d\n", id);
		display_frectangle(bricks[id].brickx, bricks[id].bricky, brick_widht, brick_height, BLACK);
		totalbricks -= 1;
	}
}*/



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


/*void get_input(struct paddle_p *paddle)
{
	paddle->dx = 0;
	if (!((GPIOB->IN & MASK_P10) && (GPIOB->IN & MASK_P11))){
		if (GPIOB->IN & MASK_P11 && paddle->paddlex < VGA_WIDTH-paddleWidht-1)
			paddle->dx = 1;
		if (GPIOB->IN & MASK_P10 && paddle->paddlex > 2)
			paddle->dx = -1;
	}
}*/

void get_input(struct paddle_p * paddle)
{
	int values_read[2];
	read_mouse(values_read);
	static int changes1 = 0, changes2 , flag = 0;
 
	if (GPIOB->IN & MASK_P8 && flag != 1){
		display_frectangle((VGA_WIDTH/2)-15, (VGA_HEIGHT)-5, paddleWidht, 5, WHITE);
		paddle->paddlex = (VGA_WIDTH/2)-15;
		flag = 1;
	}
	//if (GPIOB->IN & MASK_P10){ 
	if((values_read[0] == 0) && (flag == 1) && (paddle->paddlex > 1)){
		if(abs(values_read[1]) > 10000) changes1 = 3; 
		else if(abs(values_read[1]) > 1000)  changes1 = 2; 
		else if(abs(values_read[1]) > 100)   changes1 = 1;
		else changes1 = 0;
		display_frectangle(paddle->paddlex+paddleWidht-changes1, (VGA_HEIGHT)-5, changes1, 5, BLACK);
		display_frectangle(paddle->paddlex-changes1, (VGA_HEIGHT)-5, changes1, 5, WHITE);
		paddle->paddlex = paddle->paddlex - changes1;
		//printf("changes1 penis = %d\n", changes1);
	}
	//if (GPIOB->IN & MASK_P11){ 
	if((values_read[0] == 1) && (flag == 1) && (paddle->paddlex < VGA_WIDTH-paddleWidht-1)){
		if(abs(values_read[1]) > 10000) changes2 = 3; 
		else if(abs(values_read[1]) > 1000)  changes2 = 2; 
		else if(abs(values_read[1]) > 100)   changes2 = 1;
		else changes2 = 0;
		display_frectangle(paddle->paddlex, (VGA_HEIGHT)-5, changes2, 5, BLACK);
		display_frectangle(paddle->paddlex+paddleWidht, (VGA_HEIGHT)-5, changes2, 5, WHITE);
		paddle->paddlex = paddle->paddlex + changes2;
		//printf("changes2 penis = %d\n", changes2);
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
	pball->dx = 0;
	pball->dy = 0;
	char limits[9],cont=0, init_game = 0;

	struct paddle_p Inpaddle;
	struct paddle_p *paddle = &Inpaddle;

	const int initialTotalBricks = totalbricks;
	struct brick_b bricks[initialTotalBricks];

	while (1){
		init_display();
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
			if(cont>1 && (init_game==1)){
				test_Death(pball);
				test_limits(limits, pball);
				test_collision(limits, pball, bricks, paddle);
				update_ball(pball);
				//update_paddle(paddle);
				cont=0;
			}
			else cont++;
			init_game = init_ball(pball, 0, VGA_HEIGHT/2, 1, 1);
			get_input(paddle);
			delay_ms(5);
		}


		while (1){
			if (GPIOB->IN & MASK_P9){
				lives = 3;
				totalbricks = brick_columns * brick_per_columns;
				break;
			}
			if (GPIOB->IN & MASK_P12){
				return 1;
			}
		}
		
	}

	return 0;
}
