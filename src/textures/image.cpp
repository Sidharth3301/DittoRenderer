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
            auto u = uv.x();
            auto v = (1 - uv.y());
            Color c;
            Point2i im_res = m_image->resolution();

            if (m_filter == FilterMode::Nearest)
            {
                Point2 scaled_uv, bordered_cord;
                scaled_uv.x() = u * im_res.x();
                scaled_uv.y() = v * im_res.y();

                if (m_border == BorderMode::Clamp)
                {
                    bordered_cord.x() = scaled_uv.x() < 0 ? 0 : (scaled_uv.x() > (im_res.x() - 1) ? (im_res.x() - 1) : scaled_uv.x());
                    bordered_cord.y() = scaled_uv.y() < 0 ? 0 : (scaled_uv.y() > (im_res.y() - 1) ? (im_res.y() - 1) : scaled_uv.y());
                }
                else
                {    //we use this so that even if scaled_uv is negative the modulus will work
                    bordered_cord.x() = scaled_uv.x() - floor(scaled_uv.x() / im_res.x()) * im_res.x();
                    bordered_cord.y() = scaled_uv.y() - floor(scaled_uv.y() / im_res.y()) * im_res.y();
                }
                Point2i lattice_uv;

                lattice_uv.x() = min(floor(bordered_cord.x()), im_res.x() - 1);
                lattice_uv.y() = min(floor(bordered_cord.y()), im_res.y() - 1);

                c = (*m_image)(lattice_uv);
                c *= m_exposure;
            }
            else
            {
                Point2 scaled_uv, bordered_cord;
                scaled_uv.x() = u * im_res.x() - 0.5;
                scaled_uv.y() = v * im_res.y() - 0.5;

                if (m_border == BorderMode::Clamp)
                {
                    bordered_cord.x() = scaled_uv.x() < 0 ? 0 : (scaled_uv.x() > (im_res.x() - 1) ? (im_res.x() - 1) : scaled_uv.x());
                    bordered_cord.y() = scaled_uv.y() < 0 ? 0 : (scaled_uv.y() > (im_res.y() - 1) ? (im_res.y() - 1) : scaled_uv.y());
                }
                else
                {
                    bordered_cord.x() = scaled_uv.x() - floor(scaled_uv.x() / im_res.x()) * im_res.x();
                    bordered_cord.y() = scaled_uv.y() - floor(scaled_uv.y() / im_res.y()) * im_res.y();
                }

                Point2i lattice_cord{int(floor(bordered_cord.x())), int(floor(bordered_cord.y()))};

                float fu = bordered_cord.x() - lattice_cord.x();
                float fv = bordered_cord.y() - lattice_cord.y();

                int lu1 = lattice_cord.x() + 1;
                int lv1 = lattice_cord.y() + 1;

                if (m_border == BorderMode::Clamp)
                {
                    lu1 = lu1 < 0 ? 0 : (lu1 > (im_res.x() - 1) ? (im_res.x() - 1) : lu1);
                    lv1 = lv1 < 0 ? 0 : (lv1 > (im_res.y() - 1) ? (im_res.y() - 1) : lv1);
                }
                else
                {
                    lu1 = lu1 - floor(lu1 / im_res.x()) * im_res.x();
                    lv1 = lv1 - floor(lv1 / im_res.y()) * im_res.y();
                }

                c = (1 - fu) * (1 - fv) * (*m_image)(lattice_cord) +
                    (1 - fu) * fv * (*m_image)(Point2i(lattice_cord.x(), lv1)) +
                    fu * (1 - fv) * (*m_image)(Point2i(lu1, lattice_cord.y())) +
                    fu * fv * (*m_image)(Point2i(lu1, lv1));
                c *= m_exposure;
            }
            return c;
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
