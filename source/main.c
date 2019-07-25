#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.h"
#include "io.c"
#include "timer.h"
#include "scheduler.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void EEPROM_write(unsigned int address, unsigned char data)
{
	while(EECR & (1<<EEPE));
	EEAR = address;
	EEDR = data;
	EECR |= (1<<EEMPE);
	EECR |= (1<<EEPE);
}

unsigned char EEPROM_read(unsigned int address)
{
	while(EECR & (1<<EEPE));
	EEAR = address;
	EECR |= (1<<EERE);
	return EEDR;
}

unsigned char character[8] = {0x0E, 0x0E, 0x0E, 0x15, 0x0E, 0x04, 0x0A, 0x11};
unsigned char enermy[8] = {0x07, 0x07, 0x01, 0x1D, 0x07, 0x01, 0x07, 0x05};
unsigned char bullet[8] = {0x00, 0x00, 0x0F, 0x1F, 0x1F, 0x0F, 0x00, 0x00};
unsigned char tomb[8] = {0x00, 0x00, 0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
unsigned char skeleton[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x04, 0x0A};
unsigned char destination[8] = {0x0F, 0x0F, 0x0F, 0x08, 0x08, 0x08, 0x08, 0x1F};

unsigned char upper_area[49] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

unsigned char lower_area[49] =
{0, 0, 0, 0, 3, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 1,
0, 0, 3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 1, 0, 0,
0, 0, 3, 3, 3, 0, 0, 0, 0, 0, 0, 1, 0, 3, 0, 4};

unsigned char condition = 0;
unsigned char position = 0;
unsigned char upper_scroll = 0;
unsigned char lower_scroll = 0;
unsigned char terminal = 0;
unsigned char victory = 0;
unsigned char failure = 0;
unsigned char bullet_path1[6] = {10, 11, 12, 13, 14, 15};
unsigned char bullet_path2[6] = {24, 25, 26, 27, 28, 29};
unsigned char bullet_path3[6] = {38, 39, 40, 41, 42, 43};

enum Title{Set, Trigger};
enum Move{Still, Left, Right};
enum Jump{Init, Up, Wait};
enum Scroll_Map{Stay, Scroll};
enum Evaluate{Judge};
enum Enermy_Shooting{Bullet1, Bullet2, Bullet3, Bullet4, Bullet5, Bullet6};
enum Score_Result{Score, Best};

int Title_tick(int Title_state)
{
	switch(Title_state)
	{
		case Set:
			if(~PINA & 0x01 == 0x01)
			{
				if(condition == 1)
				{		
					condition = 0;
				}
				else if(condition == 0)
				{
					if(victory == 0 && failure == 0)
					{
						condition = 1;
					}
					else if(victory == 1 || failure == 1)
					{
						condition = 0;
						position = 17;
						upper_scroll = 0;
						lower_scroll = 0;
						terminal = 0;
						victory = 0;
						failure = 0;
						lower_area[16] = 1;
						lower_area[30] = 1;
						lower_area[44] = 1;
						PORTB = 0x00;
					}
				}
				Title_state = Trigger;
			}
			else
			{
				Title_state = Set;
			}
			break;
		case Trigger:
			Title_state = (~PINA & 0x01 == 0x01) ? Trigger : Set;
			break;
		default:
			condition = 0;
			Title_state = Set;
			break;
	}
	switch(Title_state) {
		case Set:
			break;
		case Trigger:
			break;
		default:
			break;
	}
	return Title_state;
}

int Moving_tick(int Move_state)
{
	unsigned char L = ~PINA & 0x08;
	unsigned char R = ~PINA & 0x02;
	switch (Move_state)
	{
		case Still:
			if (condition && L && !R)
			{
				Move_state = Left;
			}
			else if (condition && !L && R)
			{
				Move_state = Right;
			}
			else
			{
				Move_state = Still;
			}
			break;
		case Left:
			if (condition && L && !R)
			{
				Move_state = Left;
			}
			else
			{
				Move_state = Still;
			}
			break;
		case Right:
			if (condition && !L && R)
			{
				Move_state = Right;
			}
			else
			{
				Move_state = Still;
			}
			break;
		default:
			position = 17;
			Move_state = Still;
		break;
	}
	switch(Move_state)
	{
		case Still:
			break;
		case Left:
			if (position != 1 && position != 17)
			{
				if (position < 17)
				{
					if (upper_area[upper_scroll + position - 1] == 3 ||
						upper_area[upper_scroll + position - 1] == 1)
					{
						break;
					}
					else
					{
						position--;
					}
				}
				else
				{
					if (lower_area[lower_scroll + position - 17] == 3 ||
						lower_area[lower_scroll + position - 17] == 1)
					{
						break;
					}
					else
					{
						position--;
					}
				}
			}
			break;
		case Right:
			if (!terminal)
			{
				if (position < 26)
				{
					if (position < 17)
					{
						if (upper_area[upper_scroll + position + 1] == 3 ||
							upper_area[upper_scroll + position + 1] == 1)
						{
							break;
						}
						else
						{
							position++;
						}
					}
					else 
					{
						if (lower_area[lower_scroll + position - 15] == 3 ||
							lower_area[lower_scroll + position - 15] == 1)
						{
							break;
						}
						else
						{
							position++;
						}
					}
				}
			}
			else
			{
				if (position < 32)
				{
					if (position < 17)
					{
						if (upper_area[upper_scroll + position + 1] == 3 ||
							upper_area[upper_scroll + position + 1] == 1)
						{
							break;
						}
						else
						{
							position++;
						}
					}
					else
					{
						if (lower_area[lower_scroll + position - 15] == 3 ||
							lower_area[lower_scroll + position - 15] == 1)
						{
							break;
						}
						else
						{
							position++;
						}
					}
				}
			}
			break;
		default:
			break;
	}	
	if (condition)
	{
		LCD_ClearScreen();
		LCD_WriteData(' ');
		LCD_Cursor(position);
		LCD_WriteData(0);
		LCD_Cursor(0);
	}
	return Move_state;
}

int Jumping_tick(int Jump_state)
{
	static unsigned char i = 0;	
	unsigned char J = ~PINA & 0x04;
	switch(Jump_state)
	{
		case Init:
			if (condition && J)
			{
				if (position > 16)
				{
					if(upper_area[upper_scroll + position - 16] == 3)
					{
						Jump_state = Init;
					}
					else
					{
						position -= 16;
						Jump_state = Up;
					}
				}
				else
				{
					Jump_state = Init;
				}
			}
			else
			{
				Jump_state = Init;
			}
			break;
		case Up:
			if (i < 2)
			{
				Jump_state = Up;
			}
			else
			{
				if (position < 16)
				{
					if (lower_area[lower_scroll + position] == 3)
					{
						Jump_state = Wait;
					}
					else
					{
						position += 16;
						if (lower_area[lower_scroll + position - 16] == 1)
						{
							lower_area[lower_scroll + position - 16] = 5;
							if(lower_scroll + position - 16 == 16)
							{
								PORTB = 0x04;
							}
							if(lower_scroll + position - 16 == 30)
							{
								if(lower_area[16] == 5)
								{
									PORTB = 0x06;
								}
								else
								{
									PORTB = 0x02;
								}
							}
							if(lower_scroll + position - 16 == 44)
							{
								if(lower_area[16] == 5)
								{
									if(lower_area[30] == 5)
									{
										PORTB = 0x07;
									}
									else
									{
										PORTB = 0x05;
									}
								}
								else if(lower_area[30] == 5)
								{
									if(lower_area[16] == 5)
									{
										PORTB = 0x07;
									}
									else
									{
										PORTB = 0x03;
									}
								}
								else
								{
									PORTB = 0x01;
								}
							}
						}
						Jump_state = Init;
					}
				}
				else
				{
					Jump_state = Wait;
				}
			}
			break;
		case Wait:
			if (position < 16)
			{
				if (lower_area[lower_scroll + position] == 3)
				{
					Jump_state = Wait;
				}
				else
				{
					position += 16;
					if (lower_area[lower_scroll + position - 16] == 1)
					{
						lower_area[lower_scroll + position - 16] = 5;
						if(lower_scroll + position - 16 == 16)
						{
							PORTB = 0x04;
						}
						if(lower_scroll + position - 16 == 30)
						{
							if(lower_area[16] == 5)
							{
								PORTB = 0x06;
							}
							else
							{
								PORTB = 0x02;
							}
						}
						if(lower_scroll + position - 16 == 44)
						{
							if(lower_area[16] == 5)
							{
								if(lower_area[30] == 5)
								{
									PORTB = 0x07;
								}
								else
								{
									PORTB = 0x05;
								}
							}
							else if(lower_area[30] == 5)
							{
								if(lower_area[16] == 5)
								{
									PORTB = 0x07;
								}
								else
								{
									PORTB = 0x03;
								}
							}
							else
							{
								PORTB = 0x01;
							}
						}
					}
					Jump_state = Init;
				}
			}
			else
			{
				Jump_state = Wait;
			}
			break;
		default:
			Jump_state = Init;
			break;
	}
	switch(Jump_state)
	{
		case Init:
			i = 0;
			break;
		case Up:
			i = i < 2 ? i + 1 : 0;
			break;
		case Wait:
			break;
		default:
			break;
	}
	if (position < 1)
	{
		position = 1;
	}
	if (!terminal)
	{
		if (position > 10 && position < 17)
		{
			position = 10;
		}
		if (position > 26 && position < 32)
		{
			position = 26;
		}
	}
	if (condition)
	{
		LCD_ClearScreen();
		LCD_WriteData(' ');
		LCD_Cursor(position);
		LCD_WriteData(0);
		LCD_Cursor(0);
	}
	return Jump_state;
}

int Scrolling_tick(int Map_Scrolling_state)
{	
	switch(Map_Scrolling_state)
	{
		case Stay:
			if ((position == 10 || position == 26) && (~PINA & 0x02))
			{
				Map_Scrolling_state = Scroll;
			}
			else
			{
				Map_Scrolling_state = Stay;
			}
			break;
		case Scroll:
			if ((position == 10 || position == 26) && (~PINA & 0x02))
			{
				Map_Scrolling_state = Scroll;
			}
			else 
			{
				Map_Scrolling_state = Stay;
			}
			break;
		default:
			upper_scroll = 0;
			lower_scroll = 0;
			Map_Scrolling_state = Stay;
			break;
	}
	switch(Map_Scrolling_state)
	{
		case Stay:
			break;
		case Scroll:
			if ((position == 10 && upper_area[upper_scroll + 11] == 1) ||
				(position == 10 && upper_area[upper_scroll + 11] == 3) ||
				(position == 26 && lower_area[lower_scroll + 11] == 1) ||
				(position == 26 && lower_area[lower_scroll + 11] == 3))
			{
				break;
			}
			else
			{
				upper_scroll += upper_scroll < 32 ? 1 : 0;
				lower_scroll += lower_scroll < 32 ? 1 : 0;
			}
			if (upper_scroll == 32)
			{
				terminal = 1;
			}
			break;
		default:
			break;
	}
	return Map_Scrolling_state;
}

int Evaluation_tick(int Evaluation_state)
{
	switch(Evaluation_state)
	{
		case Judge:
			Evaluation_state = Judge;
			break;
		default:
			Evaluation_state = Judge;
			break;
	}
	switch(Evaluation_state)
	{
		case Judge:
			if (position == 32 && lower_area[lower_scroll + position - 16] == 4)
			{
				victory = 1;
			}
			else if ((position < 17 && upper_area[upper_scroll + position] == 2) ||
					 (position >= 17 && lower_area[lower_scroll + position - 16] == 2))
			{
				failure = 1;
			}
			break;
		default:
			break;
	}
	if (failure) {
		condition = 0;
		LCD_ClearScreen();
		LCD_DisplayString(1, "   You  Lose!      Game Over!");
		LCD_Cursor(0);
	}
	return Evaluation_state;
}

int Shooting_tick(int Shooting_state)
{
	unsigned char j;
	switch(Shooting_state)
	{
		case Bullet1:
			Shooting_state = Bullet2;
			break;
		case Bullet2:
			Shooting_state = Bullet3;
			break;
		case Bullet3:
			Shooting_state = Bullet4;
			break;
		case Bullet4:
			Shooting_state = Bullet5;
			break;
		case Bullet5:
			Shooting_state = Bullet6;
			break;
		case Bullet6:
			Shooting_state = Bullet1;
			break;
		default:
			Shooting_state = Bullet1;
			break;
	}
	switch(Shooting_state)
	{
		case Bullet1:
			if(lower_area[16] == 5)
			{
				lower_area[10] = 0;
				lower_area[11] = 0;
				lower_area[12] = 0;
				lower_area[13] = 0;
				lower_area[14] = 0;
				lower_area[15] = 0;
			}
			else
			{
				lower_area[10] = 0;
				lower_area[11] = 0;
				lower_area[12] = 0;
				lower_area[13] = 0;
				lower_area[14] = 0;
				lower_area[15] = 2;
			}
			if(lower_area[30] == 5)
			{
				lower_area[24] = 0;
				lower_area[25] = 0;
				lower_area[26] = 0;
				lower_area[27] = 0;
				lower_area[28] = 0;
				lower_area[29] = 0;
			}
			else
			{
				lower_area[24] = 0;
				lower_area[25] = 0;
				lower_area[26] = 0;
				lower_area[27] = 0;
				lower_area[28] = 2;
				lower_area[29] = 2;
			}
			if(lower_area[44] == 5)
			{
				lower_area[38] = 0;
				lower_area[39] = 0;
				lower_area[40] = 0;
				lower_area[41] = 0;
				lower_area[42] = 0;
				lower_area[43] = 0;
			}
			else
			{
				lower_area[38] = 0;
				lower_area[39] = 0;
				lower_area[40] = 0;
				lower_area[41] = 2;
				lower_area[42] = 0;
				lower_area[43] = 2;
			}
			break;
		case Bullet2:
			if(lower_area[16] == 5)
			{
				lower_area[10] = 0;
				lower_area[11] = 0;
				lower_area[12] = 0;
				lower_area[13] = 0;
				lower_area[14] = 0;
				lower_area[15] = 0;
			}
			else
			{
				lower_area[10] = 0;
				lower_area[11] = 0;
				lower_area[12] = 0;
				lower_area[13] = 0;
				lower_area[14] = 2;
				lower_area[15] = 0;
			}
			if(lower_area[30] == 5)
			{
				lower_area[24] = 0;
				lower_area[25] = 0;
				lower_area[26] = 0;
				lower_area[27] = 0;
				lower_area[28] = 0;
				lower_area[29] = 0;
			}
			else
			{
				lower_area[24] = 0;
				lower_area[25] = 0;
				lower_area[26] = 0;
				lower_area[27] = 2;
				lower_area[28] = 2;
				lower_area[29] = 0;
			}
			if(lower_area[44] == 5)
			{
				lower_area[38] = 0;
				lower_area[39] = 0;
				lower_area[40] = 0;
				lower_area[41] = 0;
				lower_area[42] = 0;
				lower_area[43] = 0;
			}
			else
			{
				lower_area[38] = 0;
				lower_area[39] = 0;
				lower_area[40] = 2;
				lower_area[41] = 0;
				lower_area[42] = 2;
				lower_area[43] = 0;
			}
			break;
		case Bullet3:
			if(lower_area[16] == 5)
			{
				lower_area[10] = 0;
				lower_area[11] = 0;
				lower_area[12] = 0;
				lower_area[13] = 0;
				lower_area[14] = 0;
				lower_area[15] = 0;
			}
			else
			{
				lower_area[10] = 0;
				lower_area[11] = 0;
				lower_area[12] = 0;
				lower_area[13] = 2;
				lower_area[14] = 0;
				lower_area[15] = 0;
			}
			if(lower_area[30] == 5)
			{
				lower_area[24] = 0;
				lower_area[25] = 0;
				lower_area[26] = 0;
				lower_area[27] = 0;
				lower_area[28] = 0;
				lower_area[29] = 0;
			}
			else
			{
				lower_area[24] = 0;
				lower_area[25] = 0;
				lower_area[26] = 2;
				lower_area[27] = 2;
				lower_area[28] = 0;
				lower_area[29] = 0;
			}
			if(lower_area[44] == 5)
			{
				lower_area[38] = 0;
				lower_area[39] = 0;
				lower_area[40] = 0;
				lower_area[41] = 0;
				lower_area[42] = 0;
				lower_area[43] = 0;
			}
			else
			{
				lower_area[38] = 0;
				lower_area[39] = 2;
				lower_area[40] = 0;
				lower_area[41] = 2;
				lower_area[42] = 0;
				lower_area[43] = 0;
			}
			break;
		case Bullet4:
			if(lower_area[16] == 5)
			{
				lower_area[10] = 0;
				lower_area[11] = 0;
				lower_area[12] = 0;
				lower_area[13] = 0;
				lower_area[14] = 0;
				lower_area[15] = 0;
			}
			else
			{
				lower_area[10] = 0;
				lower_area[11] = 0;
				lower_area[12] = 2;
				lower_area[13] = 0;
				lower_area[14] = 0;
				lower_area[15] = 0;
			}
			if(lower_area[30] == 5)
			{
				lower_area[24] = 0;
				lower_area[25] = 0;
				lower_area[26] = 0;
				lower_area[27] = 0;
				lower_area[28] = 0;
				lower_area[29] = 0;
			}
			else
			{
				lower_area[24] = 0;
				lower_area[25] = 2;
				lower_area[26] = 2;
				lower_area[27] = 0;
				lower_area[28] = 0;
				lower_area[29] = 0;
			}
			if(lower_area[44] == 5)
			{
				lower_area[38] = 0;
				lower_area[39] = 0;
				lower_area[40] = 0;
				lower_area[41] = 0;
				lower_area[42] = 0;
				lower_area[43] = 0;
			}
			else
			{
				lower_area[38] = 2;
				lower_area[39] = 0;
				lower_area[40] = 2;
				lower_area[41] = 0;
				lower_area[42] = 0;
				lower_area[43] = 0;
			}
			break;
		case Bullet5:
			if(lower_area[16] == 5)
			{
				lower_area[10] = 0;
				lower_area[11] = 0;
				lower_area[12] = 0;
				lower_area[13] = 0;
				lower_area[14] = 0;
				lower_area[15] = 0;
			}
			else
			{
				lower_area[10] = 0;
				lower_area[11] = 2;
				lower_area[12] = 0;
				lower_area[13] = 0;
				lower_area[14] = 0;
				lower_area[15] = 0;
			}
			if(lower_area[30] == 5)
			{
				lower_area[24] = 0;
				lower_area[25] = 0;
				lower_area[26] = 0;
				lower_area[27] = 0;
				lower_area[28] = 0;
				lower_area[29] = 0;
			}
			else
			{
				lower_area[24] = 2;
				lower_area[25] = 2;
				lower_area[26] = 0;
				lower_area[27] = 0;
				lower_area[28] = 0;
				lower_area[29] = 0;
			}
			if(lower_area[44] == 5)
			{
				lower_area[38] = 0;
				lower_area[39] = 0;
				lower_area[40] = 0;
				lower_area[41] = 0;
				lower_area[42] = 0;
				lower_area[43] = 0;
			}
			else
			{
				lower_area[38] = 0;
				lower_area[39] = 2;
				lower_area[40] = 0;
				lower_area[41] = 0;
				lower_area[42] = 0;
				lower_area[43] = 2;
			}
			break;
		case Bullet6:
			if(lower_area[16] == 5)
			{
				lower_area[10] = 0;
				lower_area[11] = 0;
				lower_area[12] = 0;
				lower_area[13] = 0;
				lower_area[14] = 0;
				lower_area[15] = 0;
			}
			else
			{
				lower_area[10] = 2;
				lower_area[11] = 0;
				lower_area[12] = 0;
				lower_area[13] = 0;
				lower_area[14] = 0;
				lower_area[15] = 0;
			}
			if(lower_area[30] == 5)
			{
				lower_area[24] = 0;
				lower_area[25] = 0;
				lower_area[26] = 0;
				lower_area[27] = 0;
				lower_area[28] = 0;
				lower_area[29] = 0;
			}
			else
			{
				lower_area[24] = 2;
				lower_area[25] = 0;
				lower_area[26] = 0;
				lower_area[27] = 0;
				lower_area[28] = 0;
				lower_area[29] = 2;
			}
			if(lower_area[44] == 5)
			{
				lower_area[38] = 0;
				lower_area[39] = 0;
				lower_area[40] = 0;
				lower_area[41] = 0;
				lower_area[42] = 0;
				lower_area[43] = 0;
			}
			else
			{
				lower_area[38] = 2;
				lower_area[39] = 0;
				lower_area[40] = 0;
				lower_area[41] = 0;
				lower_area[42] = 2;
				lower_area[43] = 0;
			}
			break;
		default:
			break;
	}
	if (condition == 0)
	{
		if (victory == 0 && failure == 0)
		{
			LCD_DisplayString(1, "Scroller: Mario");
			LCD_Cursor(17);
			LCD_WriteData(0);
			LCD_Cursor(32);
			LCD_WriteData(4);
			LCD_Cursor(0);
		}
		return Shooting_state;
	}
	for (j = 1; j <= 16; j++)
	{
		if (upper_area[upper_scroll + j] != 0)
		{
			LCD_Cursor(j);
			LCD_WriteData(upper_area[upper_scroll + j]);
		}
		if (lower_area[lower_scroll + j] != 0)
		{
			LCD_Cursor(j + 16);
			LCD_WriteData(lower_area[lower_scroll + j]);
		}
	}
	LCD_Cursor(0);
	return Shooting_state;
}

int Result_tick(int Score_Result_state)
{
	unsigned char score;
	unsigned char best_score;
	unsigned char pause = ~PINA & 0x01;
	switch(Score_Result_state)
	{
		case Score:
			Score_Result_state = Best;
			break;
		case Best:
			Score_Result_state = Score;
			break;
		default:
			score = 0;
			best_score = 0;
			Score_Result_state = Score;
			break;
	}
	switch(Score_Result_state)
	{
		case Score:
			if (victory)
			{
				condition = 0;
				if(lower_area[16] == 5)
				{
					score++;
				}
				if(lower_area[30] == 5)
				{
					score++;
				}
				if(lower_area[44] == 5)
				{
					score++;
				}
				if (score > EEPROM_read(0))
				{
					EEPROM_write(0, score);
				}
				LCD_DisplayString(1, " Win! Score:  ");
				LCD_WriteData(score + '0');
				LCD_Cursor(0);
			}
			break;
		case Best:
			if (victory && !condition)
			{
				best_score = EEPROM_read(0) % 10;
				LCD_DisplayString(18, "Best Score:  ");
				LCD_WriteData(best_score + '0');
				LCD_Cursor(0);
			}
			break;
		default:
			break;
	}
	return Score_Result_state;
}

int main(void)
{
	DDRA = 0x00;
	DDRB = 0xFF;
	DDRC = 0xFF;
	DDRD = 0xFF;
	PORTA = 0xFF;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x00;
	
	unsigned long int Title_Period = 50;
	unsigned long int Moving_Period = 100;
	unsigned long int Jumping_Period = 100;
	unsigned long int Mapping_Period = 100;
	unsigned long int Evaluation_Period = 100;
	unsigned long int Shooting_Period = 100;
	unsigned long int Result_Period = 100;

	unsigned long int tmp = 1;
	tmp = findGCD(Title_Period, Moving_Period);
	tmp = findGCD(tmp, Jumping_Period);
	tmp = findGCD(tmp, Mapping_Period);
	tmp = findGCD(tmp, Evaluation_Period);
	tmp = findGCD(tmp, Shooting_Period);
	tmp = findGCD(tmp, Result_Period);
	
	unsigned long int GCD = tmp;

	unsigned long int Title_Scheduler_Period = Title_Period/GCD;
	unsigned long int Moving_Scheduler_Period = Moving_Period/GCD;
	unsigned long int Jumping_Scheduler_Period = Jumping_Period/GCD;
	unsigned long int Mapping_Scheduler_Period = Mapping_Period/GCD;
	unsigned long int Evaluation_Scheduler_Period = Evaluation_Period/GCD;
	unsigned long int Shooting_Scheduler_Period = Shooting_Period/GCD;
	unsigned long int Result_Scheduler_Period = Result_Period/GCD;
	
	static task task1;
	static task task2;
	static task task3;
	static task task4;
	static task task5;
	static task task6;
	static task task7;
	task *tasks[] = {&task1, &task2, &task3, &task4, &task5, &task6, &task7};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	task1.state = -1;
	task1.period = Title_Scheduler_Period;
	task1.elapsedTime = Title_Scheduler_Period;
	task1.TickFct = &Title_tick;
	
	task2.state = -1;
	task2.period = Moving_Scheduler_Period;
	task2.elapsedTime = Moving_Scheduler_Period;
	task2.TickFct = &Moving_tick;
	
	task3.state = -1;
	task3.period = Jumping_Scheduler_Period;
	task3.elapsedTime = Jumping_Scheduler_Period;
	task3.TickFct = &Jumping_tick;

	task4.state = -1;
	task4.period = Mapping_Scheduler_Period;
	task4.elapsedTime = Mapping_Scheduler_Period;
	task4.TickFct = &Scrolling_tick;
	
	task5.state = -1;
	task5.period = Evaluation_Scheduler_Period;
	task5.elapsedTime = Evaluation_Scheduler_Period;
	task5.TickFct = &Evaluation_tick;
	
	task6.state = -1;
	task6.period = Shooting_Scheduler_Period;
	task6.elapsedTime = Shooting_Scheduler_Period;
	task6.TickFct = &Shooting_tick;
	
	task7.state = -1;
	task7.period = Result_Scheduler_Period;
	task7.elapsedTime = Result_Scheduler_Period;
	task7.TickFct = &Result_tick;

	TimerSet(GCD);
	TimerOn();
	LCD_init();
	
	if (EEPROM_read(0) == 255) 
	{ 
		EEPROM_write(0, 0); 
	}
	
	LCD_CustomChar(0, character);
	LCD_CustomChar(1, enermy);
	LCD_CustomChar(2, bullet);
	LCD_CustomChar(3, tomb);
	LCD_CustomChar(4, destination);
	LCD_CustomChar(5, skeleton);
	
	unsigned short i;
	while(1)
	{
		for ( i = 0; i < numTasks; i++ )
		{
			if ( tasks[i]->elapsedTime == tasks[i]->period )
			{
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}
