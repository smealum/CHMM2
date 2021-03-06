/*----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#------  This File is Part Of : ----------------------------------------------------------------------------------------#
#------- _  -------------------  ______   _   --------------------------------------------------------------------------#
#------ | | ------------------- (_____ \ | |  --------------------------------------------------------------------------#
#------ | | ---  _   _   ____    _____) )| |  ____  _   _   ____   ____   ----------------------------------------------#
#------ | | --- | | | | / _  |  |  ____/ | | / _  || | | | / _  ) / ___)  ----------------------------------------------#
#------ | |_____| |_| |( ( | |  | |      | |( ( | || |_| |( (/ / | |  --------------------------------------------------#
#------ |_______)\____| \_||_|  |_|      |_| \_||_| \__  | \____)|_|  --------------------------------------------------#
#------------------------------------------------- (____/  -------------------------------------------------------------#
#------------------------   ______   _   -------------------------------------------------------------------------------#
#------------------------  (_____ \ | |  -------------------------------------------------------------------------------#
#------------------------   _____) )| | _   _   ___   ------------------------------------------------------------------#
#------------------------  |  ____/ | || | | | /___)  ------------------------------------------------------------------#
#------------------------  | |      | || |_| ||___ |  ------------------------------------------------------------------#
#------------------------  |_|      |_| \____|(___/   ------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Licensed under the GPL License --------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Copyright (c) Nanni <lpp.nanni@gmail.com> ---------------------------------------------------------------------------#
#- Copyright (c) Rinnegatamante <rinnegatamante@gmail.com> -------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Credits : -----------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Smealum for ctrulib and ftpony src ----------------------------------------------------------------------------------#
#- StapleButter for debug font -----------------------------------------------------------------------------------------#
#- Lode Vandevenne for lodepng -----------------------------------------------------------------------------------------#
#- Jean-loup Gailly and Mark Adler for zlib ----------------------------------------------------------------------------#
#- xerpi for sf2dlib ---------------------------------------------------------------------------------------------------#
#- Special thanks to Aurelio for testing, bug-fixing and various help with codes and implementations -------------------#
#-----------------------------------------------------------------------------------------------------------------------*/
#include <3ds.h>
#include "include/luaplayer.h"
#include "include/Graphics/Graphics.h"
#include "voice.cpp"
#include "icons.cpp"
#include "icon.cpp"
extern "C"{
	#include "include/sf2d/sf2d.h"
}
extern Bitmap* decodePNGbuffer(unsigned char* in, u64 size);

struct gpu_text{
	u32 magic;
	u16 width;
	u16 height;
	sf2d_texture* tex;
};

int cur_screen;

static int lua_init(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");	
    sf2d_init();
	cur_screen = 2;
	sf2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
    return 0;
}

static int lua_term(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	cur_screen = 2;
    sf2d_fini();
    return 0;
}

static int lua_refresh(lua_State *L) {
    int argc = lua_gettop(L);
	if ((argc != 1) && (argc != 2))  return luaL_error(L, "wrong number of arguments");
	int screen = luaL_checkinteger(L,1);
	int side=0;
	if (argc == 2) side = luaL_checkinteger(L,2);
	gfxScreen_t my_screen;
	gfx3dSide_t eye;
	cur_screen = screen;
	if (screen == 0) my_screen = GFX_TOP;
	else my_screen = GFX_BOTTOM;
	if (side == 0) eye = GFX_LEFT;
	else eye = GFX_RIGHT;
    sf2d_start_frame(my_screen,eye);
    return 0;
}

static int lua_end(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
    sf2d_end_frame();
    return 0;
}

static int lua_flip(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
    sf2d_swapbuffers();
    return 0;
}

