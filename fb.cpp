#include <stdlib.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <fcntl.h>
 #include <linux/fb.h>
 #include <sys/mman.h>
 #include <sys/ioctl.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

     struct fb_var_screeninfo vinfo;
     struct fb_fix_screeninfo finfo;
     int fbfd = 0;
     long int screensize = 0;
     char *fbp = 0;
     int xmax = 0;
     int ymax = 0;
  int color = 0x0A00FF00;
  int bgcolor = 0x0A000000;
  int colorblue = 0x0A0000FF;
  unsigned char character_A[] = {0b00011000, 0b00100100, 0b01111110, 0b10000001} ;

int drawpixel (int x, int y, int32_t color)
{
     if ((x < vinfo.xres) && (y < vinfo.yres))
     {
     long int location = 0;
             location = x * (vinfo.bits_per_pixel/8) + (vinfo.yres-y-1) * finfo.line_length;

             if (vinfo.bits_per_pixel == 32) {
//               *(fbp + location) = color & 0xFFFFFF;        
                 *(fbp + location) = (color) & 0xFF;     // blue
                 *(fbp + location + 1) = (color >> 8) & 0xFF;     // A little green
                 *(fbp + location + 2) = (color >> 16) & 0xFF;    // A lot of red
                 *(fbp + location + 3) = 0x10;      // No transparency
             } else  { //assume 16bpp
                 unsigned short int t = ((color >> 16) & 0xFF)<<11 | ((color >> 8) & 0xFF)<< 5 | (color & 0xFF);
                 *((unsigned short int*)(fbp + location)) = t;
             }
     }
     else 
     {
      printf ("Pixel outside of screen x=%d y=%d\n",x,y);
      exit(1);
     }
}

void draw_horizontal_line(int x1, int x2, int y, int color)
{
	int i;
	for (i=x1;i<x2;i++)
		drawpixel(i,y,color);
}
void draw_vertical_line(int x, int y1, int y2, int color)
{
	int i;
	for (i=y1;i<y2;i++)
		drawpixel(x,i,color);
}

void draw_line(int x1, int y1, int x2, int y2, int color, int linewidth = 1)
{
	int i,dx,dy,sdx,sdy,dxabs,dyabs,x,y,px,py;

	dx=x2-x1;			//Delta x
	dy=y2-y1;			//Delta y
	dxabs=abs(dx);		//Absolute delta
	dyabs=abs(dy);		//Absolute delta
	sdx=(dx>0)?1:-1; //signum function
	sdy=(dy>0)?1:-1; //signum function
	x=dyabs>>1;
	y=dxabs>>1;
	px=x1;
	py=y1;

	if (dxabs>=dyabs)
	{
		for(i=0;i<dxabs;i++)
		{
			y+=dyabs;
			if (y>=dxabs)
			{
				y-=dxabs;
				py+=sdy;
			}
			px+=sdx;
			drawpixel(px,py,color);
		}
	}
	else
	{
		for(i=0;i<dyabs;i++)
		{
			x+=dxabs;
			if (x>=dyabs)
			{
				x-=dyabs;
				px+=sdx;
			}
			py+=sdy;
			drawpixel(px,py,color);
		}
	}
}

void plot4points(double cx, double cy, double x, double y, int color)
	{
		drawpixel(cx + x, cy + y, color);
		drawpixel(cx - x, cy + y, color);
		drawpixel(cx + x, cy - y, color);
		drawpixel(cx - x, cy - y, color);
	}

void plot4pointssolid(double cx, double cy, double x, double y, int color)
	{
		draw_horizontal_line(cx + x, cx - x, cy + y,color);
		draw_horizontal_line(cx + x, cx - x, cy - y,color);
	}

void plot8points(double cx, double cy, double x, double y, int color)
	{
		plot4points(cx, cy, x, y,color);
		plot4points(cx, cy, y, x,color);
	}

