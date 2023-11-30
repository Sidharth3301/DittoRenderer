#include <lightwave.hpp>

namespace lightwave
{

    class CheckerboardTexture : public Texture
    {
        // Color m_value;
        Color color0;
        Color color1;
        int tilesize_x;
        int tilesize_y;

    public:
        CheckerboardTexture(const Properties &properties)
        {
            // m_value = properties.get<Color>("value");
            color0 = properties.get<Color>("color0");
            color1 = properties.get<Color>("color1");
            std::pair<int,int> scale;
            std::pair<int,int> x;

            x = properties.get<scale>("scale");
            // tilesize_y = properties.get<int>("scale");

        }

        Color evaluate(const Point2 &uv) const override
        {
            // float tileSize = 10.0;

            // // Calculate the checkerboard pattern
            // int xTile = int(TexCoord.x * tileSize_x);
            // int yTile = int(TexCoord.y * tileSize_y);
            // int tileSum = xTile + yTile;

            // // Determine the color based on the tile index
            // vec3 color;
            // if (mod(tileSum, 2) == 0)
            // {
            //     color = vec3(1.0, 1.0, 1.0); // White
            // }
            // else
            // {
            //     color = vec3(0.0, 0.0, 0.0); // Black
            // }

            // // Output the final color
            // FragColor = vec4(color, 1.0);
            return color1;
        }

        std::string toString() const override
        {
            return tfm::format("ConstantTexture[\n"
                               "  value = %s\n"
                               "]",
                               indent(m_value));
        }
    };

} // namespace lightwave

REGISTER_TEXTURE(CheckerboardTexture, "checkerboard")
