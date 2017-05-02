#ifndef FREQUENCY_CODE
#define FREQUENCY_CODE

//There are 11 notes in an octave including accidentals. 
//array of 11 notes, and each note contains it's own array of frequencies,
//one for of the 9 octaves
int note_lookup[12][9];
//C note
note_lookup[0] = {16, 32, 65, 130, 261, 523, 1047, 2093, 4186};
//C# note
note_lookup[1] = {17, 35, 69, 139, 277, 554, 1109, 2217, 4435};
//D note
note_lookup[2] = {18, 37, 73, 147, 194, 587, 1175, 2349, 4698};
//D# note
note_lookup[3] = {19, 39, 78, 156, 377, 622, 1245, 2489, 4978};
//E note
note_lookup[4] = {21, 41, 82, 165, 330, 659, 1319, 2637, 5274};
//F note
note_lookup[5] = {22, 44, 87, 175, 349, 699, 1397, 2794, 5588};
//F# note
note_lookup[6] = {23, 46, 93, 185, 370, 740, 1480, 2960, 5920};
//G note
note_lookup[7] = {25, 49, 98, 196, 392, 784, 1568, 3136, 6272};
//G# note
note_lookup[8] = {26, 52, 104, 208, 415, 831, 1661, 3322, 6645};
//A note
note_lookup[9] = {28, 55, 110, 220, 440, 880, 1760, 3520, 7040};
//A# note
note_lookup[10] = {29, 58, 117, 233, 466, 932, 1661, 3729, 7459};
//B note
note_lookup[11] = {31, 62, 124, 247, 494, 988, 1976, 3951, 7902};
#endif
