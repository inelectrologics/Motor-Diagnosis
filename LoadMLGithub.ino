//
// Copyright by Pat Taweewat, This code is a part of MCU-based Motor Diagnosis Platform and experimentation for Paper II, iEECON2025.
// This is reduced version for testing on TFlite micro model reloading without recompiling via USB virtual serial communication.
// To be used with main code, put the TFlite micro model into the main loop of the main code.
// Some parts of the code is used for testing compatability for the EloquentML library, if you not need to recheck that, you can remove it without error.
//
const uint16_t adcbufsize=4096;
uint16_t adcbuf[adcbufsize],xidx[91];
#include "arduinoFFT.h"
const uint16_t samples=4096;
const double sampling=5000;
double vReal[samples];
double vImag[samples];
ArduinoFFT<double> FFT=ArduinoFFT<double>(vReal,vImag,samples,sampling);
#include <EloquentTinyML.h>
#include <eloquent_tinyml/tensorflow.h>
#define N_INPUTS 91
#define N_OUTPUTS 3
#define TENSOR_ARENA_SIZE 2*1024

uint8_t *loadedModel;
Eloquent::TinyML::TensorFlow::TensorFlow<N_INPUTS, N_OUTPUTS, TENSOR_ARENA_SIZE> ml;

void loadModel(void);