void draw_circle(double cx, double cy, int radius, int linecolor = 0xFFFFFF, int linewidth = 1, int fillcolor = -1)
{
	for (int width = 0; width < linewidth; width++)
	{
        int error = -radius + width;
	double x = radius - width;
	double y = 0;
	while (x >= y)
	{
		plot8points(cx, cy, x, y, linecolor);

		error += y;
		y++;
		error += y;

		if (error >= 0)
		{
			error += -x;
			x--;
			error += -x;
		}
	}
	}
	
	if (fillcolor >= 0)
	{
        double x = radius;
	for (int width = linewidth; x > 0; width++)
	{
        int error = -radius + width;
        x = radius - width;
        double y = 0;
        while (x >= y)
        {
                plot8points(cx, cy, x, y, fillcolor);

                error += y;
                y++;
                error += y;

                if (error >= 0)
                {
                        error += -x;
                        x--;
                        error += -x;
                }
        }
	
	}
	}
}

int drawbmp (char *filename, int posx = 0, int posy = 0)
{
		int counter;
		FILE *ptr_myfile;

		ptr_myfile=fopen(filename,"rb");
		if (!ptr_myfile)
		{
			printf("Unable to open file!");
			return 1;
		}
		int32_t color=0;
		int x = posx;
		int y = posy;
		// Read header size
		fseek (ptr_myfile, 10, SEEK_SET);
		int headersize = 0;
		fread (&headersize,4,1,ptr_myfile);
		printf ("Headersize = %d\n",headersize);
	        
	        // Read width
	        int width = 0;
		fseek (ptr_myfile, 18, SEEK_SET);
		fread (&width,4,1,ptr_myfile);
		printf ("Width = %d\n",width);
		
	        // Read height
	        int height = 0;
		fseek (ptr_myfile, 22, SEEK_SET);
		fread (&height,4,1,ptr_myfile);
		printf ("Height = %d\n",height);
		
	        // Read bits per pixel
	        int bpp = 0;
		fseek (ptr_myfile, 28, SEEK_SET);
		fread (&bpp,2,1,ptr_myfile);
		printf ("Bits per pixel = %d\n",bpp);
		
		
	        int bmpy = 0;
		fseek (ptr_myfile, headersize+1, SEEK_SET);

	        int a = 0;
	        int r = 0;
	        int g = 0;
	        int b = 0;
	        int32_t readcolor = 0;
		        
		while (!feof(ptr_myfile))
		{
		        color = 0;
		        if (bpp == 32) 
		        {
		          fread(&readcolor,4,1,ptr_myfile);
			  // Convert BMP BRGAX to RGB
			  color = ((readcolor & 0xFF) << 8) | ((readcolor & 0xFF00) << 8 ) | ((readcolor & 0xFF0000) >> 16) | 0x0A000000;
                        }
			if (bpp == 24) 
			{
			  fread(&readcolor,3,1,ptr_myfile);
			  // Convert BMP BRG to RGB
			  color = ((readcolor & 0xFF) << 8) | ((readcolor & 0xFF00) << 8 ) | ((readcolor & 0xFF0000) >> 16) | 0x0A000000; 
                        }
			  
			
			if ((x < xmax) && (y < ymax)) drawpixel(x,y,color);
			x++;
			if (x > (width + posx) - 1)
			{
			  x=posx;
			  y++;
                        }
                        if (y >= height)  return 1;
		}
		fclose(ptr_myfile);
		return 0;
}

int drawcircle (int ox, int oy, int r, int linecolor = 0xFFFFFF, int linewidth = 1, int fillcolor = 0)
{
 for (int x = -r; x < r; x++)
 {
    int height = (int)sqrt(r * r - x * x);

    for (int y = -height; y < height; y++)
        if ((y < -height + linewidth) || (y > height - linewidth) || (x < -r+linewidth) || (x > r-linewidth)) drawpixel(x + ox, y + oy, color);
        else drawpixel (x+ox, y+oy, fillcolor);
 }
}

int drawtext (char *text, int x, int y, int color)
{
  for (int characterpointer = 0; characterpointer < strlen(text); characterpointer++)
  {
    switch (text[characterpointer])
    {
      case 'a':
       for (int i = 0; i < 8; i++)
       {
         for (int j = 0; j < 4; j++) if (character_A[j]&(1<<i)) drawpixel (x+i+(characterpointer * 10),y-j,color);
       }
      break;
      default:
      break;
    }
  } 
}

