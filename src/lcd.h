
#ifndef LCD_H
#define LCD_H


#define LCD_WIDTH   160
#define LCD_HEIGHT  144


#define LCD_COLOR_GREEN_LIGHTEST_R  224
#define LCD_COLOR_GREEN_LIGHTEST_G  248
#define LCD_COLOR_GREEN_LIGHTEST_B  208

#define LCD_COLOR_GREEN_LIGHTER_R   136
#define LCD_COLOR_GREEN_LIGHTER_G   192
#define LCD_COLOR_GREEN_LIGHTER_B   112

#define LCD_COLOR_GREEN_DARKER_R    52
#define LCD_COLOR_GREEN_DARKER_G    104
#define LCD_COLOR_GREEN_DARKER_B    86

#define LCD_COLOR_GREEN_DARKEST_R   8
#define LCD_COLOR_GREEN_DARKEST_G   24
#define LCD_COLOR_GREEN_DARKEST_B   32


#define LCD_COLOR_GRAY_LIGHTEST_R   232
#define LCD_COLOR_GRAY_LIGHTEST_G   232
#define LCD_COLOR_GRAY_LIGHTEST_B   232

#define LCD_COLOR_GRAY_LIGHTER_R    160
#define LCD_COLOR_GRAY_LIGHTER_G    160
#define LCD_COLOR_GRAY_LIGHTER_B    160

#define LCD_COLOR_GRAY_DARKER_R     88
#define LCD_COLOR_GRAY_DARKER_G     88
#define LCD_COLOR_GRAY_DARKER_B     88

#define LCD_COLOR_GRAY_DARKEST_R    16
#define LCD_COLOR_GRAY_DARKEST_G    16
#define LCD_COLOR_GRAY_DARKEST_B    16


#if LCD_GRAY
#define LCD_COLOR_LIGHTEST_R  LCD_COLOR_GRAY_LIGHTEST_R
#define LCD_COLOR_LIGHTEST_G  LCD_COLOR_GRAY_LIGHTEST_G
#define LCD_COLOR_LIGHTEST_B  LCD_COLOR_GRAY_LIGHTEST_B

#define LCD_COLOR_LIGHTER_R   LCD_COLOR_GRAY_LIGHTER_R
#define LCD_COLOR_LIGHTER_G   LCD_COLOR_GRAY_LIGHTER_G
#define LCD_COLOR_LIGHTER_B   LCD_COLOR_GRAY_LIGHTER_B

#define LCD_COLOR_DARKER_R    LCD_COLOR_GRAY_DARKER_R
#define LCD_COLOR_DARKER_G    LCD_COLOR_GRAY_DARKER_G
#define LCD_COLOR_DARKER_B    LCD_COLOR_GRAY_DARKER_B

#define LCD_COLOR_DARKEST_R   LCD_COLOR_GRAY_DARKEST_R
#define LCD_COLOR_DARKEST_G   LCD_COLOR_GRAY_DARKEST_G
#define LCD_COLOR_DARKEST_B   LCD_COLOR_GRAY_DARKEST_B

#else

#define LCD_COLOR_LIGHTEST_R  LCD_COLOR_GREEN_LIGHTEST_R
#define LCD_COLOR_LIGHTEST_G  LCD_COLOR_GREEN_LIGHTEST_G
#define LCD_COLOR_LIGHTEST_B  LCD_COLOR_GREEN_LIGHTEST_B

#define LCD_COLOR_LIGHTER_R   LCD_COLOR_GREEN_LIGHTER_R
#define LCD_COLOR_LIGHTER_G   LCD_COLOR_GREEN_LIGHTER_G
#define LCD_COLOR_LIGHTER_B   LCD_COLOR_GREEN_LIGHTER_B

#define LCD_COLOR_DARKER_R    LCD_COLOR_GREEN_DARKER_R
#define LCD_COLOR_DARKER_G    LCD_COLOR_GREEN_DARKER_G
#define LCD_COLOR_DARKER_B    LCD_COLOR_GREEN_DARKER_B

#define LCD_COLOR_DARKEST_R   LCD_COLOR_GREEN_DARKEST_R
#define LCD_COLOR_DARKEST_G   LCD_COLOR_GREEN_DARKEST_G
#define LCD_COLOR_DARKEST_B   LCD_COLOR_GREEN_DARKEST_B
#endif


#endif
