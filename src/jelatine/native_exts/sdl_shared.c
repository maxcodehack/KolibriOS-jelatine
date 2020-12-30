
#include <SDL.h>

static int sdl_is_init=0;
const int default_screen_width=240;//640,
const int default_screen_height=320;//480;
const int default_deph=32;//4;//16; 
SDL_Surface *screen_surface=NULL;//main screen
SDL_Surface *nxj_lcd_screen=NULL;//buffer in 4 bit format
void nxj_draw_char_32bit(unsigned char c, int x,int y);

void nxj_draw_char_4bit(unsigned char c, int x,int y);
 void (*nxj_draw_char)(unsigned char c, int x,int y)=NULL;
 
enum{ 	
	ROP_AND =	-16777216,
	ROP_ANDINVERTED= 	-65536,
	ROP_ANDREVERSE= 	-16711936,
	ROP_CLEAR= 	0,
	ROP_COPY= 	65280,
	ROP_COPYINVERTED= 	65535,
	ROP_EQUIV= 	16777215,
	ROP_INVERT= 	16711935,
	ROP_NAND= 	-16776961,
	ROP_NOOP= 	16711680,
	ROP_NOR= 	-1,
	ROP_OR= 	-256,
	ROP_ORINVERTED= 	-16711681,
	ROP_ORREVERSE= 	-65281,
	ROP_SET= 	255,
	ROP_XOR= 	16776960,
};

int sdlg_is_init(){
	//return (SDL_INIT_VIDEO==SDL_WasInit(SDL_INIT_VIDEO))
	return sdl_is_init;
}

int sdlg_init(int w,int h,int depth)
{
	if (SDL_Init(SDL_INIT_VIDEO)!=0) {
		fprintf(stderr,"\nError!Failed SDL_Init(SDL_VIDEO)!\nErrorMSG=%s",SDL_GetError());
		return 1;
	}
	SDL_EventState(	SDL_MOUSEMOTION, SDL_IGNORE);
    //SDL_EventState(SDL_KEYUP, SDL_IGNORE);

	atexit(SDL_Quit);
	screen_surface=SDL_SetVideoMode(w,h,depth,SDL_DOUBLEBUF);
	if (screen_surface==NULL){
		fprintf(stderr,
			"\nError!Failed SDL_SetVideoMode(w=%i,h=%i,depth=%i,SDL_DOUBLEBUF)!"
			"\nErrorMSG=%s",w,h,depth,SDL_GetError());
		return 2;
	}
	//
	
	sdl_is_init=1;
	return 0;
}

int sdlg_init_default(){
	return sdlg_init(default_screen_width,default_screen_height,default_deph);
}
int sdlg_fill_masks(int deph_,Uint32* rmask_,
	Uint32* gmask_,Uint32* bmask_,Uint32* amask_)
{
	Uint32 rmask, gmask, bmask, amask;
	switch(deph_){
		case 4:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0x01000000;
    gmask = 0x00010000;
    bmask = 0x00000100;
    amask = 0x00000001;
#else
    rmask = 0x00000001;
    gmask = 0x00000100;
    bmask = 0x00010000;
    amask = 0x01000000;
#endif
		break;
case 32:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
break;
		case 16:
		break;
		case 8:
		break;
	default:
		fprintf(stderr,"\nError!Deph:%i",deph_);
		break;
	};
	*rmask_=rmask;
	*gmask_=gmask;
	*bmask_=bmask;
	*amask_=amask;
	return 0;
}

