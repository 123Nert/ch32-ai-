#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
  
// 控制 WS2812 灯条的引脚编号
#define PIN        6
  
//定义控制的 LED 数量
#define NUMPIXELS 4
  
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
  
//相邻 LED 之间的延迟，单位毫秒
#define DELAYVAL 500
  
void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.
  
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
}
 // 在这里编写你的代码，控制LED灯条的亮灭和颜色
void loop() {
  pixels.clear(); // Set all pixel colors to 'off'
  
  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
  for(int i=0; i<NUMPIXELS+1; i++) { // For each pixel...
    switch(i){
    case 0:pixels.setPixelColor(i, pixels.Color(20, 32, 255));break;//蓝
    case 1:pixels.setPixelColor(i, pixels.Color(255, 20,255));break;//紫
    case 2:pixels.setPixelColor(i, pixels.Color(255, 80,0));break;//黄
    case 3:pixels.setPixelColor(i, pixels.Color(51, 0, 0));break;//红
    case 4:for(int x=0;x<4;x++){
            pixels.setPixelColor(0, pixels.Color(20, 32, 255));
            pixels.setPixelColor(1, pixels.Color(255, 20,255));
            pixels.setPixelColor(2, pixels.Color(255, 80,0));
            pixels.setPixelColor(3, pixels.Color(51, 0, 0));
            pixels.show();   // Send the updated pixel colors to the hardware.
            delay(100);
            pixels.setPixelColor(0, pixels.Color(0, 0, 0));
            pixels.setPixelColor(1, pixels.Color(0, 0, 0));
            pixels.setPixelColor(2, pixels.Color(0, 0, 0));
            pixels.setPixelColor(3, pixels.Color(0, 0, 0));
            pixels.show();
            delay(100);};break;
    default : break;
    }
    pixels.show();   // Send the updated pixel colors to the hardware.
    delay(DELAYVAL); // Pause before next pass through loop
  }
}
