#include<stdint.h>
#include"ST7735.h"
/**************ST7735_XYplotInit***************
 Specify the X and Y axes for an x-y scatter plot
 Draw the title and clear the plot area
 Inputs:  title  ASCII string to label the plot, null-termination
          minX   smallest X data value allowed, resolution= 0.001
          maxX   largest X data value allowed, resolution= 0.001
          minY   smallest Y data value allowed, resolution= 0.001
          maxY   largest Y data value allowed, resolution= 0.001
          bcolor 16-bit color of the background
 Outputs: none
 assumes minX < maxX, and minY < maxY
*/

int32_t width = ST7735_TFTWIDTH;   // this could probably be a constant, except it is used in Adafruit_GFX and depends on image rotation
int32_t height = ST7735_TFTHEIGHT;
int32_t min_x, max_x, min_y, max_y;

void ST7735_XYplotInit(char *title, int32_t minX, int32_t maxX, 
  int32_t minY, int32_t maxY, uint16_t bcolor) {  
		ST7735_FillScreen(ST7735_BLACK);
		ST7735_SetCursor(0, 1);
		ST7735_OutString(title);
		ST7735_FillRect(0, 32, 128, 128, bcolor);
		min_x = minX;
		min_y = minY;
		max_x = maxX;
		max_y = maxY;
	}

/**************ST7735_XYplot***************
 Plot an array of (x,y) data
 Inputs:  num    number of data points in the two arrays
          bufX   array of 32-bit fixed-point data, resolution= 0.001
          bufY   array of 32-bit fixed-point data, resolution= 0.001
          color  16-bit color of the data points
 Outputs: none
 assumes ST7735_XYplotInit has been previously called
 neglect any points outside the minX maxY minY maxY bounds
*/
void ST7735_XYplot(uint32_t num, int32_t bufX[], int32_t bufY[], uint16_t color) {
		int32_t plot_x, plot_y, cur_x, cur_y;
		for(int i = 0; i < num; i++) {
			cur_x = bufX[i];
			cur_y = bufY[i];
			plot_x = (127 * (cur_x - min_x))/(max_x - min_x);
			plot_y = 32 + (127 * (max_y - cur_y))/(max_y - min_y);
			ST7735_DrawPixel(plot_x, plot_y, color);
			ST7735_DrawPixel(plot_x+1, plot_y, color);
			ST7735_DrawPixel(plot_x, plot_y+1, color);
			ST7735_DrawPixel(plot_x+1, plot_y+1, color);
		}
		return;
	}

