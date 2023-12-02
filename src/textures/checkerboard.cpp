#include <lightwave.hpp>

namespace lightwave
{

    class CheckerboardTexture : public Texture
    {
        // Color m_value;
        Color color0{0.0f};
        Color color1{1.0f};
        int tilesize_x;
        int tilesize_y;

    public:
        CheckerboardTexture(const Properties &properties)
        {
            // m_value = properties.get<Color>("value");
            color0 = properties.get<Color>("color0");
            color1 = properties.get<Color>("color1");

            Vector2 scale = properties.get<Vector2>("scale");
            tilesize_x = scale[0];
            tilesize_y = scale[1];
        }

        Color evaluate(const Point2 &uv) const override
        {   
            if (uv.x()<0 || uv.x()>1) logger(EInfo, "wronguv.x");
            if (uv.y()<0 || uv.y()>1) logger(EInfo, "wronguv.y");
            int xTile = int(floorf(uv.x()*tilesize_x));
            int yTile = int(floorf(uv.y()*tilesize_y));
            int tileSum = xTile + yTile;

            // Determine the color based on the tile index
            if ((tileSum % 2) == 1)
            {
                return color1; // White
            }
            else
            {
                return color0; // Black
            }
        }

        std::string toString() const override
        {
            return tfm::format("checkerboardTexture[\n"
                               "  color0 = %s\n"
                               "]",
                               indent(color0));
        }
    };

} // namespace lightwave

REGISTER_TEXTURE(CheckerboardTexture, "checkerboard")
