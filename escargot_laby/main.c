#include <gb/gb.h>
#include <stdio.h>
#include <gb/console.h>
#include "backgroundtiles.c"
#include "backgroundmap.c"
#include "snail.c"

const char blankmap[1] = {0x00}; //for the empty thing in the canplayermove function
UINT8 playerlocation[2];
UBYTE debug;
UBYTE haskey;
UBYTE win = 0;


/*enum notes {
  C0, Cd0, D0, Dd0, E0, F0, Fd0, G0, Gd0, A0, Ad0, B0,
  C1, Cd1, D1, Dd1, E1, F1, Fd1, G1, Gd1, A1, Ad1, B1,
  C2, Cd2, D2, Dd2, E2, F2, Fd2, G2, Gd2, A2, Ad2, B2,
  C3, Cd3, D3, Dd3, E3, F3, Fd3, G3, Gd3, A3, Ad3, B3,
  C4, Cd4, D4, Dd4, E4, F4, Fd4, G4, Gd4, A4, Ad4, B4,
  C5, Cd5, D5, Dd5, E5, F5, Fd5, G5, Gd5, A5, Ad5, B5
};


const UWORD frequencies[] = {
  44, 156, 262, 363, 457, 547, 631, 710, 786, 854, 923, 986,
  1046, 1102, 1155, 1205, 1253, 1297, 1339, 1379, 1417, 1452, 1486, 1517,
  1546, 1575, 1602, 1627, 1650, 1673, 1694, 1714, 1732, 1750, 1767, 1783,
  1798, 1812, 1825, 1837, 1849, 1860, 1871, 1881, 1890, 1899, 1907, 1915,
  1923, 1930, 1936, 1943, 1949, 1954, 1959, 1964, 1969, 1974, 1978, 1982,
  1985, 1988, 1992, 1995, 1998, 2001, 2004, 2006, 2009, 2011, 2013, 2015
};



void play_note(UBYTE notevalue){
	UBYTE flo, fhi;
	UWORD freq = frequencies[notevalue];
	NR10_REG = 0x00;
	// Volume envelope
	NR12_REG = 0xF3;	// F=maximum volume, 3=sound fade out
	// Waveform
	NR11_REG = 0x80;	// 50% square wave
	// Frequency
	flo = (UBYTE)freq & 0xFF;
	fhi = (UBYTE)((freq & 0x0700)>>8);
	NR13_REG = flo;	// Take lower 8 bits from the function argument
	NR14_REG = 0x80 | fhi;

	// Take 3 more bits from the function argument, and set the start bit


}

void music(){
	
	play_note(C3);
	performantdelay(40);
	play_note(A2);
	performantdelay(15);
	play_note(B2);
	performantdelay(40);
	play_note(G2);
	performantdelay(15);
	play_note(A2);
	performantdelay(12);
	play_note(A2);
	performantdelay(11);
	play_note(G2);
	performantdelay(11);
	play_note(A2);
	
}*/

void performantdelay(UINT8 numloops){
    UINT8 i;
    for(i = 0; i < numloops; i++){
        wait_vbl_done();
    }     
}

UBYTE canplayermove(UINT8 newplayerx, UINT8 newplayery){
	UINT16 indexTLx, indexTLy, tileindexTL;
	UBYTE result;
	
	indexTLx = (newplayerx - 8) /8;
	indexTLy = (newplayery -16) /8;
	tileindexTL = 20 * indexTLy + indexTLx;
	
	/*if(debug){
		printf("%u %u\n", (UINT16)(newplayerx), (UINT16)(newplayery));
		printf("%u %u %u\n", (UINT16)(indexTLx), (UINT16)(indexTLy), (UINT16)(tileindexTL));
	}*/
	
	result = backgroundmap[tileindexTL] == blankmap[0];
	
	if(tileindexTL==321){
		//collect the key
		set_bkg_tiles(1, 16, 1, 1, blankmap);
		haskey = 1;
		result = 1;
		
		// chanel 1 register 0, Frequency sweep settings
        // 7	Unused
        // 6-4	Sweep time(update rate) (if 0, sweeping is off)
        // 3	Sweep Direction (1: decrease, 0: increase)
        // 2-0	Sweep RtShift amount (if 0, sweeping is off)
        // 0010 0101
		NR10_REG = 0x25;
		
		// chanel 1 register 1: Wave pattern duty and sound length
        // Channels 1 2 and 4
        // 7-6	Wave pattern duty cycle 0-3 (12.5%, 25%, 50%, 75%), duty cycle is how long a quadrangular  wave is "on" vs "of" so 50% (2) is both equal.
        // 5-0 sound length (higher the number shorter the sound)
        // 1000 0001 
		NR11_REG = 0x81;
		
		// chanel 1 register 2: Volume Envelope (Makes the volume get louder or quieter each "tick")
        // On Channels 1 2 and 4
        // 7-4	(Initial) Channel Volume
        // 3	Volume sweep direction (0: down; 1: up)
        // 2-0	Length of each step in sweep (if 0, sweeping is off)
        // NOTE: each step is n/64 seconds long, where n is 1-7	
        // 1011 0 011 
		NR12_REG = 0xB3;
		
		// chanel 1 register 3: Frequency LSbs (Least Significant bits) and noise options
        // for Channels 1 2 and 3
        // 7-0	8 Least Significant bits of frequency (3 Most Significant Bits are set in register 4)
		//0111 0011
		NR13_REG = 0x73;
		
		// chanel 1 register 4: Playback and frequency MSbs
            // Channels 1 2 3 and 4
            // 7	Initialize (trigger channel start, AKA channel INIT) (Write only)
            // 6	Consecutive select/length counter enable (Read/Write). When "0", regardless of the length of data on the NR11 register, sound can be produced consecutively.  When "1", sound is generated during the time period set by the length data contained in register NR11.  After the sound is ouput, the Sound 1 ON flag, at bit 0 of register NR52 is reset.
            // 5-3	Unused
            // 2-0	3 Most Significant bits of frequency
            // 1100 0011 is 0xC3, initialize, no consecutive, frequency = MSB + LSB = 011 0000 0000 = 0x300
		NR14_REG = 0xC6;
		
	}
	
	if(tileindexTL==263 && haskey){
		//open the door
		result = 1;
	}
	
	if(tileindexTL == 339){
		printf("\n \n \n \n \n  === YOU WIN ===");
		win = 1;
	}
	
	return result;
}

