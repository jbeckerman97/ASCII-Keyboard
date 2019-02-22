// Final project
// Written by Jacob Beckerman
// CSI-370-02
// Display a keyboard with buttons pushed down based on input received from code.asm
// Due 4/30/18
// All references used are included in documentation

#include <algorithm> // Used for replace() (seen in display keyboard)
#include <inttypes.h> // Used for uint16_t
#include <iostream>
#include <map>
#include <string>

using namespace std;

extern "C" void asmMain();
           void centerKeyboard();
           void clearScreen();
extern "C" void displayKeyboard(_Uint32t flag, _Uint32t key);
           void exitInstruction();

// Key is virtual key code (stored in dx), value is string representing key on keyboard
map<_Uint32t, string> keyboardKeys = {
   // Irvine won't recognize solo key press for PrtSc, map key used was found on MSDN
   {0x001B, "| ESC |"}, {0x0070, " F1 |"}, {0x0071, " F2 |"}, {0x0072, " F3 |"}, {0x0073, "  F4  |"}, {0x0074, "  F5  |"},
   {0x0075, "  F6  |"}, {0x0076, "  F7  |"}, {0x0077, "  F8  |"}, {0x0078, "  F9  |"}, {0x0079, "  F10  |"}, {0x007A, "  F11  |"},
   {0x007B, "  F12  |"}, {0x002C, "  PRTSC  |"}, {0x002D, "  INSERT  |"}, {0x002E, "  DELETE   |\n"}, // Newline for end of row

   {0x00C0, "|   `~   |"}, {0x0031, "   1!   |"}, {0x0032, "   2@   |"}, {0x0033, "   3#   |"}, {0x0034, "   4$   |"},
   {0x0035, "   5%   |"}, {0x0036, "   6^   |"}, {0x0037, "  7&  |"}, {0x0038, "  8*  |"}, {0x0039, "  9{  |"}, {0x0030, "  0}  |"},
   {0x00BD, "  -_  |"}, {0x00BB, "  =+  |"}, {0x0008, "  BACKSPACE   |\n"}, // Newline for end of row

   {0x0009, "|  TAB   |"}, {0x0051, "   Qq   |"}, {0x0057, "   Ww   |"}, {0x0045, "   Ee   |"}, {0x0052, "   Rr   |"},
   {0x0054, "   Tt   |"}, {0x0059, "   Yy   |"}, {0x0055, "   Uu   |"}, {0x0049, "   Ii   |"}, {0x004F, "   Oo   |"},
   {0x0050, "   Pp  |"}, {0x00DB, "  {[  |"}, {0x00DD, "  }]  |"}, {0x00DC, "  |\\   |\n"}, // Newline for end of row

   // Irvine won't recognize solo key press for CapsLk, map key used found on MSDN
   {0x0014, "|  CAPSLK  |"}, {0x0041, "   Aa   |"}, {0x0053, "   Ss  |"}, {0x0044, "  Dd  |"}, {0x0046, "  Ff  |"}, {0x0047, "  Gg  |"},
   {0x0048, "  Hh  |"}, {0x004A, "  Jj  |"}, {0x004B, "  Kk  |"}, {0x004C, "  Ll  |"}, {0x00BA, "  :;  |"}, {0x00DE, "  \"'  |"},
   {0x000D, "           ENTER            |\n"}, // Newline for end of row

   // Irvine won't recognize solo key press for either Shift, map keys used found on MSDN
   {0x00A0, "|     SHIFT     |"}, {0x005A, "   Zz  |"}, {0x0058, "   Xx  |"}, {0x0043, "  Cc  |"}, {0x0056, "  Vv  |"}, {0x0042, "  Bb  |"},
   {0x004E, "  Nn  |"}, {0x004D, "  Mm  |"}, {0x00BC, "  <,  |"}, {0x00BE, "  >.  |"}, {0x00BF, "  ?/  |"}, {0x00A1, "   SHIFT    |"},
   {0x0026, "        UP        |\n"}, // Newline for end of row

   // Irvine won't recognize solo key press for either Ctrl, Fn, either Alt, map keys used found on MSDN
   // FN and Right-Alt didn't have values on MSDN so I assigned them unused key codes
   {0x00A2, "|  CTRL  |"}, {0xFFFF, "  FN  |"}, {0x005B, "  WIN  |"}, {0x0012, "  ALT  |"}, {0x0020, "              SPACE               |"},
   {0x005D, "  OPT  |"}, {0xFFFE, "  ALT  |"}, {0x00A3, "  CTRL  |"}, {0x0025, "  LEFT  |"}, {0x0028, "  DOWN  |"}, {0x0027, "  RIGHT  |\n"}
};

