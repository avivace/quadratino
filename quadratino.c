/*        quadratino
    a (shitty) Snake clone for Game Boy
    Antonio Vivace 2015

    [TODO]

    - Fix [!!] errors /stupid workarounds
    - Add Pause
    - Add Save System (Last Settings, HI-SCORE)

*/

#include <gb/gb.h> // GBDK function library
#include <gb/hardware.h> // Handy hardware references
#include <rand.h> // Random functions
#include "res.c" // Sprite and background data

// Function declaration
void drawFrame();
void fadeOutBkg(UINT8);
void fadeInBkg(UINT8);
void spawnHead();
void spawnFood();
void spawnBonus();
void killBonus();
void checkMargins();
void printScore(UINT16);
void clearNumbers();
void moveHead();
void VBLwait(UINT8);
void updateInputBuffer();
void gameOver();
void stopSFX();
void playSFX(UINT8);
void updateDiffBar();
void clearBKG();
UINT16 mapCoordinate(UINT8, UINT8);
UINT8 randomize(UINT8,UINT8);

#define movrate 8
#define bonusrate 3
#define timeToLive 8*8
#define sfoodValue 1
#define sbonusValue 5
#define sbaseValue 1

// Variables
UBYTE start,up,levelbar_x,levelbar_y,k,n,justStarted,soundStart,s,bonus_score,food_score,life,justBonused,bonus_x,bonus_y,foodRedraw,counter,head_x,head_y,food_x,food_y,last,lastb,respawnFood,food_xr,food_yr,valid, initialFoodSpawn, initialHeadSpawn,buffer[20],tail[100][3],i,p,sp;
UINT16 digit[3],x,y,score,pos,j,frame,estimate,difficulty,framej,snake[150],bar[8];
UINT8 modeselect;
UWORD seed;

