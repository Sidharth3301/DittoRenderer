// /**
//  * @brief Contains the denoise interface.
//  */

// #include <lightwave/core.hpp>
// #include <lightwave/color.hpp>
// #include <lightwave/math.hpp>
// #include <lightwave/image.hpp>
// #include<lightwave.hpp>

// namespace lightwave {

// /**
//  * @brief Post processes alter an input image to produce an improved output image (e.g., tonemapping or denoising).
//  */
// class Denoise final : public Postprocess {
// protected:
//     /// @brief The input image that is to be processed.
//     ref<Image> m_input;
//     /// @brief The output image that will be produced.
//     ref<Image> m_output;

// public:
//     Denoise(const Properties &properties) {
//         m_input = properties.get<Image>("input");
//         m_output = properties.getChild<Image>();
//     }
// };

// }
// REGISTER_POSTPROCESS(Denoise, "denoise")
