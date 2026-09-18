#define PIN_BUZZER 2
// Define note frequencies in Hz (High Octave 6 & 7)
// Note frequencies (Hz)
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_D6  1175
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_G6  1568

// The melody array
int melody[] = {
  NOTE_E6, NOTE_E6, 0, NOTE_E6, 0, NOTE_C6, NOTE_E6, 0, 
  NOTE_G6, 0, 0,  0, NOTE_G5, 0, 0, 0,
  NOTE_C6, 0, 0, NOTE_G5, 0, 0, NOTE_E5, 0,
  0, NOTE_A5, 0, NOTE_B5, 0, NOTE_A5, NOTE_G5, 0,
  
  // Underworld segment
  NOTE_C5, NOTE_C6, NOTE_A5, NOTE_A5, NOTE_A5, 0,
  NOTE_D5, NOTE_D6, NOTE_B5, NOTE_B5, NOTE_B5, 0
};

// Note durations (matching the indices above: 4 = quarter note, 8 = eighth note, etc.)
int noteDurations[] = {
  8, 8, 8, 8, 8, 8, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4,
  
  6, 6, 6, 6, 6, 4,
  6, 6, 6, 6, 6, 4
};

void setup() {
  pinMode(PIN_BUZZER, OUTPUT);
}

void loop() {
  int totalNotes = sizeof(melody) / sizeof(int);

  for (int thisNote = 0; thisNote < totalNotes; thisNote++) {
    // Calculate note duration (e.g., 1000ms / 4 = 250ms)
    int noteDuration = 1000 / noteDurations[thisNote];
    
    if (melody[thisNote] == 0) {
      // If the note is 0, treat it as a musical rest
      noTone(PIN_BUZZER);
    } else {
      tone(PIN_BUZZER, melody[thisNote], noteDuration);
    }

    // A brief pause between notes to make them distinct
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    
    noTone(PIN_BUZZER);
  }

  // 5-second silence before repeating the song
  delay(5000); 
}