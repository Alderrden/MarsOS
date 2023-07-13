#include "pong.h"
#include "string.h"
#include "vesa.h"
#include "bitmap.h"
#include "timer.h"
#include "keyboard.h"

// pads position y, will change on keys
uint16 pad_pos_y = 2;
// score counnt
int score_count = 0;
int vbe_h = vbe_get_height();
int vbe_w = vbe_get_width();
uint32 digit_count(int num)
{
  uint32 count = 0;
  if(num == 0)
    return 1;
  while(num > 0){
    count++;
    num = num/10;
  }
  return count;
}


#define PAD_POS_X    vbe_w - PAD_WIDTH - 1
// initialize game with into
static void init_game()
{
    uint8 b = 0;

    bitmap_draw_string("PONG GAME", 10, 70, VBE_RGB(255,0,0));
    draw_rect(100, 4, 120, 25, VBE_RGB(0,0,255));
    bitmap_draw_string("HOW TO PLAY", 10, 70, VBE_RGB(255,0,0));
    draw_rect(2, 40, 235, 80, VBE_RGB(165,42,42));
    bitmap_draw_string("ARROW KEY UP", 10, 70, VBE_RGB(255,0,0));
    bitmap_draw_string("TO MOVE BOTH PADS UP", 10, 70, VBE_RGB(255,0,0));
    bitmap_draw_string("ARROW KEY DOWN", 10, 70, VBE_RGB(255,0,0));
    bitmap_draw_string("TO MOVE BOTH PADS DOWN", 10, 70, VBE_RGB(255,0,0));
    bitmap_draw_string("PRESS ENTER TO START", 10, 70, VBE_RGB(255,0,0));
#ifdef VIRTUALBOX
    sleep(10);
#endif
    while (1)
    {
        b = kb_get_scancode();
        sleep(5);
        if (b == SCAN_CODE_KEY_ENTER)
            break;
        b = 0;
    }
    vesa_clear(VBE_RGB(0, 0, 0));
}

// update score count text
static void update_score_count()
{
    char str[32];
    int d = digit_count(score_count);
    itoa(str, score_count, d);
    bitmap_draw_string(str, 150, 2, VBE_RGB(255,255,240));
}

// if lose then display final score & restart game
static void lose()
{
    uint8 b = 0;
    char str[32];
    int d = digit_count(score_count);
    itoa(str, score_count, d);
    vesa_clear(VBE_RGB(0, 0, 0));
    bitmap_draw_string("NICE PLAY", 120, 15, VBE_RGB(34,139,34));
    bitmap_draw_string("SCORE", 125, 45, VBE_RGB(255,255,240));
    bitmap_draw_string(str, 180, 45, VBE_RGB(255,255,240));
    bitmap_draw_string("PRESS ENTER TO PLAY AGAIN", 45, 130, VBE_RGB(255,255,0));
    bitmap_draw_string("PRESS ESC TO BACK", 55, 130, VBE_RGB(255,255,0));

#ifdef VIRTUALBOX
    sleep(10);
#endif
    while (1)
    {
        b = kb_get_scancode();
        sleep(5);
        if (b == SCAN_CODE_KEY_ENTER)
            break;
        b = 0;
    }
    score_count = 0;
    vesa_clear(VBE_RGB(0, 0, 0));
    pong_game();
}

// move both pads simultaneously on pressed keys
void move_pads()
{
    uint8 b;

    // draw both pads
    fill_rect(0, pad_pos_y, PAD_WIDTH, PAD_HEIGHT, VBE_RGB(255,255,240));
    fill_rect(PAD_POS_X, pad_pos_y, PAD_WIDTH, PAD_HEIGHT, VBE_RGB(255,255,240));

    b = kb_get_scancode();
    // if down key pressed, move both pads down
    if (b == SCAN_CODE_KEY_DOWN)
    {
        if (pad_pos_y < vbe_h - PAD_HEIGHT)
            pad_pos_y = pad_pos_y + PAD_SPEED;
        fill_rect(0, pad_pos_y, PAD_WIDTH, PAD_HEIGHT, VBE_RGB(255,255,240));
        fill_rect(PAD_POS_X, pad_pos_y, PAD_WIDTH, PAD_HEIGHT, VBE_RGB(255,255,240));
    }
    // if up key pressed, move both pads up
    else if (b == SCAN_CODE_KEY_UP)
    {
        if (pad_pos_y >= PAD_WIDTH)
            pad_pos_y = pad_pos_y - PAD_SPEED;
        fill_rect(0, pad_pos_y, PAD_WIDTH, PAD_HEIGHT, VBE_RGB(255,255,240));
        fill_rect(PAD_POS_X, pad_pos_y, PAD_WIDTH, PAD_HEIGHT, VBE_RGB(255,255,240));
    }
#ifdef VIRTUALBOX
    sleep(1);
#endif
}


void pong_game()
{
    uint16 rect_pos_x = RECT_SIZE + 20;
    uint16 rect_pos_y = RECT_SIZE;
    uint16 rect_speed_x = RECT_SPEED_X;
    uint16 rect_speed_y = RECT_SPEED_Y;

    init_game();

    while (1)
    {
        // add speed values to positions
        rect_pos_x += rect_speed_x;
        rect_pos_y += rect_speed_y;

        // check if position x < left pad position x
        if (rect_pos_x - RECT_SIZE <= PAD_WIDTH + 1)
        {
            // if position of rect is not between left pad position,
            // then lose, bounced rect is not in y range of pad
            if ((rect_pos_y > 0 && rect_pos_y < pad_pos_y) || 
                (rect_pos_y <= vbe_h && rect_pos_y > pad_pos_y + PAD_HEIGHT))
            {
                lose();
            }
            else
            {
                // set speed x to negative, means move opposite direction
                rect_speed_x = -rect_speed_x;
                // set position x to rect size
                rect_pos_x = PAD_WIDTH + RECT_SIZE;
                // increase score
                score_count++;
            }
        }
        // check if position x >= right pad position x
        else if (rect_pos_x + RECT_SIZE >= PAD_POS_X + RECT_SIZE - 1)
        {
            // in range of y pad position
            if ((rect_pos_y > 0 && rect_pos_y < pad_pos_y) || 
                (rect_pos_y <= vbe_h && rect_pos_y > pad_pos_y + PAD_HEIGHT) || 
                (rect_pos_y + RECT_SIZE > 0 && rect_pos_y + RECT_SIZE < pad_pos_y))
            {
                lose();
            }
            else
            {
                // set speed x to negative, means move opposite direction
                rect_speed_x = -rect_speed_x;
                // set position x to minimum of pad position x - rect size
                rect_pos_x = PAD_POS_X - RECT_SIZE;
                // increase score
                score_count++;
            }
        }

        // change rect y position by checking boundries
        if (rect_pos_y - RECT_SIZE <= 0)
        {
            rect_speed_y = -rect_speed_y;
            rect_pos_y = RECT_SIZE;
        }
        else if(rect_pos_y + RECT_SIZE > vbe_h + RECT_SIZE)
        {
            rect_speed_y = -rect_speed_y;
            rect_pos_y = vbe_h - RECT_SIZE;
        }

        // clear screen for repaint
        vesa_clear(VBE_RGB(0, 0, 0));
        // move pads on keys
        move_pads();
        // update score count
        update_score_count();
        // fill bounced rect
        fill_rect(rect_pos_x - RECT_SIZE, rect_pos_y - RECT_SIZE, RECT_SIZE, RECT_SIZE, VBE_RGB(255,255,240));
        // change sleep value if running in VirtualBox or on bare metal
        sleep(1);
    }
}