int sdlg_init_nxj_screen(){
	int ret,i;
	int deph;
	Uint32 rmask, gmask, bmask, amask;

	if (sdl_is_init==0){
		ret=sdlg_init_default();
		if (ret != 0){
			fprintf(stderr,"\nError in sdlg_init_nxj_screen!\nSDL init failed!");
			return 1;
		} 
	}
	deph=32;
	sdlg_fill_masks(deph,&rmask, &gmask, &bmask, &amask);
#if 1
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    switch(deph){
		case 32:
			nxj_draw_char=nxj_draw_char_32bit;
			break;
		case 4:
			nxj_draw_char=nxj_draw_char_4bit;
			break;
	};
	nxj_lcd_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 
		default_screen_width,default_screen_height,deph,
		//width, height, 32,
                                   rmask, gmask, bmask, amask);
    if(nxj_lcd_screen == NULL) {
        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
        return 2;
    }
	
	if (SDL_LockSurface(nxj_lcd_screen)==0){
		/*
		for (i=0;i<240*320/4;i++){
			((unsigned char *)nxj_lcd_screen->pixels)[i]=(i & 0xff);
		};
		*/
		
	}
	nxj_draw_char_32bit('A',0,0);
	SDL_UnlockSurface(nxj_lcd_screen);
	SDL_BlitSurface(nxj_lcd_screen, NULL, screen_surface, NULL);
	SDL_Flip(screen_surface);
	//sleep(3);
	sdl_is_init=2;
	return 0;
}
void nxj_draw_char_4bit(unsigned char c, int x,int y){
	if (nxj_lcd_screen==NULL) return;
	c= c &0x7f; //% 128 ->only ASCII 
	if (SDL_LockSurface(nxj_lcd_screen)==0)
	{
		int fw=5;
		int fh=8;
		int lastX=x+fw-1;
		int lastY=y+fh-1;
		int cy,cx;
		//
		unsigned char *pixels=(unsigned char *)nxj_lcd_screen->pixels;
		//{
		for (cx=0;cx<5;cx++,x++)
		{
				unsigned char cul_column=nxj_font[c][cx];
				for (cy=0;cy<8;cy++){
					int pos=(y+cy)*240+x;
					int pos_2= pos>>1;
					int pos_mod_2= pos & 1;
					unsigned char *cur=pixels+pos_2;
					if (cul_column & 1)//black
					{
						if (pos_mod_2){
							//clear 4 hi bits
							*cur=(*cur & 0xf0);//0xf0=% 0b11110000 
						}else{
							//clear 4 low bits
							*cur=(*cur & 0x0f);//0xf=0b1111
						}
					}else{ //white
						
						if (pos_mod_2){
							//set hi bits
							*cur=(*cur | 0xf0);//0xf0=% 0b11110000 
						}else{
							//set lo bits
							*cur=(*cur | 0x0f);//0xf=0b1111
						}
					}
					cul_column=cul_column >>1;//shift
				}
				
		}
		
		//}
	}else{
		fprintf(stderr,"\nErr lockSurface:%s",SDL_GetError());
	}
	SDL_UnlockSurface(nxj_lcd_screen);	
}

Uint32 bg32bit_color=0xffffffff;
Uint32 fg32bit_color=0;

void nxj_draw_char_32bit(unsigned char c, int x,int y)
{
	char buf[32];
	int oldX=x;
	if (nxj_lcd_screen==NULL) return;
	c= c &0x7f; //% 128 ->only ASCII 
	
	if (SDL_LockSurface(nxj_lcd_screen)==0)
	{
		int fw=5;
		int fh=8;
		int lastX=x+fw-1;
		int lastY=y+fh-1;
		int cy,cx;
		int pos;
		Uint32 *cur;
		//
		unsigned char *pixels=(unsigned char *)nxj_lcd_screen->pixels;
		//{
		for (cx=0;cx<5;cx++,x++)
		{
				unsigned char cul_column=nxj_font[c][cx];
				//buf[0]=0;buf[8]=0;
				for (cy=0;cy<8;cy++){
					pos=(y+cy)*240+x;
					//int pos_2= pos>>1;
					//int pos_mod_2= pos & 1;
					//unsigned char *cur=pixels+pos_2;
					cur=((Uint32*)pixels)+pos;
					if (cul_column & 1)//IN 0 BIT
					{
						//black
						//printf("\npix is black");
						//buf[cy]='*';
						*cur=fg32bit_color;
					}else{ //white
						//printf("\npix is white");
						*cur=bg32bit_color;;
						//buf[cy]=' ';
					}
					cul_column=cul_column >>1;//shift
					
				}
				//fprintf(stderr,"\n%s",buf);
				
		}
		//cx=4;
		x=oldX+5;
		//BACKGROUND ON CX==5(POSITION 6)
		for (cy=0;cy<8;cy++)
		{
			pos=(y+cy)*240+x;
			cur=((Uint32*)pixels)+pos;
			*cur=bg32bit_color;
		}
		//}
	}
	else{
		fprintf(stderr,"\nErr lockSurface:%s",SDL_GetError());
	}
	SDL_UnlockSurface(nxj_lcd_screen);	
}

