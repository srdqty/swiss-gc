#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gccore.h>
#include <ogc/exi.h>
#include <ogc/machine/processor.h>
#include "deviceHandler.h"
#include "FrameBufferMagic.h"
#include "IPLFontWrite.h"
#include "swiss.h"
#include "main.h"
#include "ata.h"
#include "exi.h"
#include "bba.h"
#include "wkf.h"

char topStr[256];

char *getSramLang(u8 lang) {
	switch(lang) {
		case 5:
			return "Dutch";
		case 4:
			return "Italian";
		case 3:
			return "Spanish";
		case 2:
			return "French";
		case 1:
			return "German";
		case 0:
			return "English";
	}
	return "Unknown";
}

uiDrawObj_t * info_draw_page(int page_num) {
	uiDrawObj_t *container = DrawEmptyBox(20,60, getVideoMode()->fbWidth-20, 420);
	syssram* sram = __SYS_LockSram();
	__SYS_UnlockSram(0);
	
	// System Info (Page 1/3)
	if(!page_num) {
		DrawAddChild(container, DrawLabel(30, 65, "System Info (1/3):"));
		// Model
		if(is_gamecube()) {
			if(*(u32*)&driveVersion[0] == 0x20010831) {
				sprintf(topStr, "Panasonic Q SL-GC10-S");
			}
			else if(IPLInfo[0x55]=='M') {							// MPAL 1.1 (Brazil)
				sprintf(topStr, "Nintendo GameCube DOL-002 (BRA)");
			}
			else if((!IPLInfo[0x55])									// NTSC 1.0 
					|| (IPLInfo[0x55] == 'P' && IPLInfo[0x65]=='0')		// PAL 1.0 
					|| (IPLInfo[0x55] != 'P' && IPLInfo[0x65]=='1')) {	// NTSC 1.1
				sprintf(topStr, "Nintendo GameCube DOL-001");
			}
			else if((IPLInfo[0x55] == 'P' && IPLInfo[0x65]=='0')	// PAL 1.1
					|| IPLInfo[0x65]=='2') {						// NTSC 1.2
				sprintf(topStr, "Nintendo GameCube DOL-101");
			}
		}
		else {
			sprintf(topStr, "Nintendo Wii");
		}
		DrawAddChild(container, DrawStyledLabel(640/2, 110, topStr, 1.0f, true, defaultColor));
		// IPL version string
		if(is_gamecube()) {
			if(!IPLInfo[0x55]) {
				sprintf(topStr, "NTSC Revision 1.0");
			}
			else {
				sprintf(topStr, "%s", &IPLInfo[0x55]);
			}
		}
		else {
			sprintf(topStr, "Wii IPL");
		}
		DrawAddChild(container, DrawStyledLabel(640/2, 140, topStr, 1.0f, true, defaultColor));
		if(swissSettings.hasDVDDrive) {
			if((!__wkfSpiReadId() || (__wkfSpiReadId() == 0xFFFFFFFF))) {
				sprintf(topStr, "DVD Drive %02X %02X%02X/%02X (%02X)",driveVersion[2],driveVersion[0],driveVersion[1],driveVersion[3],driveVersion[4]);
			} else {
				sprintf(topStr, "WKF Serial %s",wkfGetSerial());
			}
		}
		else {
			sprintf(topStr, "No DVD Drive present");
		}
		DrawAddChild(container, DrawStyledLabel(640/2, 170, topStr, 1.0f, true, defaultColor));
		sprintf(topStr, "%s", getVideoModeString());
		DrawAddChild(container, DrawStyledLabel(640/2, 200, topStr, 1.0f, true, defaultColor));
		sprintf(topStr,"%s / %s",getSramLang(sram->lang), sram->flags&4 ? "Stereo":"Mono");
		DrawAddChild(container, DrawStyledLabel(640/2, 230, topStr, 1.0f, true, defaultColor));
		// GC 00083214, 00083410
		sprintf(topStr,"CPU Revision/Version (%08X)",mfpvr());
		DrawAddChild(container, DrawStyledLabel(640/2, 290, topStr, 0.75f, true, defaultColor));
		sprintf(topStr,"CPU Unique ECID %08X:%08X:%08X",mfspr(0x39C),mfspr(0x39D),mfspr(0x39E));
		DrawAddChild(container, DrawStyledLabel(640/2, 320, topStr, 0.75f, true, defaultColor));
	}
	else if(page_num == 1) {
		DrawAddChild(container, DrawLabel(30, 65, "Device Info (2/3):"));
		sprintf(topStr,"BBA: %s", exi_bba_exists() ? "Installed":"Not Present");
		DrawAddChild(container, DrawLabel(30, 110, topStr));
		if(exi_bba_exists()) {
			sprintf(topStr,"IP: %s", net_initialized ? bba_ip:"Not Available");
		}
		else {
			sprintf(topStr,"IP: Not Available");
		}
		DrawAddChild(container, DrawLabel(270, 110, topStr));
		sprintf(topStr,"Component Cable Plugged in: %s",VIDEO_HaveComponentCable()?"Yes":"No");
		DrawAddChild(container, DrawLabel(30, 140, topStr));
		if(usb_isgeckoalive(0)||usb_isgeckoalive(1)) {
			sprintf(topStr,"USB Gecko: Installed in %s",usb_isgeckoalive(0)?"Slot A":"Slot B");
		}
		else {
			sprintf(topStr,"USB Gecko: Not Present");
		}
		DrawAddChild(container, DrawLabel(30, 170, topStr));
		sprintf(topStr, "Current Device: %s", (devices[DEVICE_CUR] != NULL ? devices[DEVICE_CUR]->deviceName : "None"));
		DrawAddChild(container, DrawLabel(30, 200, topStr));
		sprintf(topStr, "Config Device: %s", (devices[DEVICE_CONFIG] != NULL ? devices[DEVICE_CONFIG]->deviceName : "None"));
		DrawAddChild(container, DrawLabel(30, 230, topStr));
	}
	else if(page_num == 2) {
		DrawAddChild(container, DrawLabel(30, 65, "Credits (3/3):"));
		DrawAddChild(container, DrawStyledLabel(640/2, 115, "Swiss version 0.4", 1.0f, true, defaultColor));
		DrawAddChild(container, DrawStyledLabel(640/2, 140, "by emu_kidid 2019", 0.75f, true, defaultColor));
		sprintf(txtbuffer, "Commit %s Revision %s", GITREVISION, GITVERSION);
		DrawAddChild(container, DrawStyledLabel(640/2, 165, txtbuffer, 0.75f, true, defaultColor));
		DrawAddChild(container, DrawStyledLabel(640/2, 210, "Thanks to", 0.75f, true, defaultColor));
		DrawAddChild(container, DrawStyledLabel(640/2, 228, "Testers & libOGC/dkPPC authors", 0.75f, true, defaultColor));
		DrawAddChild(container, DrawStyledLabel(640/2, 246, "Extrems, FIX94, megalomaniac, sepp256", 0.75f, true, defaultColor));
		DrawAddChild(container, DrawStyledLabel(640/2, 300, "Web/Support https://www.gc-forever.com/", 0.75f, true, defaultColor));
		DrawAddChild(container, DrawStyledLabel(640/2, 318, "Source at https://github.com/emukidid/swiss-gc", 0.75f, true, defaultColor));
		DrawAddChild(container, DrawStyledLabel(640/2, 354, "Visit us at #gc-forever on EFNet", 0.75f, true, defaultColor));
	}
	if(page_num != 2) {
		DrawAddChild(container, DrawLabel(520, 390, "->"));
	}
	if(page_num != 0) {
		DrawAddChild(container, DrawLabel(100, 390, "<-"));
	}
	DrawAddChild(container, DrawStyledLabel(640/2, 400, "Press A to return", 1.0f, true, defaultColor));
	return container;
}

