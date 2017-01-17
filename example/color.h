#pragma once

struct Color
{
    uint8_t r, g, b, a;
    enum Name
    {
      First,
      BrRed = First,
      Red,
      BrGreen,
      Green,
      BrBlue,
      Blue, 
      Black,
      White,
      Count
    };
    
    Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_)
        : r(r_), g(g_), b(b_), a(a_)
    {
        ;
    }
    
    static Color GetNamed(const Name& name)
    {
        const static Color colors[] = 
        {
            { 255,   0, 0, 255 },
            { 128,   0, 0, 255 },
            {   0, 255, 0, 255 },
            {   0, 128, 0, 255 },
            {   0,   0, 255, 255 },
            {   0,   0, 128, 255 },
            {   0,   0,   0, 255 },
            { 255, 255, 255, 255 }
        };

        if (name >= Count) {
            return colors[First];
        }
        else {
            return colors[name];
        }
    }

    static Color FromRGBA(int r, int g, int b, int a)
    {
        return Color(r, g, b, a);
    }

};
