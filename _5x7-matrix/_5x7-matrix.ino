int idx = 0; unsigned long last;

void setup() {
  last = millis();  
  pinMode( 9, OUTPUT );
  pinMode( 10, OUTPUT );
  pinMode( 11, OUTPUT );
  pinMode( 12, OUTPUT );
  pinMode( 13, OUTPUT );
  pinMode( 2, OUTPUT );
  
  pinMode( 3, OUTPUT );
  pinMode( 4, OUTPUT );
  pinMode( 5, OUTPUT );
  pinMode( 6, OUTPUT );
  pinMode( 7, OUTPUT );
  pinMode( 8, OUTPUT );

   for( int r = 0; r < 7; r++) {
     digitalWrite( r + 2, LOW );
   }
   for( int c = 0; c < 5; c++) {
     digitalWrite( c + 9, HIGH);
   }
}

byte leds[7][5]; 

void setPattern( byte pattern[20][5], int idx ) {
  for( int r =0; r < 7; r++) {
    for( int c = 0; c < 5; c++) {
      leds[r][c] = pattern[r + idx][c];
     }
   } 
}

void draw() {   

  for( int r =0; r < 7; r ++ ) {
    digitalWrite( r + 2, HIGH );
    for( int c=0; c < 5; c ++ ) {
      digitalWrite( 13 - c, ( leds[r][c] == 1 ? LOW : HIGH ));
    }
    delayMicroseconds(900);
    digitalWrite( r + 2, LOW );
  }
}

void loop() {
  if ( millis() - last > 600 ) {
    idx = (idx == 0 ?  7 : 0);
    last = millis();
  }

  byte tmp[14][5] = {
  { 1,1,1,1,1},
  { 0,0,0,0,0},
  { 0,0,0,0,0},
  { 0,0,0,0,0},
  { 0,0,0,0,0},
  { 0,0,0,0,0},
  { 0,0,0,0,0},

  { 0,0,0,0,0},
  { 0,0,0,0,0},
  { 0,0,0,0,0},
  { 0,0,0,0,0},
  { 0,0,0,0,0},
  { 0,0,0,0,0},
  { 0,0,0,0,0},
  };

  setPattern( tmp, idx );
  draw();
}
