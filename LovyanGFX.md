# LovyanGFX Library Usage

```cpp
 setColor      (            0xFF0000U);                    // Set color
 fillScreen    (                color);                    // Fill the entire screen
 drawPixel     ( x, y         , color);                    // Pixel
 drawFastVLine ( x, y   , h   , color);                    // Vertical line
 fillRect      ( x, y, w, h   , color);                    // Fill rectangle
 drawFastHLine ( x, y, w      , color);                    // Horizontal line
 drawRect      ( x, y, w, h   , color);                    // Outline of rectangle
 drawRoundRect ( x, y, w, h, r, color);                    // Outline of rounded rectangle
 fillRoundRect ( x, y, w, h, r, color);                    // Fill rounded rectangle
 drawCircle    ( x, y      , r, color);                    // Outline of circle
 fillCircle    ( x, y      , r, color);                    // Fill circle
 drawEllipse   ( x, y, rx, ry , color);                    // Outline of ellipse
 fillEllipse   ( x, y, rx, ry , color);                    // Fill ellipse
 drawLine      ( x0, y0, x1, y1        , color);           // Line between two points
 drawTriangle  ( x0, y0, x1, y1, x2, y2, color);           // Outline of triangle
 fillTriangle  ( x0, y0, x1, y1, x2, y2, color);           // Fill triangle
 drawBezier    ( x0, y0, x1, y1, x2, y2, color);           // Bezier curve with three points
 drawBezier    ( x0, y0, x1, y1, x2, y2, x3, y3, color);   // Bezier curve with four points
 drawArc       ( x, y, r0, r1, angle0, angle1, color);     // Outline of arc
 fillArc       ( x, y, r0, r1, angle0, angle1, color);     // Fill arc

// The drawGradientLine function, which draws a gradient line, cannot omit the color specification.
    drawGradientLine( 0, 80, 80, 0, 0xFF0000U, 0x0000FFU); // Gradient line from red to blue

// You can fill the entire screen with clear or fillScreen.
// fillScreen is the same as specifying the entire screen with fillRect, and the color specification is treated as the drawing color.

    lcd.fillScreen(0xFFFFFFu);  // Fill with white
    lcd.setColor(0x00FF00u);    // Set the drawing color to green
    lcd.fillScreen();           // Fill with green

// clear is treated separately from the drawing functions as a background color.
// The background color is rarely used, but it is also used as the color to fill gaps when using the scroll function.

    lcd.clear(0xFFFFFFu);       // Specify white as the background color and fill
    lcd.setBaseColor(0x000000u);// Specify black as the background color
    lcd.clear();                // Fill with black

// Allocation and release of the SPI bus are done automatically when a drawing function is called,
// but if you prioritize drawing speed, use startWrite and endWrite before and after the drawing process.
// This suppresses the allocation and release of the SPI bus and improves speed.
// In the case of an electronic paper display (EPD), drawings after startWrite() are reflected on the screen when endWrite() is called.

    lcd.drawLine(0, 1, 39, 40, red);       // Allocate SPI bus, draw line, release SPI bus
    lcd.drawLine(1, 0, 40, 39, blue);      // Allocate SPI bus, draw line, release SPI bus
    lcd.startWrite();                      // Allocate SPI bus
    lcd.drawLine(38, 0, 0, 38, 0xFFFF00U); // Draw line
    lcd.drawLine(39, 1, 1, 39, 0xFF00FFU); // Draw line
    lcd.drawLine(40, 2, 2, 40, 0x00FFFFU); // Draw line
    lcd.endWrite();                        // Release SPI bus

// startWrite and endWrite internally count the number of calls,
// and if called repeatedly, only the first and last calls will take effect.
// Always use startWrite and endWrite in pairs.
// (If you don't mind monopolizing the SPI bus, you can call startWrite once at the beginning and not call endWrite.)

    lcd.startWrite();     // Count +1, allocate SPI bus
    lcd.startWrite();     // Count +1
    lcd.startWrite();     // Count +1
    lcd.endWrite();       // Count -1
    lcd.endWrite();       // Count -1
    lcd.endWrite();       // Count -1, release SPI bus
    lcd.endWrite();       // Does nothing
// Note that if endWrite is called excessively, it will do nothing, and the count will not go negative.

// To forcibly release and allocate the SPI bus regardless of the count state of startWrite,
// use endTransaction and beginTransaction.
// Be careful as the count is not cleared, so the consistency is maintained.

    lcd.startWrite();       // Count +1, allocate SPI bus
    lcd.startWrite();       // Count +1
    lcd.drawPixel(0, 0);    // Draw
    lcd.endTransaction();   // Release SPI bus
    // Here, other SPI devices can be used
    // If using another device (like an SD card) on the same SPI bus,
    // make sure to do it only when the SPI bus is released.
    lcd.beginTransaction(); // Allocate SPI bus
    lcd.drawPixel(0, 0);    // Draw
    lcd.endWrite();         // Count -1
    lcd.endWrite();         // Count -1, release SPI bus

// Apart from drawPixel, there is a function called writePixel that draws a pixel.
// While drawPixel allocates the SPI bus as needed,
// writePixel does not check the status of the SPI bus.

    lcd.startWrite();  // Allocate SPI bus
    for (uint32_t x = 0; x < 128; ++x) {
        for (uint32_t y = 0; y < 128; ++y) {
            lcd.writePixel(x, y, lcd.color888( x*2, x + y, y*2));
        }
    }
    lcd.endWrite();    // Release SPI bus
// You must explicitly call startWrite before using any function that starts with write~.
// This applies to writePixel, writeFastVLine, writeFastHLine, and writeFillRect.

// The same drawing functions can be used for drawing to a sprite (off-screen).
// First, specify the color depth of the sprite with setColorDepth. (If omitted, it is treated as 16.)
//sprite.setColorDepth(1);   // Set to 1 bit (2 colors) palette mode
//sprite.setColorDepth(2);   // Set to 2 bits (4 colors) palette mode
//sprite.setColorDepth(4);   // Set to 4 bits (16 colors) palette mode
//sprite.setColorDepth(8);   // Set to 8 bits of RGB332
//sprite.setColorDepth(16);  // Set to 16 bits of RGB565
    sprite.setColorDepth(24);  // Set to 24 bits of RGB888

//Sprites

    static LGFX_Sprite sprite(&lcd);

// Allocate memory by specifying the width and height with createSprite.
// The memory consumed is proportional to the color depth and area. Be careful as too large sizes can fail to allocate memory.
    sprite.createSprite(65, 65); // Create a sprite with width 65 and height 65.

    for (uint32_t x = 0; x < 64; ++x) {
        for (uint32_t y = 0; y < 64; ++y) {
        sprite.drawPixel(x, y, lcd.color888(3 + x*4, (x + y)*2, 3 + y*4));  // Draw on the sprite
        }
    }
    sprite.drawRect(0, 0, 65, 65, 0xFFFF);

// The created sprite can be output at any coordinate with pushSprite.
// The output destination is the LGFX that was passed as an argument when the instance was created.
    sprite.pushSprite(64, 0);        // Draw the sprite at coordinates 64,0 on the lcd

// If you haven't passed the drawing destination pointer when creating the sprite instance,
// or if there are multiple LGFX instances, you can also specify the output destination as the first argument and pushSprite.
    sprite.pushSprite(&lcd, 0, 64);  // Draw the sprite at coordinates 0,64 on the lcd

    delay(1000);

  // The sprite can be drawn rotated and scaled with pushRotateZoom.
  // The coordinates set with setPivot are treated as the center of rotation, and it is drawn so that the rotation center is positioned at the drawing destination coordinates.
    sprite.setPivot(32, 32);    // Treat coordinates 32,32 as the center
    int32_t center_x = lcd.width() / 2;
    int32_t center_y = lcd.height() / 2;
    lcd.startWrite();
    for (int angle = 0; angle <= 360; ++angle) {
        sprite.pushRotateZoom(center_x, center_y, angle, 2.5, 3); // Draw at the screen center with angle, width scale 2.5, height scale 3

        if ((angle % 36) == 0) lcd.display(); // Update the display every 36 turns for electronic paper
    }
    lcd.endWrite();

    delay(1000);

// Use deleteSprite to free the memory of a sprite that is no longer in use.
    sprite.deleteSprite();  
    
    // Even after deleteSprite, the same instance can be reused.
    sprite.setColorDepth(4);     // Set to 4-bit (16 colors) palette mode
    sprite.createSprite(65, 65);    
    
    // In palette mode sprites, the color argument of the drawing function is treated as a palette number.
    // When drawing with pushSprite, etc., the actual drawing color is determined by referring to the palette.  
    // In 4-bit (16 colors) palette mode, palette numbers 0-15 are available.
    // The initial palette colors are black for 0 and white for the last palette, with a gradient from 0 to the last.
    // Use setPaletteColor to set the colors of the palette.
    sprite.setPaletteColor(1, 0x0000FFU);    // Set palette number 1 to blue
    sprite.setPaletteColor(2, 0x00FF00U);    // Set palette number 2 to green
    sprite.setPaletteColor(3, 0xFF0000U);    // Set palette number 3 to red 
    sprite.fillRect(10, 10, 45, 45, 1);             // Fill rectangle with palette number 1
    sprite.fillCircle(32, 32, 22, 2);               // Fill circle with palette number 2
    sprite.fillTriangle(32, 12, 15, 43, 49, 43, 3); // Fill triangle with palette number 3  
    
    // The last argument of pushSprite can specify a color to be treated as transparent.
    sprite.pushSprite( 0,  0, 0);  // Draw sprite with palette 0 treated as transparent
    sprite.pushSprite(65,  0, 1);  // Draw sprite with palette 1 treated as transparent
    sprite.pushSprite( 0, 65, 2);  // Draw sprite with palette 2 treated as transparent
    sprite.pushSprite(65, 65, 3);  // Draw sprite with palette 3 treated as transparent 
    
    delay(5000);    
    
    lcd.startWrite(); // By calling startWrite() here, the SPI bus is kept occupied.


```