void setup() {
    Serial.begin(115200);
    delay(3000);
}
const uint16_t dsize=8192;
uint16_t tflmSz,idx,aval;
uint8_t dval;
byte inBytes[2],bBytes[8],aBytes[2];
uint8_t d[dsize];
double b[4096],xmin[2045],xrange[2045],bval;
float xbuf[91];
float yprd[3];
void loop() {
  if(Serial.available()>0){
    byte c=Serial.read();
    if(c=='d'){//put model buffer
    Serial.readBytes(inBytes,2);
    memcpy(&tflmSz,inBytes,2);
    idx=0;
    while(idx<tflmSz){
     if(Serial.available()>=1){
      dval=Serial.read();
      d[idx]=dval;
      idx++;
    }
   }
  }else if(c=='e'){//show model data
  Serial.println("Data received:");
  for (int i = 0; i < tflmSz; i++) {
    Serial.println(d[i]);
  }Serial.println(idx);
 }else if(c=='l'){//load model
  loadModel();
  if (!ml.begin(loadedModel)) {
     Serial.println("Cannot inialize model");
     Serial.println(ml.getErrorMessage());
     delay(60000);
   }
 }else if(c=='a'){//put adcbuf
   idx=0;
   while(idx<4096){
    if(Serial.available()>=2){
     Serial.readBytes(aBytes,2);
     memcpy(&aval,aBytes,2);
     adcbuf[idx]=aval;
     idx++;
    }
   }
  }else if(c=='x'){//put xbuf
   idx=0;
   while(idx<91){
    if(Serial.available()>=8){
     Serial.readBytes(bBytes,8);
     memcpy(&bval,bBytes,8);
     b[idx]=bval;
     idx++;
    }
   }
  }else if(c=='b'){//adcbuf show
    for(uint16_t i;i<4096;i++){
     Serial.println(adcbuf[i]);
    }
  }else if(c=='v'){//load xbuf and show
   for(uint16_t i;i<91;i++){
    xbuf[i]=float(b[i]);Serial.println(b[i]);
   }
  }else if(c=='t'){//put xbuf and load xbuf used for testing in one step
   idx=0;
   while(idx<91){
    if(Serial.available()>=8){
     Serial.readBytes(bBytes,8);
     memcpy(&bval,bBytes,8);
     b[idx]=bval;
     idx++;
    }
   }
   for(uint16_t i;i<91;i++){
    xbuf[i]=float(b[i]);
   }
  }else if(c=='o'){// predict and show used for downloading output
    float predicted = ml.predict(xbuf,yprd);
    Serial.println(yprd[0]);
    Serial.println(yprd[1]);
    Serial.println(yprd[2]);
  }else if(c=='f'){// load adcbuf, compute FFt and show FFT
    for(int i=0;i<4096;i++){
      vReal[i]=adcbuf[i];
      vImag[i]=0;
    }
    FFT.windowing(FFTWindow::Hann, FFTDirection::Forward);//ADC, compute FFT and sending
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();
    for(uint16_t i=0;i<2048;i++){
     bval=vReal[i];
     Serial.write(reinterpret_cast<uint8_t*>(&bval),8);
    }
  }else if(c=='g'){//show compute FFT
    for(uint16_t i;i<2048;i++){
     Serial.println(vReal[i]);
    }
  }else if(c=='h'){// put xmin and put xrange
   idx=0;
   while(idx<4090){
    if(Serial.available()>=8){
     Serial.readBytes(bBytes,8);
     memcpy(&bval,bBytes,8);
     b[idx]=bval;
     idx++;
    }
   } 
   for(uint16_t i=0;i<2045;i++){
     xmin[i]=b[i];
     xrange[i]=b[i+2045];// xmin and xrange are loaded in the same buffer first 2045 and last 2045
   } 
  }else if(c=='i'){// compute input feature vector and show
    for(int i=0;i<4096;i++){//load adc->fft
      vReal[i]=adcbuf[i];
      vImag[i]=0;
    }
    FFT.windowing(FFTWindow::Hann, FFTDirection::Forward);//ADC, compute FFT and sending
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();
    for(uint16_t i=0;i<2045;i++){//reindex start from bin 3rd
      b[i]=double(vReal[i+3]);
    }
    for(uint16_t i=0;i<2045;i++){
      b[i]=(b[i]-xmin[i])/xrange[i];
    }
    for(uint16_t i=0;i<91;i++){
       xbuf[i]=float(b[xidx[i]]);       
    }
  }else if(c=='j'){// put xidx to reindex only fft bin used
   idx=0;
   while(idx<91){
    if(Serial.available()>=2){
     Serial.readBytes(aBytes,2);
     memcpy(&aval,aBytes,2);
     xidx[idx]=aval;
     idx++;
    }
   }
  }else if(c=='k'){//show input vector index and value
    for(uint16_t i;i<91;i++){
     Serial.print(xidx[i]);
     Serial.print(":");
     Serial.println(xbuf[i]);
    }
  }else if(c=='p'){// This part is used to check if it also work with other ml model provided by the EloquentML Sine example. and I found that also works well.
// If you not need to recheck of past compatability, then remove this part of code.
    long t0=micros();
    float x = 3.14 * random(100) / 100;
    float y = sin(x);
    float input[3] = { x,x,x };
    float predicted = ml.predict(xbuf,yprd);
    Serial.print("sin(");
    Serial.print(x);
    Serial.print(") = ");
    Serial.print(y);
    Serial.print("\t predicted: ");
    Serial.print(predicted);Serial.print(":");
    Serial.print(ml.predictClass(xbuf));Serial.print(":");
    Serial.print(ml.probaToClass(xbuf));
    Serial.print("<");
    Serial.print(yprd[0]);
    Serial.print(":");
    Serial.print(yprd[1]);
    Serial.print(":");
    Serial.print(yprd[2]);
    Serial.println(">");
    Serial.println((micros()-t0)/1000000);
    delay(1000);
// If you not need to recheck of past compatability, then remove this part of code.
 }
 }
}


/**
 * This is part where we can reloading model.
 * As this part I have modified from the EloquentML example, I need to ensure that it is work by memory buffer directly and 
 * found that it works well in the test.
 */
void loadModel() {
    size_t modelSize=tflmSz;
    Serial.print("Found model of size ");
    Serial.println(modelSize);

    loadedModel = (uint8_t*) malloc(modelSize);

    for (size_t i = 0; i < modelSize; i++)
      loadedModel[i]=d[i];
    Serial.println(ESP.getFreeHeap());
    Serial.println(ESP.getHeapSize());
}