// Do things
int main(){
  difficulty=3;
  modeselect=0;

  start:    // point of start after losing (or first run)
  start = 1;
  up=1;
  clearBKG();
  k=1;
  mode=1;
  last=3;
  lastb=3;
  justStarted=1;
  levelbar_x= 27;
  levelbar_y= 95;

  // start screen res
  disable_interrupts();
  DISPLAY_OFF;
  SPRITES_8x8;
  set_sprite_data(0, 21, game_sprites);
  for (i=0;i<3;i++) set_sprite_tile(i,i);
  set_bkg_data(0,168,splash_tile);
  set_bkg_tiles(0,0,32,32,splash_map);
  
  // set up Sound
  NR52_REG = 0x8F;  // TURN SOUND ON
  NR51_REG = 0xff;  // play sfx on Channel 1
  NR50_REG = 0x20;  // Volume (MAX = 0x77, min = 0x00)

  DISPLAY_ON;
  SHOW_SPRITES;
  enable_interrupts();

  // START SCREEN

  fadeInBkg(255);

  move_sprite(11,levelbar_x+8*0,levelbar_y);
  move_sprite(12,levelbar_x+8*1,levelbar_y);
  move_sprite(13,levelbar_x+8*2,levelbar_y);
  move_sprite(14,levelbar_x+8*3,levelbar_y);
  move_sprite(15,levelbar_x+8*4,levelbar_y);
  move_sprite(16,levelbar_x+8*5,levelbar_y);  
  move_sprite(17,levelbar_x+8*6,levelbar_y);
  move_sprite(18,120,levelbar_y);

  while(start){
    wait_vbl_done();
    wait_vbl_done();
    wait_vbl_done();
    stopSFX();
    counter = joypad();   // Update Input
    updateDiffBar();      // Update Diff Bar and Mode Select
    set_sprite_tile(18,4+modeselect);

    if (counter & J_A && (up==0)) {
      playSFX(1);
      difficulty++;
      up=1;
    }

    if (counter & J_B && (up==0)) {
      playSFX(1);
      modeselect++;
      up=1;
    }

    if (counter & J_START && (up==0)) start=0;


    if (counter==0) up=0;
    if (difficulty==8) difficulty=0;
    if (modeselect==2) modeselect=0;
  }
  seed |= (UWORD)DIV_REG << 8;  // Init rand seed
  initrand(seed);
  for (i=0;i<30;i++) move_sprite(i,0,0);
  fadeOutBkg(255);

  // Initial spawns
  spawnHead();
  spawnFood();

  // SET UP game BKG 
  set_bkg_data(0,168,tail_tile_data);
  set_bkg_tiles(0,0,32,32,tail_map_data);
  fadeInBkg(1);

  // Set up Scoreboard
  move_sprite(7,143,16);
  move_sprite(8,151,16);
  move_sprite(9,159,16);
  set_sprite_tile(7,14);
  set_sprite_tile(8,14);
  set_sprite_tile(9,14);

  if (modeselect==1) drawFrame();  // The frame, some manual drawing..
  printScore(score);  //Print 0

  switch(difficulty){
    case 0: {
      framej=14;
      break;
    }
    case 1: {
      framej=12;
      break;
    }    
    case 2: {
      framej=10;
      break;
    }    
    case 3: {
      framej=8;
      break;
    }    
    case 4: {
      framej=6;
      break;
    }    
    case 5: {
      framej=4;
      break;
    }
    case 6: {
      framej=2;
      break;
    }
    case 7: {
      framej=0;
      break;
    }
  }

while(1){
 updateInputBuffer();
 wait_vbl_done();
 if (frame==framej || framej==0) {   //  main gameflow
    frame=1;
    if (head_x%8==0 && head_y%8==0) {     // apply change direction when aligned in grid
      if(((last==1 || last==2) && (lastb==3 || lastb==4)) || ((last==3 || last==4) && (lastb==1 || lastb==2))) {
        last=lastb;
          }
    }

    x=(head_x-8)/8;
    y=(head_y-16)/8;

    if (frame%100==0){		// Shift back array [!!]
      for(i=0;i<food_score+2;i++){
        snake[food_score-i]=snake[k-i];
      }
      k=food_score;
      frame=0;
    }

    snake[k]=y*32+x;
    tail_map_data[snake[k-1-food_score]]= 0x01;    // Clear 

    if (tail_map_data[y*32+x]==0x02 || tail_map_data[y*32+x]==0x00) {    // Collision check (Snake,Snake),(Snake,Frame)
        gameOver();
        goto start;
    }

    if(justStarted!=1) moveHead();

    tail_map_data[snake[k]]= 0x02;  // Draw Head
    k++;

    set_bkg_tiles(0,0,32,32,tail_map_data);

    // Game-Tick updates
    if (life==0) killBonus(); 
    if (life>0) life--;    // We need the bonus to die

    // Collision check
    checkMargins();

    // Spawn Bonus
    if (food_score!=0 && food_score%bonusrate==0 && justBonused==0) spawnBonus();

    // Head-Bonus collisionCheck
    if ((head_x==bonus_x && head_y==bonus_y)) {
      playSFX(2);
      soundStart=frame;
      killBonus();
      food_score++;     // [!]
      bonus_score++;
      score = sbaseValue*(food_score*sfoodValue+bonus_score*sbonusValue); // Update total score [!]
      printScore(score); // Print Score [!]
    }

    // Stop any playing sound
    if ((frame%soundStart)==3) stopSFX(); //[!!]

    // Head-Food collisionCheck
    if ((head_x==food_x && head_y==food_y)){
        playSFX(1);
        justBonused=0;    // Flag to avoid infinite spawns of Bonus
        food_score++;
        spawnFood();
        soundStart=frame;
        score = sbaseValue*(food_score*sfoodValue+bonus_score*sbonusValue); // Update total score [!]
        printScore(score); // Print Score [!]
      }
    }
    frame++;
}
  
}


// FUNCTIONS
void updateDiffBar(){
  for(i=0;i<8;i++){
    bar[i]=0;
  }
  for(i=0;i<difficulty+1;i++){
    bar[i]=3;
  }
  set_sprite_tile(11,15+bar[1]);
  set_sprite_tile(12,16+bar[2]);
  set_sprite_tile(13,16+bar[3]);
  set_sprite_tile(14,16+bar[4]);
  set_sprite_tile(15,16+bar[5]);
  set_sprite_tile(16,16+bar[6]);
  set_sprite_tile(17,17+bar[7]);
}

void clearBKG(){
  for (j=0;j<564; j++) tail_map_data[j]=0x01; // Clear BKG
}

void playSFX(UINT8 id){ // Some noise
  if (id==1){
    NR21_REG = 0x7F;  
    NR22_REG = 0x7F;
    NR23_REG = (32-1)*16;
    NR24_REG = 0x86;
  }
  if (id==2){
    NR21_REG = 0x7F;
    NR22_REG = 0x7F;
    NR23_REG = (30)*2;
    NR24_REG = 0x86;
  }
}

void stopSFX(){
  NR21_REG = 0x00;    //Silence
  NR22_REG = 0x00;
  NR23_REG = 0x00;
  NR24_REG = 0x00;
}

void updateInputBuffer(){
  counter = joypad();
  if(counter & J_UP) lastb = 1;       // buffer joypad input
  if(counter & J_DOWN) lastb = 2;
  if(counter & J_LEFT) lastb = 3;
  if(counter & J_RIGHT) lastb = 4;
  //if(counter & J_START) pause();
  if(counter) justStarted=0;
}

