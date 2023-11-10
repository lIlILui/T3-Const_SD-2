#include <hf-risc.h>
#include "vga_drv.h"

struct ball_s {
	unsigned int ballx, bally;
	unsigned int last_ballx, last_bally;
	int dx, dy;
};

/*int main(void)
{

    display_background(BLACK);

    while(1){
        //display_pixel(0,0,BLACK);
	    display_pixel(0,0,WHITE);
        display_pixel(0,217,BLUE);
        display_pixel(299,0,RED);
        display_pixel(2,0,LRED);
        display_pixel(3,0,LRED);
        display_pixel(4,0,LRED);
         display_pixel(5,0,LRED);
        display_frectangle(2,2,4,5,LRED);
        //display_pixel(0,2,WHITE);
        //display_pixel(2,2,WHITE);
    }
	return 0;
}*/

void init_display()
{
    int colors[8] = {RED,RED, LRED,LRED,GREEN,GREEN,LYELLOW,LYELLOW};
	display_background(BLACK);
    for(int i=0; i<8;++i){
        for(int j=0; j<30;j++){  
            display_frectangle(j*10, 30+(6*i), 8, 3, colors[i]) ; 
        } 
    }
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

void init_input()
{
	// configure GPIOB pins 8 .. 12 as inputs 
	GPIOB->DDR &= ~(MASK_P8 | MASK_P9 | MASK_P10 | MASK_P11 | MASK_P12);
}



void test_limits(char *limits, struct ball_s *ball)
{
	unsigned int ballx, bally;
	
	ballx = ball->ballx;
	bally = ball->bally;
	
	for(int j=0; j<8;j++){ 
		limits[j] = 0;
	}

	//display_pixel(ball->last_ballx, ball->last_bally, BLACK);
	for(int problem=2; problem < 3; problem++){ 
		limits[0] |= display_getpixel(ballx+problem, bally-problem);
		limits[1] |= display_getpixel(ballx, bally-problem);
		limits[2] |= display_getpixel(ballx-problem, bally+problem);
		limits[3] |= display_getpixel(ballx+problem, bally);
		limits[4] |= display_getpixel(ballx-problem, bally-problem);
		limits[5] |= display_getpixel(ballx, bally+problem);
		limits[6] |= display_getpixel(ballx+problem, bally+problem);
		limits[7] |= display_getpixel(ballx-problem, bally);
	}
	//display_pixel(ball->last_ballx, ball->last_bally, WHITE);
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
		else if (limits[2]) {
			ball->dx = -ball->dx;
			ball->dy = -ball->dy;
		}
		else if (limits[4]) {
			ball->dx = -ball->dx;
			ball->dy = -ball->dy;
		}
		else if (limits[6]) {
			ball->dx = -ball->dx;
			ball->dy = -ball->dy;
		}
		else {
			if (limits[1]) ball->dy = -ball->dy;
			if (limits[3]) ball->dx = -ball->dx;
			if (limits[5]) ball->dy = -ball->dy;
			if (limits[7]) ball->dx = -ball->dx;
		}
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

void get_input(int last_value[])
{

	if (GPIOB->IN & MASK_P8){
		display_frectangle((VGA_WIDTH/2)-15, (VGA_HEIGHT)-5, 20, 5, WHITE);
		last_value[0] = (VGA_WIDTH/2)-15;
	}
	if (GPIOB->IN & MASK_P11){ 
		display_frectangle(last_value[0], (VGA_HEIGHT)-5, 1, 5, BLACK);
		if(last_value[0]<278) last_value[0] = last_value[0]+1;
		display_frectangle(last_value[0], (VGA_HEIGHT)-5, 20, 5, WHITE);
	}
	if (GPIOB->IN & MASK_P10){ 
		display_frectangle(last_value[0]+20, (VGA_HEIGHT)-5, 1, 5, BLACK);
		if(last_value[0]>2) last_value[0] = last_value[0]-1;
		display_frectangle(last_value[0], (VGA_HEIGHT)-5, 20, 5, WHITE);
	}

	//else
		//display_frectangle(299, (VGA_HEIGHT/2)-10, 12, 4, BLACK);
}

int main(void)
{
	struct ball_s ball;
	struct ball_s *pball = &ball;
	char limits[9];
	int last_value[3];

	for(int i=0;i<3;++i){
		last_value[i] = 1;
	}
	
	init_display();
	init_ball(pball, 150, 105, 0, 1);
	init_input();

	while (1) {
		test_limits(limits, pball);
		test_collision(limits, pball);
		delay_ms(10);
		update_ball(pball);
		get_input(last_value);
	}

	return 0;
}
