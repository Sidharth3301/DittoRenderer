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
            Point2 modified_uv;
            Point2i im_res = m_image->resolution();
            switch (m_border)
            {
            case BorderMode::Clamp:
            {
                modified_uv.x() = modified_uv.x() < 0 ? 0 : (modified_uv.x() > 1 ? 1 : modified_uv.x());
                modified_uv.y() = modified_uv.y() < 0 ? 0 : (modified_uv.y() > 1 ? 1 : modified_uv.y());
                break;
            }
            case BorderMode::Repeat:
            {
                modified_uv.x() = modified_uv.x() < 0 ? 1 : (modified_uv.x() > 1 ? 0 : modified_uv.x());
                modified_uv.y() = modified_uv.y() < 0 ? 1 : (modified_uv.y() > 1 ? 0 : modified_uv.y());
                break;
            }
            }
            switch (m_filter)
            {
            case FilterMode::Nearest:
            {
                return nearest_coord(modified_uv, im_res);
                break;
            }
            case FilterMode::Bilinear:
            {
                return bilinear_interp(modified_uv, im_res);
                break;
            }
            }
            return Color(0.0f);
        }

        Color nearest_coord(const Point2 &uv, Point2i &im_res) const
        {
            auto u = uv.x() * im_res.x();
            auto v = uv.y() * im_res.y();

            auto lu = min(floorf(u), im_res.x() - 1);
            auto lv = min(floorf(v), im_res.y() - 1);

            return (*m_image)(Point2i(lu, lv));
        }

        Color bilinear_interp(const Point2 &uv, Point2i &im_res) const
        {
            auto u = uv.x() * (im_res.x() - 1);
            auto v = uv.y() * (im_res.y() - 1);

            auto uf = floorf(u);
            auto vf = floorf(v);
            auto fu = u - uf;
            auto fv = v - vf;

            return (1 - fu) * (1 - fv) * (*m_image)(Point2i(uf, vf)) + (1 - fu) * (fv) * (*m_image)(Point2i(uf, vf + 1)) + fu * (1 - fv) * (*m_image)(Point2i(uf + 1, vf)) + fu * fv * (*m_image)(Point2i(uf + 1, vf + 1));
        }

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
