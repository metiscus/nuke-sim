#pragma once

#include "color.h"

struct RenderOp
{
    enum Type
    {
        DrawSprite,
        DrawRect,
        DrawLine,
        DrawText,
    };

    Type type;
    int layer;

    RenderOp(Type type_)
        : type(type_)
        , layer(0)
    {
    }

    inline bool operator<(const RenderOp& rhs) const
    {
        return rhs.layer < layer;
    }

    struct sprite_t{
        uint32_t id;
        float x;
        float y;
        float w;
        float h;
        bool stretch;
        int layer;
        
        sprite_t()
            : id (0)
            , x(0), y(0), w(0), h(0)
            , stretch(false)
            , layer(0)
        {

        }
    };

    struct text_t{
        std::string text;
        uint32_t size;
        int x;
        int y;
    };

    struct linerect_t{
        linerect_t()
            : color(Color::GetNamed(Color::Black))
        {

        }

        Color color;
        int from_x;
        int from_y;
        int to_x;
        int to_y;
        bool fill;
    };

    sprite_t sprite;
    text_t text;
    linerect_t linerect;
};