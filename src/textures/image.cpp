#include <lightwave.hpp>

namespace lightwave
{

    class ImageTexture : public Texture
    {
        enum class BorderMode
        {
            Clamp,
            Repeat,
        };

        enum class FilterMode
        {
            Nearest,
            Bilinear,
        };

        ref<Image> m_image;
        float m_exposure;
        BorderMode m_border;
        FilterMode m_filter;

    public:
        ImageTexture(const Properties &properties)
        {
            if (properties.has("filename"))
            {
                m_image = std::make_shared<Image>(properties);
            }
            else
            {
                m_image = properties.getChild<Image>();
            }
            m_exposure = properties.get<float>("exposure", 1);

            m_border =
                properties.getEnum<BorderMode>("border", BorderMode::Repeat,
                                               {
                                                   {"clamp", BorderMode::Clamp},
                                                   {"repeat", BorderMode::Repeat},
                                               });

            m_filter = properties.getEnum<FilterMode>(
                "filter", FilterMode::Bilinear,
                {
                    {"nearest", FilterMode::Nearest},
                    {"bilinear", FilterMode::Bilinear},
                });
        }

        Color evaluate(const Point2 &uv) const override
        {
            // Point2 modified_uv;
            auto u = uv.x();
            auto v = uv.y();
            auto im_res = m_image->resolution();
            int im_height = m_image->resolution()[0];
            int im_width = m_image->resolution()[1];
            Color c;

            auto y = (1 - u) * im_height;
            auto x = v * im_width;

            auto fx = x - floorf(x);
            auto fy = y - floorf(y);

            auto int_cord = Point2i(floorf(y), floorf(x));
            auto frac_cord = Point2(fy, fx);

            Point2i lat_cord = bordermodes(int_cord, im_res);

            // logger(EInfo, "bordermode test for coord ");
            // border_test(im_res);

            switch (m_filter)
            {
            case FilterMode::Nearest:
            {
                lat_cord.x() = min(lat_cord.x(), im_height - 1);
                lat_cord.y() = min(lat_cord.y(), im_width - 1);
                c = (*m_image)(lat_cord);
                break;
            }
            case FilterMode::Bilinear:
            {
                auto fu = frac_cord[0];
                auto fv = frac_cord[1];
                Point2i p1 = lat_cord;
                auto p2 = bordermodes((lat_cord + Vector2i(0, 1)), im_res);
                auto p3 = bordermodes((lat_cord + Vector2i(1, 0)), im_res);
                auto p4 = bordermodes((lat_cord + Vector2i(1, 1)), im_res);

                c = (1 - fu) * (1 - fv) * (*m_image)(p1) + (1 - fu) * (fv) * (*m_image)(p2) +
                    fu * (1 - fv) * (*m_image)(p3) + fu * fv * (*m_image)(p4);
                    break;
            }
            }
            return c;
        }

        Point2i
        bordermodes(const Point2i &lat_coords, const Point2i &im_res) const
        {
            Point2i new_lat_cords;
            int im_height = im_res[0];
            int im_width = im_res[1];
            switch (m_border)
            {
            case BorderMode::Clamp:
            {
                new_lat_cords[1] = lat_coords[1] < 0 ? 0 : (lat_coords[1]> (im_width - 1) ? (im_width - 1) : lat_coords[1]);
                new_lat_cords[0] = lat_coords[0] < 0 ? 0 : (lat_coords[0] > (im_height - 1) ? (im_height - 1) : lat_coords[0]);
                break;
            }
            case BorderMode::Repeat:
            {
                new_lat_cords[1] = lat_coords[1] < 0 ? (im_width - 1) : (lat_coords[1] > (im_width - 1) ? 0 : lat_coords[1]);
                new_lat_cords[0] = lat_coords[0] < 0 ? (im_height - 1) : (lat_coords[0] > (im_height - 1) ? 0 : lat_coords[0]);
                break;
            }
            }


            return new_lat_cords;
        }
        // Point2i border_test(Point2i& input, Point2i& im_res){
        //     return bordermodes(input, im_res);
        // }
        // void border_test_input(Point2i& im_res){
        //     int im_height = im_res[0];
        //     int im_width = im_res[1];

        //     logger(EInfo, "border_test for +1,+1");

        // }

        // Color bilinear_interp(Point2i &int_cord, Point2 &frac_cord, Point2i &im_res) const
        // {
        //     auto fu = frac_cord[0];
        //     auto fv = frac_cord[1];
        //     auto p1 = int_cord;
        //     auto p2 = bordermodes(Point2i(int_cord.x(), int_cord.y() + 1), im_res);
        //     auto p3 = bordermodes(Point2i(int_cord.x() + 1, int_cord.y()), im_res);
        //     auto p4 = bordermodes(Point2i(int_cord.x() + 1, int_cord.y() + 1), im_res);

        //     return (1 - fu) * (1 - fv) * (*m_image)(p1) + (1 - fu) * (fv) * (*m_image)(p2) + fu * (1 - fv) * (*m_image)(p3) + fu * fv * (*m_image)(p4);
        // }

        std::string toString() const override
        {
            return tfm::format("ImageTexture[\n"
                               "  image = %s,\n"
                               "  exposure = %f,\n"
                               "]",
                               indent(m_image), m_exposure);
        }
    };

} // namespace lightwave

REGISTER_TEXTURE(ImageTexture, "image")
