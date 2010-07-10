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


/** @file VesaVideo.h
 *
 * Contains the code for dealing with VESA support.
 */

#ifndef VESAVIDEO_H
#define VESAVIDEO_H

#include <constants.h>
#include <types.h>
#include <VideoDriver.h>
#include <VirtualConsole.h>
#include <VirtualConsoleManager.h>
#include <PhysicalMemManager.h>
#include <ProcessManager.h>

#include <vector.h>

using namespace k_std;

/**
 * @class VESAVideo
 * 
 * This class deals with everything that is needed to use VESA frame buffer support.
 */
class VESAVideo : public VideoDriver
{
public:
	/**
	 * Constructs the VESAVideo object and polls the modes.
	 */
	VESAVideo();
	
	/**
	 * Prints the modes and their corresponding mode numbers to the given console.
	 * @param console The console to print the modes to
	 */
	void PrintModes(VirtualConsole &console = *VirtualConsoleManager::GetInstance().GetCurrentConsole());
	
	/**
	 * Switches from one video mode to another.
	 * @param mode The mode to switch to.
	 * @return True if the switch works, false otherwise.
	 */
	bool SwitchMode(ushort mode);
	
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
	 * Refreshes the screen from the screen buffer.
	 */
	void RefreshScreen();
	
	/**
	 * Draws whatever hasn't been rendered from the buffer
	 */
	void UpdateScreen();
	
	/**
	 * Draws what is in the screen buffer, but in all black.
	 * MUCH faster than calling clear screen.
	 */
	void BlackScreen();
	
private:
	/**
	 * @struct VGAMode
	 * A structure to store the important mode information.
	 */
	struct VGAMode
	{
		ushort	modeNumber;	///< The VESA mode number
		ushort	xresolution;	///< Horizontal resolution in pixel or chars
		ushort	yresolution;	///< Vertical resolution in pixel or chars
//		ushort	bytesperscanline;	///< The number of bytes per line
		ulong	physicalAddr;	///< The physical address of this mode
	};
	
	/**
	 * Draws a single character to video memory.
	 */
	void DrawCharacter(uint x, uint y, char c);

	ushort			currentMode;	///< The current video mode
	vector<VGAMode>		modes;		///< A vector of VGA mode structs
	PhysicalMemManager	*physMem;	///< A pointer to the physical memory manager
	ProcessManager		&procMan;	///< A reference to the process manager
	ulong			myPageDir;	///< The page dir used for all the threads
	Registers		regs;		///< The registers for the V86Thread
	Thread			*v86Thread;	///< The V86 thread pointer
	ulong			color;		///< The current font color
	ulong			xpos, ypos;	///< The current x & y position in the character buffer
	ulong			lastPos;	///< The last values of xpos & ypos
	ulong			maxX, maxY;	///< The max x & y positions in the character buffer
	ulong			videoMemSize;	///< The size of the video memory in <b>bytes</b>
	volatile ulong		*videoAddr;	///< The address where the video memory resides
	ulong			bufferSize;	///< The size of the buffer in bytes
	char			*screenBuffer;	///< The buffer that holds what should be on the screen
	
	// these are all dependant upon the font used
	const static int	FONTDATAMAX = 2048;
	const static int	charHeight = 8;		///< Char height in pixels
	const static int	charBoxHeight = charHeight + 1;	///< Num of pixesl for the char and the space
	const static int	charWidth = 8;		///< Char width in pixels
	const static int	charBoxWidth = charWidth + 1;	///< Num of pixels for the char and the space
	const static unsigned char	font[VESAVideo::FONTDATAMAX];	///< The pixel map for the font
};

struct VBEInfo
{
	char	vbesignature[4];   ///< VBE Signature
	ushort	vbeversion;        ///< VBE Version
	ulong	oemstringptr;      ///< Pointer to OEM String
	uchar	capabilities[4];   ///< Capabilities of graphics cont.
	ushort	videomodeptr[2];   ///< Pointer to Video Mode List
	ushort	totalmemory;       ///< number of 64Kb memory blocks
	ushort	oemsoftwarerev;    ///< VBE implementation Software revision
	ulong	oemvendornameptr;  ///< Pointer to Vendor Name String
	ulong	oemproductnameptr; ///< Pointer to Product Name String
	ulong	oemproductrevptr;  ///< Pointer to Product Revision String
	char	reserved[222];     ///< Reserved for VBE implementation scratch area
	char	oemdata[256];      ///< Data Area for OEM Strings
} __attribute__((packed));


struct VGAModeInfoBlock
{
   // Mandatory information for all VBE revision
   ushort modeattributes;		// Mode attributes
   uchar winaattributes;		// Window A attributes
   uchar winbattributes;		// Window B attributes
   ushort wingranularity;		// Window granularity
   ushort winsize;			// Window size
   ushort winasegment;		// Window A start segment
   ushort winbsegment;		// Window B start segment
   uint winfuncptr;			// pointer to window function
   ushort bytesperscanline;	// bytes per scan line

   // Mandatory information for VBE 1.2 and above
   ushort xresolution;		// Horizontal resolution in pixel or chars
   ushort yresolution;		// Vertical resolution in pixel or chars
   uchar xcharsize;           // Character cell width in pixel
   uchar ycharsize;           // Character cell height in pixel
   uchar numberofplanes;      // Number of memory planes
   uchar bitsperpixel;        // Bits per pixel
   uchar numberofbanks;       // Number of banks
   uchar memorymodel;         // Memory model type
   uchar banksize;            // Bank size in KB
   uchar numberofimagepages;  // Number of images
   uchar reserved1;           // Reserved for page function

   // Direct Color fields (required for direct/6 and YUV/7 memory models)
   uchar redmasksize;         // Size of direct color red mask in bits
   uchar redfieldposition;    // Bit position of lsb of red bask
   uchar greenmasksize;       // Size of direct color green mask in bits
   uchar greenfieldposition;  // Bit position of lsb of green bask
   uchar bluemasksize;        // Size of direct color blue mask in bits
   uchar bluefieldposition;   // Bit position of lsb of blue bask
   uchar rsvdmasksize;        // Size of direct color reserved mask in bits
   uchar rsvdfieldposition;   // Bit position of lsb of reserved bask
   uchar directcolormodeinfo; // Direct color mode attributes

   // Mandatory information for VBE 2.0 and above
   uint physbaseptr;		// Physical address for flat frame buffer
   uint offscreenmemoffset;	// Pointer to start of off screen memory
   ushort offscreenmemsize;	// Amount of off screen memory in 1Kb units
   uchar reserved2[206];	// Remainder of ModeInfoBlock
};

void PollHardware();

void VesaTest(void *arg);

void VesaSetup(void *arg);

#endif // VesaVideo.h