void animatesprite(UINT8 spriteindex, INT8 movex, INT8 movey){
	while(movex!=0){
		scroll_sprite(spriteindex, movex < 0 ? -1 : 1, 0); 	//small if : condition ? return what if it's TRUE : or FALSE
		movex += movex < 0 ? 1 : -1;
		wait_vbl_done();
	}
	while(movey!=0){
		scroll_sprite(spriteindex, 0, movey < 0 ? -1 : 1);
		movey += movey < 0 ? 1 : -1;
		wait_vbl_done();
	}
	
	
	
	
}

void noize(){
	
	//bit 5-0 : Sound length
	//0001 1111 is 0x1F the maximum length
	NR41_REG = 0x1F;
	
	// volume envelope
    // bit 7-4 - Initial Volume of envelope (0-0Fh) (0=No Sound)
    // bit 3 - Envelope Direction (0=Decrease, 1=Increase)
    // bit 2-0 - Number of envelope sweep (n: 0-7) (If zero, stop envelope operation.)
    // 0101 0 001
	NR42_REG = 0x51;
	
	// bit 7-4 - Shift Clock Frequency (s)
    // bit 3   - Counter Step/Width (0=15 bits, 1=7 bits)
    // bit 2-0 - Dividing Ratio of Frequencies (r)
    // The amplitude is randomly switched between high and low at the given frequency. 
    // A higher frequency will make the noise to appear 'softer'. 
    // When Bit 3 is set, the output will become more regular, and some frequencies will sound more like Tone than Noise.
    // 0001 0 000
	NR43_REG = 0x10;
	
	// bit 7   - Initial (1=Restart Sound)
    // bit 6   - Controls if last forever or stops when NR41 ends
    // (1=Stop output when length in NR41 expires)
    // bit 5-0	Unused
    // 1100 0000
	NR44_REG = 0xC0;
}

void wall(){
	//0000 0000
	NR41_REG = 0x1F;
	NR42_REG = 0xA7;
	NR43_REG = 0x74;
	NR44_REG = 0xC0;
}

void main(){
	NR52_REG = 0x80;
	NR50_REG = 0x77;
	NR51_REG = 0xFF;
	
	//defines the map, with his tiles
	set_bkg_data(0, 4, backgroundtiles);
	set_bkg_tiles(0, 0, 20, 18, backgroundmap);
	
	set_sprite_data(0, 1, snail); //defines the sprite data
	set_sprite_tile(0, 0);	//defines the tile of the snail, in 0
	
	playerlocation[0] = 16;
	playerlocation[1] = 24;
	
	move_sprite(0, playerlocation[0], playerlocation[1]);
	
	SHOW_SPRITES;
	SHOW_BKG;
	DISPLAY_ON;
	
	
	
	while(!win){
		// DEBUG FEATURES
		/*if(joypad() & J_A){
			debug = 1;
			music();
		}*/

		if(joypad() & J_LEFT){
			/*NR41_REG = 0x1F;
			NR42_REG = 0xF1;
			NR43_REG = 0x30;
			NR44_REG = 0xC0;*/
			if(canplayermove(playerlocation[0] - 8, playerlocation[1])){ //verify if the next tile is empty or not
				playerlocation[0] -= 8;
				noize();
				animatesprite(0, -8, 0);
			}
			else{
				wall();
			}
		}
		else if(joypad() & J_RIGHT){
			if(canplayermove(playerlocation[0] + 8, playerlocation[1])){
				playerlocation[0] += 8;
				noize();
				animatesprite(0, 8, 0);
			}
			else{
				wall();
			}
			
		}
		else if(joypad() & J_UP){
			if(canplayermove(playerlocation[0], playerlocation[1] - 8)){
				playerlocation[1] -= 8;
				noize();
				animatesprite(0, 0, -8);	
			}
			else{
				wall();
			}
		}
		else if(joypad() & J_DOWN){
			if(canplayermove(playerlocation[0], playerlocation[1] + 8)){
				playerlocation[1] += 8;
				noize();
				animatesprite(0, 0, 8);
			}
			else{
				wall();
			}
		}
		performantdelay(6);
	}
	
	
}