static int lua_rect(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments");
	float x1 = luaL_checknumber(L,1);
	float x2 = luaL_checknumber(L,2);
	float y1 = luaL_checknumber(L,3);
	float y2 = luaL_checknumber(L,4);
	float radius = 0;
	if (x2 < x1){
		int tmp = x2;
		x2 = x1;
		x1 = tmp;
	}
	if (y2 < y1){
		int tmp = y2;
		y2 = y1;
		y1 = tmp;
	}
	u32 color = luaL_checkinteger(L,5);
	if (argc == 6) radius = luaL_checknumber(L,6);
	if (radius == 0){
		#ifndef SKIP_ERROR_HANDLING
			if (cur_screen != 1 && cur_screen != 0) return luaL_error(L, "you need to call initBlend to use GPU rendering");
		#endif
		sf2d_draw_rectangle(x1, y1, x2-x1, y2-y1, RGBA8((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color) & 0xFF, (color >> 24) & 0xFF));
    }else{
		#ifndef SKIP_ERROR_HANDLING
			if (cur_screen != 1 && cur_screen != 0) return luaL_error(L, "you need to call initBlend to use GPU rendering");
		#endif
		sf2d_draw_rectangle_rotate(x1, y1, x2-x1, y2-y1, RGBA8((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color) & 0xFF, (color >> 24) & 0xFF), radius);
	}
	return 0;
}

static int lua_fillcircle(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments");
	float x = luaL_checknumber(L,1);
	float y = luaL_checknumber(L,2);
	int radius = luaL_checkinteger(L,3);
	u32 color = luaL_checkinteger(L,4);
	#ifndef SKIP_ERROR_HANDLING
		if (cur_screen != 1 && cur_screen != 0) return luaL_error(L, "you need to call initBlend to use GPU rendering");
	#endif
	sf2d_draw_fill_circle(x, y, radius, RGBA8((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color) & 0xFF, (color >> 24) & 0xFF));
	return 0;
}


static int lua_line(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 5) return luaL_error(L, "wrong number of arguments");
	float x1 = luaL_checknumber(L,1);
	float x2 = luaL_checknumber(L,2);
	float y1 = luaL_checknumber(L,3);
	float y2 = luaL_checknumber(L,4);
	#ifndef SKIP_ERROR_HANDLING
		if (cur_screen != 1 && cur_screen != 0) return luaL_error(L, "you need to call initBlend to use GPU rendering");
	#endif
	u32 color = luaL_checkinteger(L,5);
    sf2d_draw_line(x1, y1, x2, y2, RGBA8((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color) & 0xFF, (color >> 24) & 0xFF));
    return 0;
}

static int lua_emptyrect(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 5) return luaL_error(L, "wrong number of arguments");
	float x1 = luaL_checknumber(L,1);
	float x2 = luaL_checknumber(L,2);
	float y1 = luaL_checknumber(L,3);
	float y2 = luaL_checknumber(L,4);
	#ifndef SKIP_ERROR_HANDLING
		if (cur_screen != 1 && cur_screen != 0) return luaL_error(L, "you need to call initBlend to use GPU rendering");
	#endif
	u32 color = luaL_checkinteger(L,5);
    sf2d_draw_line(x1, y1, x1, y2, RGBA8((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color) & 0xFF, (color >> 24) & 0xFF));
    sf2d_draw_line(x2, y1, x2, y2, RGBA8((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color) & 0xFF, (color >> 24) & 0xFF));
	sf2d_draw_line(x1, y2, x2, y2, RGBA8((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color) & 0xFF, (color >> 24) & 0xFF));
	sf2d_draw_line(x1, y1, x2, y1, RGBA8((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color) & 0xFF, (color >> 24) & 0xFF));
	return 0;
}

