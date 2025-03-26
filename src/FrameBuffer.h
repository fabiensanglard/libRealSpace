#pragma once
#include "precomp.h"
class FrameBuffer {
public:
    uint8_t *framebuffer{nullptr};
    int width{0};
    int height{0};
    FrameBuffer(int w, int h);
    ~FrameBuffer();	
    void Clear();
    void FillWithColor(uint8_t color);
    bool DrawShape(RLEShape *shape);
    bool DrawShapeWithBox(RLEShape *shape, int bx1, int bx2, int by1, int by2);
    void FillLineColor(size_t lineIndex, uint8_t color);
    void plot_pixel(int x, int y, uint8_t color);
    void line(int x1, int y1, int x2, int y2, uint8_t color);
    void lineWithSkip(int x1, int y1, int x2, int y2, uint8_t color, int skip);
    void lineWithBox(int x1, int y1, int x2, int y2, uint8_t color, int bx1, int bx2, int by1, int by2);
    void lineWithBoxWithSkip(int x1, int y1, int x2, int y2, uint8_t color, int bx1, int bx2, int by1, int by2,
                             int skip);
    void rect_slow(int left, int top, int right, int bottom, uint8_t color);
    void circle_slow(int x, int y, int radius, uint8_t color);
    void PrintText(RSFont *font, Point2D *coo, char *text, uint8_t color, size_t start, uint32_t size,
                   size_t interLetterSpace, size_t spaceSize);
    void PrintText(RSFont *font, Point2D coo, std::string text, uint8_t color);
    void PrintText_SM(RSFont *font, Point2D *coo, char *text, uint8_t color, size_t start, uint32_t size,
                      size_t interLetterSpace, size_t spaceSize, bool isSmall);
    void blit(uint8_t *srcBuffer, int x, int y, int width, int height);
    void blitWithMask(uint8_t *srcBuffer, int x, int y, int width, int height, uint8_t maxk);
    void blitWithMaskAndOffset(uint8_t *srcBuffer, int x, int y, int width, int height, uint8_t maxk, uint8_t offset);
    void blitLargeBuffer(uint8_t *srcBuffer, int srcWidth, int srcHeight, int srcX, int srcY, int destX, int destY,
                         int width, int height);
};