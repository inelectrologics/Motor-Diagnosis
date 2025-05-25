# Motor-Diagnosis
This is for main arduino code used to implemente diagnosis platform on MCU board.
The code here is tested and implemented on ESP32-C3 Dual USB board for motor signal diagnosis.
Please ensure your compatibility for flash, RAM, WiFi mode, pin numbers, ADC configuration, ADC timing and usb virtual serial port.
I found that the code can be compiled with "pass" and "uploaded successfully" on any other ESP32, ESP32-S2, ESP32-S3, ESP32-C3 and ESP32-C6 with 4MB flash or more
but I have tested on real signal capture and diagnosis on ESP32-C3 Dual USB only.
In the code, the input feature size for trained model in the experiment is less than 192 but xbuf can have input upto 192 vector size.
If your model is finalized and has lower number than the initial xbuf then this number can be reduced to save memory. The class is upto 3 classes which is depended on ybuf size.
If you need more than 3 classes then this buffer should be modified.
Actually, xbuf and ybuf can be larger but I used those numbers of sizes for test past and backward compatability to work with some examples of the EloquentML library which you can check some examples at https://github.com/eloquentarduino/EloquentTinyML
If you not need the compatability test then removing will reduce compiled binary size.