static int lua_assets(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");	
	Bitmap* bitmap1 = decodePNGbuffer(icon_png,size_icon_png);
	Bitmap* bitmap2 = decodePNGbuffer(icons_png,size_icons_png);
	Bitmap* bitmap3 = decodePNGbuffer(voice_png,size_voice_png);
	sf2d_texture *tex = sf2d_create_texture_mem_RGBA8(bitmap1->pixels, bitmap1->width, bitmap1->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	gpu_text* result = (gpu_text*)malloc(sizeof(gpu_text));
	result->magic = 0x4C545854;
	result->tex = tex;
	result->width = bitmap1->width;
	result->height = bitmap1->height;
	free(bitmap1->pixels);
	free(bitmap1);
	sf2d_texture *tex2 = sf2d_create_texture_mem_RGBA8(bitmap2->pixels, bitmap2->width, bitmap2->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	gpu_text* result2 = (gpu_text*)malloc(sizeof(gpu_text));
	result2->magic = 0x4C545854;
	result2->tex = tex2;
	result2->width = bitmap2->width;
	result2->height = bitmap2->height;
	free(bitmap2->pixels);
	free(bitmap2);
	sf2d_texture *tex3 = sf2d_create_texture_mem_RGBA8(bitmap3->pixels, bitmap3->width, bitmap3->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	gpu_text* result3 = (gpu_text*)malloc(sizeof(gpu_text));
	result3->magic = 0x4C545854;
	result3->tex = tex3;
	result3->width = bitmap3->width;
	result3->height = bitmap3->height;
	free(bitmap3->pixels);
	free(bitmap3);
    lua_pushinteger(L, (u32)(result));
	lua_pushinteger(L, (u32)(result2));
	lua_pushinteger(L, (u32)(result3));
    return 3;
}

static int lua_loadimg(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	char* text = (char*)(luaL_checkstring(L, 1));
	Handle fileHandle;
	u32 bytesRead;
	u16 magic;
	u64 long_magic;
	FS_path filePath=FS_makePath(PATH_CHAR, text);
	FS_archive script=(FS_archive){ARCH_SDMC, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FSUSER_OpenFileDirectly(NULL, &fileHandle, script, filePath, FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	FSFILE_Read(fileHandle, &bytesRead, 0, &magic, 2);
	Bitmap* bitmap;
	if (magic == 0x5089){
		FSFILE_Read(fileHandle, &bytesRead, 0, &long_magic, 8);
		FSFILE_Close(fileHandle);
		svcCloseHandle(fileHandle);
		if (long_magic == 0x0A1A0A0D474E5089) bitmap = decodePNGfile(text);
	}else if (magic == 0x4D42){
		FSFILE_Close(fileHandle);
		svcCloseHandle(fileHandle);
		bitmap = decodeBMPfile(text);
	}else if (magic == 0xD8FF){
		FSFILE_Close(fileHandle);
		svcCloseHandle(fileHandle);
		bitmap = decodeJPGfile(text);
	}
	if(!bitmap) return luaL_error(L, "Error loading image");
	if (bitmap->bitperpixel == 24){
		int length = bitmap->width * bitmap->height * 4;
		u8* real_pixels = (u8*)malloc(length);
		int i = 0;
		int z = 0;
		while (i < length){
			real_pixels[i] = bitmap->pixels[z];
			real_pixels[i+1] = bitmap->pixels[z+1];
			real_pixels[i+2] = bitmap->pixels[z+2];
			real_pixels[i+3] = 0xFF;
			i = i + 4;
			z = z + 3;
		}
		free(bitmap->pixels);
		bitmap->pixels = real_pixels;
	}
	sf2d_texture *tex = sf2d_create_texture_mem_RGBA8(bitmap->pixels, bitmap->width, bitmap->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	gpu_text* result = (gpu_text*)malloc(sizeof(gpu_text));
	result->magic = 0x4C545854;
	result->tex = tex;
	result->width = bitmap->width;
	result->height = bitmap->height;
	free(bitmap->pixels);
	free(bitmap);
    lua_pushinteger(L, (u32)(result));
	return 1;
}

static int lua_convert(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	Bitmap* bitmap = (Bitmap*)(luaL_checkinteger(L, 1));
	#ifndef SKIP_ERROR_HANDLING
		if (bitmap->magic != 0x4C494D47) return luaL_error(L, "attempt to access wrong memory block type");
	#endif
	u8* real_pixels;
	u8* flipped = (u8*)malloc(bitmap->width * bitmap->height * (bitmap->bitperpixel / 8));
	flipped = flipBitmap(flipped, bitmap);
	int length = bitmap->width * bitmap->height * 4;
	if (bitmap->bitperpixel == 24){		
		real_pixels = (u8*)malloc(length);
		int i = 0;
		int z = 0;
		while (i < length){
			real_pixels[i] = flipped[z+2];
			real_pixels[i+1] = flipped[z+1];
			real_pixels[i+2] = flipped[z];
			real_pixels[i+3] = 0xFF;
			i = i + 4;
			z = z + 3;
		}
		free(flipped);
	}else{
		real_pixels = flipped;
		int i = 0;
		while (i < length){
			u8 tmp = real_pixels[i+2];
			real_pixels[i+2] = real_pixels[i];
			real_pixels[i] = tmp;
			i = i + 4;
		}
	}
	sf2d_texture *tex = sf2d_create_texture_mem_RGBA8(real_pixels, bitmap->width, bitmap->height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	gpu_text* result = (gpu_text*)malloc(sizeof(gpu_text));
	result->magic = 0x4C545854;
	result->tex = tex;
	result->width = bitmap->width;
	result->height = bitmap->height;
	free(real_pixels);
    lua_pushinteger(L, (u32)(result));
	return 1;
}

static int lua_drawimg(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 3 && argc != 4) return luaL_error(L, "wrong number of arguments");
	float x = luaL_checknumber(L,1);
	float y = luaL_checknumber(L,2);
	gpu_text* texture = (gpu_text*)luaL_checkinteger(L,3);
	u32 color = 0;
	if (argc == 4){ 
		color = luaL_checkinteger(L,4);
		#ifndef SKIP_ERROR_HANDLING
			if (texture->magic != 0x4C545854) return luaL_error(L, "attempt to access wrong memory block type");
			if (cur_screen != 1 && cur_screen != 0) return luaL_error(L, "you need to call initBlend to use GPU rendering");
		#endif
		sf2d_draw_texture_blend(texture->tex, x, y, RGBA8((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color) & 0xFF, (color >> 24) & 0xFF));
	}else{
		#ifndef SKIP_ERROR_HANDLING
			if (texture->magic != 0x4C545854) return luaL_error(L, "attempt to access wrong memory block type");
			if (cur_screen != 1 && cur_screen != 0) return luaL_error(L, "you need to call initBlend to use GPU rendering");
		#endif
		sf2d_draw_texture(texture->tex, x, y);
	}
	return 0;
}

static int lua_drawimg_scale(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 5 && argc != 6) return luaL_error(L, "wrong number of arguments");
	float x = luaL_checknumber(L,1);
	float y = luaL_checknumber(L,2);
	gpu_text* texture = (gpu_text*)luaL_checkinteger(L,3);
	float scale_x = luaL_checknumber(L,4);
	float scale_y = luaL_checknumber(L,5);
	u32 color;
	#ifndef SKIP_ERROR_HANDLING
		if (texture->magic != 0x4C545854) return luaL_error(L, "attempt to access wrong memory block type");
		if (cur_screen != 1 && cur_screen != 0) return luaL_error(L, "you need to call initBlend to use GPU rendering");
	#endif
	if (argc == 6){
		color = luaL_checkinteger(L,6);
		sf2d_draw_texture_scale_blend(texture->tex, x, y, scale_x, scale_y, RGBA8((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color) & 0xFF, (color >> 24) & 0xFF));
	}else sf2d_draw_texture_scale(texture->tex, x, y, scale_x, scale_y);
	return 0;
}

static int lua_drawimg_full(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 10) return luaL_error(L, "wrong number of arguments");
	float x = luaL_checknumber(L,1);
	float y = luaL_checknumber(L,2);
	int st_x = luaL_checkinteger(L, 3);
    int st_y = luaL_checkinteger(L, 4);
	int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
	float radius = luaL_checknumber(L, 7);
	float scale_x = luaL_checknumber(L, 8);
	float scale_y = luaL_checknumber(L, 9);
	gpu_text* texture = (gpu_text*)luaL_checkinteger(L, 10);
	#ifndef SKIP_ERROR_HANDLING
		if (texture->magic != 0x4C545854) return luaL_error(L, "attempt to access wrong memory block type");
		if (cur_screen != 1 && cur_screen != 0) return luaL_error(L, "you need to call initBlend to use GPU rendering");
	#endif
	sf2d_draw_texture_part_rotate_scale(texture->tex, x, y, radius, st_x, st_y, width, height, scale_x, scale_y);
	return 0;
}

static int lua_partial(lua_State *L){
	int argc = lua_gettop(L);
	if (argc != 7 && argc != 8) return luaL_error(L, "wrong number of arguments");
	float x = luaL_checknumber(L,1);
	float y = luaL_checknumber(L,2);
	int st_x = luaL_checkinteger(L, 3);
    int st_y = luaL_checkinteger(L, 4);
	int width = luaL_checkinteger(L, 5);
    int height = luaL_checkinteger(L, 6);
	gpu_text* file = (gpu_text*)luaL_checkinteger(L, 7);
	u32 color;
	if (argc == 8) color = luaL_checkinteger(L, 8);
	#ifndef SKIP_ERROR_HANDLING
		if (file->magic != 0x4C545854) return luaL_error(L, "attempt to access wrong memory block type");
		if (cur_screen != 1 && cur_screen != 0) return luaL_error(L, "you need to call initBlend to use GPU rendering");
	#endif
	if (argc == 8) sf2d_draw_texture_part_blend(file->tex, x, y, st_x, st_y, width, height, RGBA8((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color) & 0xFF, (color >> 24) & 0xFF));
	else sf2d_draw_texture_part(file->tex, x, y, st_x, st_y, width, height);
	return 0;
}

static int lua_free(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	gpu_text* texture = (gpu_text*)luaL_checkinteger(L,1);
	#ifndef SKIP_ERROR_HANDLING
		if (texture->magic != 0x4C545854) return luaL_error(L, "attempt to access wrong memory block type");
	#endif
	sf2d_free_texture(texture->tex);
	free(texture);
	return 0;
}

static int lua_getWidth(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	gpu_text* src = (gpu_text*)luaL_checkinteger(L, 1);
	#ifndef SKIP_ERROR_HANDLING
		if (src->magic != 0x4C545854) return luaL_error(L, "attempt to access wrong memory block type");
	#endif
	lua_pushinteger(L,src->width);
	return 1;
}

static int lua_getHeight(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	gpu_text* src = (gpu_text*)luaL_checkinteger(L, 1);
	#ifndef SKIP_ERROR_HANDLING
		if (src->magic != 0x4C545854) return luaL_error(L, "attempt to access wrong memory block type");
	#endif
	lua_pushinteger(L,src->height);
	return 1;
}

//Register our Graphics Functions
static const luaL_Reg Graphics_functions[] = {
  {"init",					lua_init},
  {"loadAssets",			lua_assets},
  {"term",					lua_term},
  {"initBlend",				lua_refresh},
  {"loadImage",				lua_loadimg},
  {"drawImage",				lua_drawimg},
  {"drawPartialImage",		lua_partial},
  {"drawScaleImage",		lua_drawimg_scale},
  {"drawImageExtended",		lua_drawimg_full},
  {"fillRect",				lua_rect},
  {"fillEmptyRect",			lua_emptyrect},
  {"drawCircle",			lua_fillcircle},
  {"drawLine",				lua_line},
  {"termBlend",				lua_end},
  {"flip",					lua_flip},
  {"freeImage",				lua_free},
  {"getImageWidth",			lua_getWidth},
  {"getImageHeight",		lua_getHeight}, 
  {"convertFrom",			lua_convert},
  {0, 0}
};

void luaGraphics_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Graphics_functions, 0);
	lua_setglobal(L, "Graphics");
}