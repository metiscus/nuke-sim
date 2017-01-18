#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include "utility.h"

#include "stb/stb_truetype.h"
#include "stb/stb_image.h"

#include <GL/gl.h>

#include "renderer_gl.h"
#include "renderop.h"

#include "resourcemanager.h"
#include "binaryresource.h"

RendererGL::RendererGL(int width, int height)
    : inFrame_(false)
{
    width_ = width;
    height_ = height;

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glOrtho(0.0, width, 0.0, height, -10.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.4, 0.4, 0.4, 1.0);
    glClearDepth(0.0);
}

RendererGL::~RendererGL()
{
    free_sprites();
    free_fonts();
}

/*!
\brief perform the required functions to begin drawing a frame
*/
void RendererGL::begin()
{
    inFrame_ = true;
}

/*!
\brief perform the required functions to finish drawing a frame
*/
void RendererGL::end()
{    
    inFrame_ = false;

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // Process the list of operations
    for(auto op : render_ops_)
    {
        switch(op.type)
        {
            case RenderOp::DrawSprite:
            {
                const Sprite& sprite = sprites_[op.sprite.id];
                FloatRect dest;
                dest.x = op.sprite.x;
                dest.y = op.sprite.y;
                dest.w = op.sprite.w;
                dest.h = op.sprite.h;

                const FloatRect& src = sprite.size;

                if(sprite.tex != current_texture_)
                {
                	glBindTexture(GL_TEXTURE_2D, sprite.tex);
                	current_texture_ = sprite.tex;
                    //printf("binding texture %d\n", sprite.tex);
                }

                float verts [] = {
                    dest.x, dest.y, (float)op.layer,
                    dest.x + dest.w, dest.y, (float)op.layer,
                    dest.x + dest.w, dest.y + dest.h, (float)op.layer,
                    dest.x, dest.y + dest.h, (float)op.layer
                };

                //fprintf(stderr, "verts[%d] %f %f %f\n", 0, verts[0], verts[1], verts[2]);
                //fprintf(stderr, "verts[%d] %f %f %f\n", 1, verts[3], verts[4], verts[5]);
                //fprintf(stderr, "verts[%d] %f %f %f\n", 2, verts[6], verts[7], verts[8]);
                //fprintf(stderr, "verts[%d] %f %f %f\n", 3, verts[9], verts[10], verts[11]);

                float texcoords [] = {
                    src.x,         src.y,
                    src.x + src.w, src.y,
                    src.x + src.w, src.y + src.h,
                    src.x,         src.y + src.h
                };

                glEnableClientState(GL_VERTEX_ARRAY);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);

                glVertexPointer(3, GL_FLOAT, 0, verts);
                glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

                glDrawArrays(GL_QUADS, 0, 4);

                glDisableClientState(GL_VERTEX_ARRAY);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                break;
            }

            case RenderOp::DrawText:
            {
                FontData *fontData = fonts_[op.text.size];
                
                float x = op.text.x, y = op.text.y;
                for(auto letter : op.text.text) 
                {
                    stbtt_aligned_quad q;
                    stbtt_GetBakedQuad(fontData->cdata, 256, 256, letter-32, &x, &y, &q, 1);
                    float w = q.x1 - q.x0;
                    float h = q.y1 - q.y0;
                    FloatRect src  = { (float)(q.s0), (float)(q.t0), w, h };
                    FloatRect dest  = { (float)q.x0, (float)q.y0, w, h };
#if 0
                    float verts [] = {
                        dest.x,          y, (float)op.layer,
                        dest.x + dest.w, y, (float)op.layer,
                        dest.x + dest.w, y + op.text.size, (float)op.layer,
                        dest.x, y + op.text.size, (float)op.layer
                    };
#endif
                    float offset = (y - dest.y) - dest.h;
                    float verts [] = {
                        dest.x,          y - op.text.size + offset, (float)op.layer,
                        dest.x + dest.w, y - op.text.size + offset, (float)op.layer,
                        dest.x + dest.w, dest.y + dest.h + offset, (float)op.layer,
                        dest.x, dest.y + dest.h + offset, (float)op.layer
                    };
                    float texcoords [] = {
                        src.x, src.y + src.h / 256.0,
                        src.x + src.w / 256.0, src.y + src.h / 256.0,
                        src.x + src.w / 256.0, src.y,
                        src.x, src.y,
                    };

                    if(fontData->texture != current_texture_)
                    {
                        glBindTexture(GL_TEXTURE_2D, fontData->texture);
                        current_texture_ = fontData->texture;
                        //printf("binding texture %d\n", fontData->texture);
                    }

                    glEnableClientState(GL_VERTEX_ARRAY);
                    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                    //glEnableClientState(GL_COLOR_ARRAY);

                    glVertexPointer(3, GL_FLOAT, 0, verts);
                    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
                    //glColorPointer(4, GL_FLOAT, 0, colors);

                    glDrawArrays(GL_QUADS, 0, 4);

                    glDisableClientState(GL_VERTEX_ARRAY);
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                    glDisableClientState(GL_COLOR_ARRAY);
                }
                break;
            }

            case RenderOp::DrawRect:
            {
                glDisable(GL_TEXTURE_2D);

                uint8_t colors [] = {
                    op.linerect.color.r, op.linerect.color.g, op.linerect.color.b, op.linerect.color.a,
                    op.linerect.color.r, op.linerect.color.g, op.linerect.color.b, op.linerect.color.a,
                    op.linerect.color.r, op.linerect.color.g, op.linerect.color.b, op.linerect.color.a,
                    op.linerect.color.r, op.linerect.color.g, op.linerect.color.b, op.linerect.color.a,
                    op.linerect.color.r, op.linerect.color.g, op.linerect.color.b, op.linerect.color.a,
                    op.linerect.color.r, op.linerect.color.g, op.linerect.color.b, op.linerect.color.a,
                    op.linerect.color.r, op.linerect.color.g, op.linerect.color.b, op.linerect.color.a,
                    op.linerect.color.r, op.linerect.color.g, op.linerect.color.b, op.linerect.color.a
                };
                
                glEnableClientState(GL_VERTEX_ARRAY);
                glEnableClientState(GL_COLOR_ARRAY);

                glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
                
                if(op.linerect.fill == true)
                {
                    float verts [] = {
                        op.linerect.from_x, op.linerect.from_y, (float)op.layer,
                        op.linerect.to_x, op.linerect.from_y, (float)op.layer,
                        op.linerect.to_x, op.linerect.to_y, (float)op.layer,
                        op.linerect.from_x, op.linerect.to_y, (float)op.layer
                    };                  
                    glVertexPointer(3, GL_FLOAT, 0, verts);  
                    glDrawArrays(GL_QUADS, 0, 4);
                }
                else
                {
                   float verts [] = {
                        op.linerect.from_x, op.linerect.from_y, (float)op.layer,
                        op.linerect.to_x, op.linerect.from_y, (float)op.layer,
                        op.linerect.to_x, op.linerect.from_y, (float)op.layer,
                        op.linerect.to_x, op.linerect.to_y, (float)op.layer,
                        op.linerect.to_x, op.linerect.to_y, (float)op.layer,
                        op.linerect.from_x, op.linerect.to_y, (float)op.layer,
                        op.linerect.from_x, op.linerect.from_y, (float)op.layer,
                        op.linerect.from_x, op.linerect.to_y, (float)op.layer
                    };                  
                    glVertexPointer(3, GL_FLOAT, 0, verts); 
                    glLineWidth(5);
                    glDrawArrays(GL_LINES, 0, 8);
                }
                
                glDisableClientState(GL_VERTEX_ARRAY);
                glDisableClientState(GL_COLOR_ARRAY);
                glEnable(GL_TEXTURE_2D);

                break;
            }

            case RenderOp::DrawLine:
            {
                //SDL_SetRenderDrawColor(renderer_, op.linerect.color.r, op.linerect.color.g, op.linerect.color.b, op.linerect.color.a);
                //SDL_RenderDrawLine(renderer_, op.linerect.from_x, op.linerect.from_y, op.linerect.to_x, op.linerect.to_y);
                //SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0);
                break;
            }
        }
    }

    render_ops_.clear();
}

