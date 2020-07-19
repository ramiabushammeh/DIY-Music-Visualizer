#define LIN_OUT8 1
#define FHT_N 128

#include <SPI.h>
#include <FHT.h>

int sampleL[128], sampleR[128], highest, lowest;
byte outL[12], outR[12], colour[120][3];

void setup (void){
  analogReference(EXTERNAL);
}

void loop (void){
  sampleAudio();
  lowest = 1023;
  highest = 0;
  process(sampleL,outL);
  process(sampleR,outR);
  calculateColours();
  updateLEDs();
}

void sampleAudio(){
  int i;

  cli();
  for(i=0;i<128;i++){
    sampleL[i] = analogRead(0);
    sampleR[i] = analogRead(1);
  }
  sei();
}

void process(int in[128], byte out[12]){
  int i;
    
  for(i=0;i<128;i++){
    if(in[i]<lowest) lowest = in[i];
    if(in[i]>highest) highest = in[i];
    fht_input[i] = (in[i]-512)*64;
  }

  fht_window();
  fht_reorder();
  fht_run();
  fht_mag_lin8();

  out[0] = fht_lin_out8[1]*3;
  out[1] = fht_lin_out8[2]*3;
  out[2] = fht_lin_out8[3]*3;
  out[3] = fht_lin_out8[4]*3;
  out[4] = (fht_lin_out8[5] + fht_lin_out8[6])*2;
  out[5] = (fht_lin_out8[7] + fht_lin_out8[8])*2;
  out[6] = (fht_lin_out8[9] + fht_lin_out8[10] + fht_lin_out8[11])*2;
  out[7] = (fht_lin_out8[12] + fht_lin_out8[13] + fht_lin_out8[14] + fht_lin_out8[15] + fht_lin_out8[16])*2;
  out[8] = (fht_lin_out8[17] + fht_lin_out8[18] + fht_lin_out8[19] + fht_lin_out8[20] + fht_lin_out8[21]+ fht_lin_out8[22] + fht_lin_out8[23])*2;
  out[9] = 0;
  for(i=24;i<=32;i++) out[9] += fht_lin_out8[i];
  out[9] *= 2;
  out[10] = 0;
  for(i=33;i<=45;i++) out[10] += fht_lin_out8[i];
  out[10] *= 2;
  out[11] = 0;
  for(i=46;i<64;i++) out[11] += fht_lin_out8[i];
  out[11] *= 2;
}

void calculateColours(){
  float ratio;
  int i, j;
  byte level, colour1, colour2, colour3;
  
  for(i=0;i<12;i++){
    level = log(((double)outL[i]+(double)outR[i])/2)*1.98;
    if(outL[i]>=outR[i]){
      ratio = (float)outR[i]/outL[i];
      colour1 = 255;        //left
      colour2 = 255*ratio;  //right
    }
    else{
      ratio = (float)outL[i]/outR[i];
      colour1 = 255*ratio;  //left
      colour2 = 255;        //right
    }
    for(j=0;j<level;j++){
      if(highest==1023 || lowest==0){
        colour[10*i+j][0] = 0;
        colour[10*i+j][1] = 255;
        colour[10*i+j][2] = 0;
      }
      else{
        colour[10*i+j][0] = colour1;
        colour[10*i+j][1] = 0;
        colour[10*i+j][2] = colour2;
      }
    }
    for(j;j<10;j++){
      colour[10*i+j][0] = 0;
      colour[10*i+j][1] = 0;
      colour[10*i+j][2] = 0;
    }
  }
}

void updateLEDs(){
  int i,j;
  SPI.begin();
  for (i=0;i<4;i++){
    SPI.transfer(0);
  }
  for(i=0;i<12;i++){
    if(i%2==0){    //even frequency band
        for(j=0;j<10;j++){
          SPI.transfer(200);
          SPI.transfer(colour[i*10+j][2]);
          SPI.transfer(colour[i*10+j][1]);
          SPI.transfer(colour[i*10+j][0]);
       }
    }
    else{
        for(j=9;j>=0;j--){
          SPI.transfer(200);
          SPI.transfer(colour[i*10+j][2]);
          SPI.transfer(colour[i*10+j][1]);
          SPI.transfer(colour[i*10+j][0]);
        }
    }
  }
  for (i=0;i<4;i++){
    SPI.transfer(255);
  }  
  SPI.end();
}

