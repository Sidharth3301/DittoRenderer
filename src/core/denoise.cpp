/**
 * @brief Contains the denoise interface.
 */

#include <lightwave/core.hpp>
#include <lightwave/color.hpp>
#include <lightwave/math.hpp>
#include <lightwave/image.hpp>
#include <lightwave.hpp>
#include <OpenImageDenoise/oidn.hpp>

namespace lightwave
{

    /**
     * @brief Post processes alter an input image to produce an improved output image (e.g., tonemapping or denoising).
     */
    class Denoise final : public Postprocess
    {
    protected:
        /// @brief The input image that is to be processed.
        ref<Image> m_input;
        /// @brief The output image that will be produced.
        ref<Image> m_output;
        ref<Image> m_albedo;
        ref<Image> m_normals;

    public:
        Denoise(const Properties &properties) : Postprocess(properties)
        {
            m_input = properties.get<Image>("input");
        m_output = properties.getChild<Image>();
            m_normals = properties.get<Image>("normals", nullptr);
            m_albedo = properties.get<Image>("albedo", nullptr);
        }
        virtual void execute() override
        {
            const Point2i resolution = m_input->resolution();
            auto width = m_input->resolution().x();
            auto height = m_input->resolution().y();
            m_output->initialize(resolution);
            if (!m_input || !m_output)
                throw std::runtime_error("Input or output image not set for Denoising");

            // Initialize OIDN device
            oidn::DeviceRef device = oidn::newDevice(oidn::DeviceType::CPU);
            device.commit();

            // Create a filter for denoising a beauty (color) image using optional auxiliary images too
            // This can be an expensive operation, so try no to create a new filter for every image!
            oidn::FilterRef filter = device.newFilter("RT");                                // generic ray tracing filter
            filter.setImage("color", m_input->data(), oidn::Format::Float3, width, height); // beauty
            if (m_normals && m_albedo)
            {
                filter.setImage("albedo", m_albedo->data(), oidn::Format::Float3, width, height);  // auxiliary
                filter.setImage("normal", m_normals->data(), oidn::Format::Float3, width, height); // auxiliary
            }
            filter.setImage("output", m_output->data(), oidn::Format::Float3, width, height); // denoised beauty
            filter.set("hdr", true);                                                          // beauty image is HDR
            filter.setProgressMonitorFunction([](void *userPtr, double n) -> bool
                                              {
                logger(EDebug, "Denoising image...\n");
                return true; });

            filter.commit();

            filter.execute();
            // Assuming m_output has a method like setData(float* data, size_t size)
            // You need to know or calculate the size of the data buffer. Here's an example calculation:
            const char *errorMessage;
            if (device.getError(errorMessage) != oidn::Error::None)
                throw std::runtime_error(errorMessage);

            logger(EDebug, "Saving denoised image...\n");
            m_output->save(); // Save the output image

            // Show the image on Tev
            Streaming stream{*m_output};
            stream.update();
        }

        std::string toString() const override
        {
            return tfm::format("denoiser");
        }
    };

}
REGISTER_POSTPROCESS(Denoise, "denoise")