/*!
\brief Loads various resources from a specified file
*/
bool RendererGL::load_data_file(const std::string& filename)
{
#if 0
    namespace fs = boost::filesystem;
    bool result = true;
    if(fs::exists(filename))
    {
        std::string path = fs::extension(filename);
        printf("RendererGL::load_data_file path is '%s'\n", path.c_str());
        if(path == ".spr")
        {
            //load a spritesheet text file
            std::ifstream infile(filename.c_str());
            while(!infile.eof()) {
                std::string line;
                std::getline(infile, line);
                std::stringstream ss;
                ss<<line;

                // get the filename for the sprite texture and ensure it is loaded
                std::string filename;
                ss>>filename;
                if(load_data_file(filename))
                {
                    // now build a sprite from the texture
                    auto itr = textures_.find(filename);
                    assert(itr != textures_.end());

                    Sprite spr;
                    spr.tex = itr->second.handle;
                    uint32_t id = 0;
                    ss>>id;
                    ss>>spr.size.x;
                    ss>>spr.size.y;
                    ss>>spr.size.w;
                    ss>>spr.size.h;

                    //fprintf(stderr, "sprite %f %f %f %f\n", spr.size.x, spr.size.y, spr.size.w, spr.size.h);

                    spr.size.x /= (float)itr->second.width;
                    spr.size.y /= (float)itr->second.height;
                    spr.size.w /= (float)itr->second.width;
                    spr.size.h /= (float)itr->second.height;
                    sprites_.insert(std::make_pair(id, spr));

                    printf("Loaded sprite %d\n", id);
                }
            }
        }
        else if (path == ".png")
        {
            result = load_texture(filename.c_str());
        }
    }
    else
    {
        printf("Failed to open '%s' for read.\n", filename.c_str());
        result = false;
    }

    return result;
#endif
    return false;
}

