/*
 * Copyright (c) 2005
 * William R. Speirs
 *
 * Permission to use, copy, distribute, or modify this software for
 * the purpose of education is herby granted without fee. Permission
 * to sell this software or its documentation is hereby denied without
 * first obtaining the written consent of the author. In all cases, the 
 * above copyright notice must appear and this permission notice must 
 * appear in the supporting documentation. William R. Speirs makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */


/** @file VideoDriver.h
 * Contains the base class for video drivers and an implementation of the VGA video driver.
 */

#ifndef VIDEODRIVER_H
#define VIDEODRIVER_H


#include <constants.h>
#include <types.h>
#include <mem_utils.h>


/** @class VideoDriver
 *
 * @brief The base class for all video drivers.
 *
 **/

class VideoDriver
{
public:
	/**
	 * The base constructor for the video driver. Clears the screen.
	 */
	VideoDriver()
	{  }
	
	/**
	 * An enumeration of the available colors.
	 */
	enum Color {
		RED,
		GREEN, 
		BLUE, 
		WHITE,
		GREY
	};
	
	/**
	 * Changes the color of the font.
	 * @param fc The new font color.
	 */
	virtual void SetFontColor(Color fc) = 0;

	/**
	 * Puts a character on the screen at the current position.
	 * (Maybe allow it to move the current position later, but usually has defaults?)
	 * @param c The character to place on the screen.
	 * @param refresh Indicates if the screen should be refreshed.
	 */
	virtual void PutCharacter(char c, bool refresh = true) = 0;
	
	/**
	 * Clears the screen to black.
	 */
	virtual void ClearScreen() = 0;
	
	/**
	 * Refreshes the screen.
	 */
	virtual void RefreshScreen() = 0;
};

class VGADriver : public VideoDriver
{
public:
	/**
	 * The constructor for the VGA video driver.
	 * Creates the memory for the screen buffer.
	 */
	VGADriver();
	
	/**
	 * The destructor for the VGA video driver.
	 */
	~VGADriver();
	
	/**
	 * Changes the color of the font.
	 * @param fc The new font color.
	 */
	void SetFontColor(Color fc);
	
	/**
	 * Puts a character on the screen at the current position.
	 * @param c The character to place on the screen.
	 * @param refresh Indicates if the screen should be refreshed.
	 */
	void PutCharacter(char c, bool refresh = true);
	
	/**
	 * Clears the screen to black.
	 */
	void ClearScreen();
	
	/**
	 * Copies the screen buffer into the video memory.
	 */
	void RefreshScreen();
	
private:
	uchar			color;		///< current character color
	int			xpos, ypos;	///< x & y positions
	const static int	maxX = 80;	///< The max value in the x direction
	const static int	maxY = 24;	///< The max value in the y direction
	volatile uchar		*videoAddr;	///< The address where the video memory resides
	uint			bufferSize;	///< The size of the buffer in bytes
	uchar			*screenBuffer;	///< The buffer that holds what should be on the screen
	
	// font colors
	const static uchar	FG_BLACK		= 0x00;
	const static uchar	FG_BLUE			= 0x01;
	const static uchar	FG_GREEN		= 0x02;
	const static uchar	FG_GREEN_BLUE		= 0x03;
	const static uchar	FG_RED			= 0x04;
	const static uchar	FG_PURPLE		= 0x05;
	const static uchar	FG_BROWN		= 0x06;
	const static uchar	FG_LIGHT_GREY		= 0x07;
	const static uchar	FG_DARK_GREY		= 0x08;
	const static uchar	FG_LIGHT_BLUE		= 0x09;
	const static uchar	FG_LIGHT_GREEN		= 0x0A;
	const static uchar	FG_LIGHT_LIGHT_BLUE	= 0x0B;
	const static uchar	FG_PINK			= 0x0C;
	const static uchar	FG_LIGHT_PURPLE		= 0x0D;
	const static uchar	FG_YELLOW		= 0x0E;
	const static uchar	FG_WHITE		= 0x0F;

};


#endif // VideoDriver.h