void nxj_blit_bits_to_array(array_t*  src,
                          int sw,
                          int sh,
                          int sx,
                          int sy,
                         array_t*  dst,
						int dw,
                          int dh,
                          int x,
                          int y,
                          int w,
                          int h,
                          int rop,
						  int is_display_array)
{
	unsigned char* dst_data;
	unsigned char* src_data;
	unsigned char* surf_pixels;
	unsigned char pix,src_pix,dst_pix,d_pix;
	int cx,cy,src_x,src_y;
	int dst_base_line;
	int dst_base_line_y;
	int src_base_line;
	int src_remX;
	int dst_remX;
	int dst_index;
	int src_index;
	
	dst_data=(unsigned char*)array_get_data(dst);
	src_data=(unsigned char*)array_get_data(dst);
	surf_pixels=NULL;
	if (is_display_array && (sdl_is_init>=2))
		if (SDL_LockSurface(nxj_lcd_screen)==0){
			surf_pixels=(unsigned char *)nxj_lcd_screen->pixels;
		}

	for (cy=y,src_y=sy;(cy<h)&&(cy<dh);cy++,src_y++){
		//base_line_y=cy*dw;
		for (cx=x;(cx<w)&&(cx<dw);cx++){
			//base_line=base_line_y + cx/8;
			dst_index=(cy*dw+cx);///8;
			dst_base_line=dst_index /8;
			dst_remX=dst_index % 8;
			src_index=(src_y*sw)+(sx+cx-x);
			src_base_line=src_index /8 ;
			src_remX=src_index% 8;
			src_pix=src_data[src_base_line];
			pix=src_pix & (1<<src_remX);
			pix=(pix!=0);
			dst_pix=dst_data[dst_base_line];
			d_pix=dst_pix & (1<<dst_remX);
			d_pix=(d_pix!=0);
			//try get pixel from src
			switch(rop){
				case ROP_AND:
					pix=pix & d_pix;
					if (pix)
						dst_pix=(dst_pix | (1<<dst_remX));
					else 
						dst_pix=(dst_pix & ~(1<<dst_remX));
					break;
				case ROP_ANDINVERTED:
					pix=!(pix & d_pix);
					if (pix)
						dst_pix=(dst_pix | (1<<dst_remX));
					else 
						dst_pix=(dst_pix & ~(1<<dst_remX));
					break;
				case ROP_ANDREVERSE:
					pix=(!pix) & (!d_pix);
					if (pix)
						dst_pix=(dst_pix | (1<<dst_remX));
					else 
						dst_pix=(dst_pix & ~(1<<dst_remX));
					break;
					break;
				case ROP_CLEAR:
					dst_pix=(dst_pix & ~(1<<dst_remX));
					break;
				case ROP_COPY:
					if (pix)
						dst_pix=(dst_pix | (1<<dst_remX));
					else 
						dst_pix=(dst_pix & ~(1<<dst_remX));
					break;
				case ROP_COPYINVERTED:
					if (!pix)
						dst_pix=(dst_pix | (1<<dst_remX));
					else 
						dst_pix=(dst_pix & ~(1<<dst_remX));
					break;
				case ROP_EQUIV:
					break;
				case ROP_INVERT:
					
					break;
				case ROP_NAND:
					pix=!(pix & d_pix);
					if (pix)
						dst_pix=(dst_pix | (1<<dst_remX));
					else 
						dst_pix=(dst_pix & ~(1<<dst_remX));
					break;
				case ROP_NOOP:
					break;
				case ROP_NOR:
					pix=!(pix | d_pix);
					if (pix)
						dst_pix=(dst_pix | (1<<dst_remX));
					else 
						dst_pix=(dst_pix & ~(1<<dst_remX));
					break;
				case ROP_OR:
					pix=pix | d_pix;
					if (pix)
						dst_pix=(dst_pix | (1<<dst_remX));
					else 
						dst_pix=(dst_pix & ~(1<<dst_remX));
					break;
				case ROP_ORINVERTED:
					break;
				case ROP_ORREVERSE:
					break;
				case ROP_SET:
					break;
				case ROP_XOR:
					break;
			
			}
			dst_data[dst_base_line]=dst_pix;
			if (surf_pixels){
				pix=dst_pix & (1<<dst_remX);
				int byte_ix=(cy*default_screen_width+cx)*4;
				if (pix){
					*(uint32_t*)(surf_pixels+byte_ix)=fg32bit_color;
				}else{
					*(uint32_t*)(surf_pixels+byte_ix)=bg32bit_color;
				}	
			}
			
		}
	}
	if (surf_pixels){
		SDL_UnlockSurface(nxj_lcd_screen);
	}
}