/* Need separate map for these because some of the EBX flag codes are the same as virtual key codes This map will be used in displayKeyboard
   to determine if the flag is being pressed at the same time as one of the keys. Flag code is stored in EBX through Irvine. Sometimes
   the virtual code changes, and I can't figure out why. The code for any flag is one of two options, so I handle both.
   0x00000010 or 0x00000030 is Shift, 0x00000080 or 0x000000A0 is Caps Lock, 0x00000002 or 0x00000022 is Left-Alt, 
   0x00000001 or 0x00000021 is Right-Alt, 0x00000008 or 0x00000028 is Left-Ctrl, 0x00000004 or 0x00000024 is Right-Ctrl */
map<_Uint32t, _Uint32t> virtualFlagCodes = {
   {0x00000010, 0x00A0}, {0x00000030, 0x00A0},
   {0x00000080, 0x0014}, {0x000000A0, 0x0014},
   {0x00000002, 0x0012}, {0x00000022, 0x0012},
   {0x00000001, 0x0012}, {0x00000021, 0x0012},
   {0x00000008, 0x00A2}, {0x00000028, 0x00A2},
   {0x00000004, 0x00A3}, {0x00000024, 0x00A3}
};

// Need this array because using an iterator for displaying the map will print out by ascending order of key code, which is not desired
// Key is virtual key code (stored in dx through Irvine)
_Uint32t virtualKeyCodes[] = {
   0x001B, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x002C, 0x002D, 0x002E,
   0x00C0, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x0030, 0x00BD, 0x00BB, 0x0008,
   0x0009, 0x0051, 0x0057, 0x0045, 0x0052, 0x0054, 0x0059, 0x0055, 0x0049, 0x004F, 0x0050, 0x00DB, 0x00DD, 0x00DC,
   0x0014, 0x0041, 0x0053, 0x0044, 0x0046, 0x0047, 0x0048, 0x004A, 0x004B, 0x004C, 0x00BA, 0x00DE, 0x000D,
   0x00A0, 0x005A, 0x0058, 0x0043, 0x0056, 0x0042, 0x004E, 0x004D, 0x00BC, 0x00BE, 0x00BF, 0x00A1, 0x0026,
   0x00A2, 0xFFFF, 0x005B, 0x0012, 0x0020, 0x005D, 0xFFFE, 0x00A3, 0x0025, 0x0028, 0x0027
};

/* Seemed like it would be more efficient to declare the following four variables globally once rather than create new
   variables every time displayKeyboard is called, which is frequently. I also chose to store key strings for pressed
   keys in separate variables. I thought it would be more efficient to do that than to modify the string stored in
   keyboardKeys twice (once to insert asterisks for spaces, once to insert spaces for asterisks) and having to find
   the key value for the appropriate string for the latter string replacement. */
/* The data type here is uint16_t because I found that when pushing dx, it accumulated other values in the first 16 bits
   that made a comparison between the key codes I define and the value being received into key unequal. By making storeKey
   uint16_t and setting it equal to key, it only stores the last 16 bits. storeKey16 is then converted back to a _Uint32t
   type because the codes in virtualKeyCodes are compared as 32-bit data types, and checking for equality between the 16-bit
   and 32-bit versions of the same number returns false. */
uint16_t storeKey16;
_Uint32t storeKey32;
string tempPressKey, tempPressFlag;

int main() {
   asmMain();
}

void centerKeyboard() {
   cout << "\n\n\n\n\n\n\n\n\n";
}

void clearScreen() {
   cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
}

void displayKeyboard(_Uint32t flag, _Uint32t key) {
   clearScreen();
   exitInstruction();
   // See note by global variable declarations for explanation
   storeKey16 = key;
   storeKey32 = storeKey16;

   // See note by global variable declarations for explanation
   tempPressKey = tempPressFlag = "";
   
   for (int i = 0; i < (sizeof(virtualKeyCodes) / sizeof(virtualKeyCodes[0])); i++) {
      if (virtualKeyCodes[i] == storeKey32) { // Checking if the key pressed is the current index in virtualKeyCodes
         tempPressKey = keyboardKeys[virtualKeyCodes[i]];
         replace(tempPressKey.begin(), tempPressKey.end(), ' ', '*');
         cout << tempPressKey;
      }

      if (virtualFlagCodes[flag] == virtualKeyCodes[i]) { // Checking if the flag passed in is the current index in virtualKeyCodes
         tempPressFlag = keyboardKeys[virtualKeyCodes[i]];
         replace(tempPressFlag.begin(), tempPressFlag.end(), ' ', '*');
         cout << tempPressFlag;
      }

      // Unnecessary but included to be explicit
      if (virtualKeyCodes[i] != storeKey32 && virtualFlagCodes[flag] != virtualKeyCodes[i]) // Print standard key view
         cout << keyboardKeys[virtualKeyCodes[i]];
   }

   centerKeyboard();
}

void exitInstruction() {
   cout << "Press escape to exit" << endl;
}