int drawrectangle(int xstart, int ystart, int width, int height, int bordercolor = 0xFFFFFF, int borderwidth = 1, int fillcolor = -1)
{
     // Figure out where in memory to put the pixel
     for (int y = ystart; y < ystart+height; y++)
         for (int x = xstart; x < xstart+width; x++) {
           // Draw rectangle borders
           if ((y < ystart + borderwidth) || (y > ystart + height - borderwidth -1) ||
               (x < xstart + borderwidth) || (x > xstart + width - borderwidth-1)) drawpixel (x,y,bordercolor);
           // Fill rectangle
           if ((fillcolor >= 0) && (y >= ystart + borderwidth) && (y <= ystart + height - borderwidth-1) &&
               (x >= xstart + borderwidth) && (x <= xstart + width - borderwidth-1)) drawpixel (x,y,fillcolor);
     }
}


int drawprocessbar (int xstart, int ystart, int width, int height, float percent, int bordercolor = 0xFFFFFF, int borderwidth = 1, int barcolor = 0xFFFFFF, int backgroundcolor = 0)
{
  drawrectangle(xstart,ystart,width,height,bordercolor,borderwidth);
  drawrectangle(xstart+borderwidth,ystart+borderwidth,((width-borderwidth-borderwidth) * percent)/100,height-(2*borderwidth),barcolor,0,barcolor);
  drawrectangle(xstart+borderwidth+((width * percent)/100),ystart+borderwidth,((width * (100-percent))/100)-borderwidth-borderwidth,height-borderwidth-borderwidth,backgroundcolor,0,backgroundcolor);
}

 int init()
 {
     int x = 0, y = 0;

     // Open the file for reading and writing
     fbfd = open("/dev/fb0", O_RDWR);
     if (fbfd == -1) {
         perror("Error: cannot open framebuffer device");
         exit(1);
     }
     printf("The framebuffer device was opened successfully.\n");

     // Get fixed screen information
     if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
         perror("Error reading fixed information");
         exit(2);
     }

     // Get variable screen information
     if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
         perror("Error reading variable information");
         exit(3);
     }

     printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

     // Figure out the size of the screen in bytes
     screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
     
     xmax = vinfo.xres;
     ymax = vinfo.yres;

     // Map the device to memory
     fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                        fbfd, 0);
     if ((long)fbp == -1) {
         perror("Error: failed to map framebuffer device to memory");
         exit(4);
     }
     printf("The framebuffer device was mapped to memory successfully.\n");
}

void clearscreen(int color = 0)
{
     drawrectangle (0,0,xmax,ymax,color,0,color);
}


void cleanup ()
{
     munmap(fbp, screensize);
     close(fbfd);
 }

int main ()
{
  init();
  // Clear Screen
  clearscreen(0);
  
  drawbmp ("background.bmp",0,0);
  
  drawrectangle (100,100,100,20,color);
  drawrectangle (100,140,100,20,color,0xFF0000,2);
  drawrectangle (100,180,100,20,color);
  drawrectangle (100,220,100,20,color);
  drawrectangle (100,260,100,20,color);
  drawrectangle (100,300,100,20,color);

  draw_line (100,100,600,600,0xFF0000);

  draw_circle(400,500,40,0xFF0000,10);
  
  char text[] = "allo";
  drawtext (text, 10,100,color);

  while(1)
  {
  for (int i = 0; i <= 1000; i++)
  {
     drawprocessbar (100,340,600,40,(float)i/10,0xBBBBBB,4,0x00FF00,0x777777);
     draw_circle(100,500,40,color,i/25,0xFF0000);
     usleep (1000);
  }
  for (int i = 1000; i >= 0; i--)
  {
     drawprocessbar (100,340,600,40,(float)i/10,0xBBBBBB,4,0x00FF00,0x777777);
     draw_circle(100,500,40,color,i/25,0xFF0000);
     usleep (1000);
  }
  }
  
  cleanup();

} 