/*!
\brief render a sprite on the screen
\param id the unique id of the sprite to draw
\param x the position in pixels on the screen (horizontal)
\param y the position in pixels on the screen (vertical)
*/
void RendererGL::draw_sprite(uint32_t id, float x, float y, int layer)
{
    assert(sprites_.count(id)>0);
    assert(inFrame_ == true);
    
    RenderOp op(RenderOp::DrawSprite);
    op.layer = layer;
    op.sprite.id = id;
    op.sprite.x = x;
    op.sprite.y = y;
    store_render_op(op);
}

/*!
\brief render a sprite on the screen
\param id the unique id of the sprite to draw
\param x the position in pixels on the screen (horizontal)
\param y the position in pixels on the screen (vertical)
*/
void RendererGL::draw_sprite_size(uint32_t id, float x, float y, float w, float h, int layer)
{
    assert(sprites_.count(id)>0);
    assert(inFrame_ == true);
    
    RenderOp op(RenderOp::DrawSprite);
    op.layer = layer;
    op.sprite.id = id;
    op.sprite.x = x;
    op.sprite.y = y;
    op.sprite.w = w;
    op.sprite.h = h;
    op.sprite.stretch = true;
    store_render_op(op);
}

/*!
\brief draws a rectangle on the screen, optionally fills it with color
\param color the color to use when drawing
\param from_x the leftmost point of the box
\param from_y the topmost point of the box
\param to_x the rightmost point of the box 
\param to_y the bottommost point of the box
\param fill should the box be drawn filled in or just an outline
\warning Has a side effect of resetting the draw color to black
*/
void RendererGL::draw_rectangle(const Color& color, int from_x, int from_y, int to_x, int to_y, bool fill, int layer)
{
    assert(inFrame_ == true);
    
    if(from_x == to_x && from_y == to_y) {
        assert(false);
        return;
    }
    
    RenderOp op(RenderOp::DrawRect);
    op.layer = layer;
    op.linerect.color = color;
    op.linerect.from_x = from_x;
    op.linerect.from_y = from_y;
    op.linerect.to_x = to_x;
    op.linerect.to_y = to_y;
    op.linerect.fill = fill;
    store_render_op(op);
}

/*!
\brief draws a line on the screen
\param color the color to use when drawing
\param from_x the leftmost point of the line
\param from_y the topmost point of the line
\param to_x the rightmost point of the line
\param to_y the bottommost point of the line
\warning Has a side effect of resetting the draw color to black
*/
void RendererGL::draw_line(const Color& color, int from_x, int from_y, int to_x, int to_y)
{
    assert(inFrame_ == true);

    if(from_x == to_x && from_y == to_y) {
        assert(!(from_x == to_x && from_y == to_y));
        return;
    }
    
    RenderOp op(RenderOp::DrawLine);
    op.linerect.color = color;
    op.linerect.from_x = from_x;
    op.linerect.from_y = from_y;
    op.linerect.to_x = to_x;
    op.linerect.to_y = to_y;
    store_render_op(op);
}