void show_info() {
	int page = 0;
	uiDrawObj_t* pagePanel = NULL;
	while (PAD_ButtonsHeld(0) & PAD_BUTTON_A){ VIDEO_WaitVSync (); }
	while(1) {
		uiDrawObj_t* newPanel = info_draw_page(page);
		if(pagePanel != NULL) {
			DrawDispose(pagePanel);
		}
		pagePanel = newPanel;
		DrawPublish(pagePanel);
		while (!((PAD_ButtonsHeld(0) & PAD_BUTTON_RIGHT) 
			|| (PAD_ButtonsHeld(0) & PAD_BUTTON_LEFT) 
			|| (PAD_ButtonsHeld(0) & PAD_BUTTON_B)
			|| (PAD_ButtonsHeld(0) & PAD_BUTTON_A)
			|| (PAD_ButtonsHeld(0) & PAD_TRIGGER_R)
			|| (PAD_ButtonsHeld(0) & PAD_TRIGGER_L)))
			{ VIDEO_WaitVSync (); }
		u16 btns = PAD_ButtonsHeld(0);
		if(((btns & PAD_BUTTON_RIGHT) || (PAD_ButtonsHeld(0) & PAD_TRIGGER_R)) && page < 2)
			page++;
		if(((btns & PAD_BUTTON_LEFT) || (PAD_ButtonsHeld(0) & PAD_TRIGGER_L)) && page > 0)
			page--;
		if((btns & PAD_BUTTON_A) || (btns & PAD_BUTTON_B))
			break;
		while ((PAD_ButtonsHeld(0) & PAD_BUTTON_RIGHT) 
			|| (PAD_ButtonsHeld(0) & PAD_BUTTON_LEFT) 
			|| (PAD_ButtonsHeld(0) & PAD_BUTTON_B)
			|| (PAD_ButtonsHeld(0) & PAD_BUTTON_A)
			|| (PAD_ButtonsHeld(0) & PAD_TRIGGER_R)
			|| (PAD_ButtonsHeld(0) & PAD_TRIGGER_L))
			{ VIDEO_WaitVSync (); }
	}
	DrawDispose(pagePanel);
	while (PAD_ButtonsHeld(0) & PAD_BUTTON_A){ VIDEO_WaitVSync (); }
}