void moveHead(){
  if(last==1) head_y-=movrate;
  if(last==2) head_y+=movrate;
  if(last==3) head_x-=movrate;
  if(last==4) head_x+=movrate;
  //move_sprite(0,head_x,head_y);
  stopSFX();
}

void printScore(UINT16 score){
  /*for(i=0,j=10;i<3;i++) { // Divide in digits, digit[0] is the least significant
    
  } */
  estimate = 1;
  if (score>10) estimate = 2;
  if (score>100) estimate = 3;
  for (i=0,j=10;i<estimate;i++,j*=10){
    digit[i]=(score%j)/(j/10);
    set_sprite_tile(9-i,4+digit[i]);

  }
}

void printScore_old(UINT8 score){
  for(i=0,j=10;i<3;i++,j*10) digit[i]=(score%j)/(j/10);    // Divide in digits, digit[0] is the least significant
  clearNumbers();
  move_sprite(4+digit[0],60,60);
}

void clearNumbers(){ //deprecated
  for(i=0;i<10;i++){
    move_sprite(4+i,0,0);
  }
}

void fadeOutBkg(UINT8 edelay){
  BGP_REG=0xE4;
  delay(edelay);
  SHOW_BKG;
  BGP_REG=0xA4;
  SHOW_BKG;
  delay(edelay);
  BGP_REG=0x54;
  SHOW_BKG;
  delay(edelay);
  BGP_REG=0x00;
  SHOW_BKG;
  }

void fadeInBkg(UINT8 edelay){
  BGP_REG=0x00;
  delay(edelay);
  SHOW_BKG;
  BGP_REG=0x54;
  SHOW_BKG;
  delay(edelay);
  BGP_REG=0xA4;
  SHOW_BKG;
  delay(edelay);
  BGP_REG=0xE4;
  SHOW_BKG;
  }

void spawnHead(){
  valid=0;
  head_x=8*12;
  head_y=8*15;
}

void spawnFood(){
  food_x=8*randomize(2,19);
  food_y=8*randomize(3,18);
  move_sprite(1,food_x,food_y);
}

UINT8 randomize(UINT8 range1,UINT8 range2){     // Generate a valid random n so range1=<n=<range2 
  UINT8 n;                                      // I don't really get why we need to do this
  valid=0;
  while(valid==0){
    n=range1+rand()%range2;
    valid=1;
    if (n < range1 || n > range2) valid=0;
  }
  return n;
}

void drawFrame(){
  for (p=0;p<18; p++) tail_map_data[17*32+1+p]=0x00;
  for (j=0;j<17; j++) tail_map_data[j]=0x00;
  for (j=0;j<18; j++) tail_map_data[j*32]=0x00;
  for (j=1;j<18; j++) tail_map_data[j*32+19]=0x00;
}  

void spawnBonus(){
  valid=0;
  while(valid==0){
    bonus_x=8*(2+rand()%19);
    bonus_y=8*(3+rand()%18);
    if (head_x < 8 || head_x > 8*20 || head_y < 8*2 || head_y > 8*19 || lastb <1 || lastb >4) valid=0;
    valid= 1;
    }
  move_sprite(2,bonus_x,bonus_y);
  justBonused=1;
  life=timeToLive;
}

void killBonus(){ // RIP
  bonus_x=0;
  bonus_y=0;
  move_sprite(2,bonus_x,bonus_y);
}

void gameOver(){
  stopSFX();
  for(i=0;i<10;i++) move_sprite(i,0,0);
  food_score=0;
  bonus_score=0;
  head_x=64;
  head_y=64;
  move_sprite(1,0,0);
  move_sprite(0,0,0);
  set_bkg_data(0,168,game_over_tile_data);
  set_bkg_tiles(0,0,32,32,game_over_map_data);
  move_sprite(7,80,88);
  move_sprite(8,88,88);
  move_sprite(9,96,88);
  waitpad(0x80);
  move_sprite(7,0,0);
  move_sprite(8,0,0);
  move_sprite(9,0,0);
  score=0;
}

void checkMargins(){    
  if (head_y<=8)
    head_y=152;
  else if(head_y>=160) 
    head_y=16;
  if (head_x>=168)
    head_x=0; 
  else if(head_x<=0) 
    head_x=160;
}

UINT16 mapCoordinate(UINT8 x, UINT8 y){
  UINT16 n;
  n = (y*32)+x;
  return n;
}

/*

      0~563
      0,0~17,19
      32 per row
      19 visible per row
      y*32 + x
      */