/*!
\brief draws a text string on the screen
\param text the string to draw
\param size the font size (in screen pixels) of the font
\param from_x the leftmost point of the line of text
\param from_y the topmost point of the line of text
\warning Requires the font to have been loaded with the correct size
*/
void RendererGL::draw_text(const std::string& text, uint32_t size, int from_x, int from_y, int layer)
{
    assert(inFrame_ == true);
    assert(fonts_.count(size)!=0);
    
    RenderOp op(RenderOp::DrawText);
    op.layer = layer;
    op.text.text = text;
    op.text.size = size;
    op.text.x = from_x;
    op.text.y = from_y;
    store_render_op(op);
}

void RendererGL::store_render_op(RenderOp& op)
{
    auto itr = render_ops_.begin();
    while(itr != render_ops_.end() && *itr < op)
    {
        ++itr;
    }
    render_ops_.emplace(itr, op);
}

bool RendererGL::load_texture(const char* filename)
{
#if 0
    namespace fs = boost::filesystem;
    assert(filename != nullptr);
    bool result = false;

    auto itr = textures_.find(filename);

    if((itr==textures_.end()) && (fs::exists(filename)))
    {
        // generate the texture and set it up
        Texture texture;
        glGenTextures(1, &texture.handle);
        glBindTexture(GL_TEXTURE_2D, texture.handle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        // set the texture
        void *pixels = stbi_load(filename, &texture.width, &texture.height, &texture.channels, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        stbi_image_free(pixels);

        //fprintf(stderr, "Loaded texture %s using texture %d\n", filename, texture.handle);

        textures_.insert(std::make_pair(std::string(filename), texture));
        result = true;
    }
    else if(itr != textures_.end())
    {
        result = true;
    }
    return result;
#endif
    return false;
}

void RendererGL::load_sprites(const char* filename)
{
    assert(false);
}

void RendererGL::free_sprites()
{
    for(auto itr : textures_) {
        //TODO!
        //glDeleteTexture(1, &itr->handle);
    }
    sprites_.clear();
}

void RendererGL::load_font(const char*filename, const std::vector<uint32_t>& sizes)
{
    //TODO: break font loading out later
    Resource::Guid guid = Resource::random_guid();
    auto guid_future = ResourceManager::load_file_as_resource(std::string(filename), guid, BinaryResourceType);
    guid_future.wait();
    guid = guid_future.get();
    if(!guid.is_nil())
    {
        std::shared_ptr<BinaryResource> res = std::static_pointer_cast<BinaryResource>(ResourceManager::get_resource(guid));

        const uint8_t *ttf_buffer = res->get_data();
        uint8_t pixels[256*256];
        for(uint32_t ii=0; ii<sizes.size(); ++ii) {
            FontData *font = new FontData();

            // generate the texture and set it up
            glGenTextures(1, &font->texture);
            glBindTexture(GL_TEXTURE_2D, font->texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        
            stbtt_BakeFontBitmap(ttf_buffer, 0, sizes[ii], (unsigned char*)pixels, 256, 256, 32, 96, font->cdata);
            
            fprintf(stderr, "Processing texture.");
            uint8_t *new_pixels = (uint8_t*)malloc(256*256*4);
            for(int i=0; i<256*256; ++i)
            {
                new_pixels[i*4 + 0] = pixels[i];
                new_pixels[i*4 + 1] = pixels[i];
                new_pixels[i*4 + 2] = pixels[i];
                new_pixels[i*4 + 3] = pixels[i];
            }
            fprintf(stderr, "...done\n");

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, new_pixels);
            free(new_pixels);

            fprintf(stderr, "Loaded font %s size %d using texture %d\n", filename, sizes[ii], font->texture);

            fonts_[sizes[ii]] = font;
        }
    }
    else
    {
        fprintf(stderr, "Failed to load the font!\n");
    }
}

void RendererGL::free_fonts()
{
    for(auto &fnt : fonts_)
    {
        //SDL_DestroyTexture(fnt.second->texture);
        //TODO!
        delete fnt.second;
    }
}