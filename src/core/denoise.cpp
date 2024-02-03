#include <OpenImageDenoise/oidn.hpp>
#include <lightwave>
namespace lightwave{
class Denoise{

    public:
oidn::DeviceRef device = oidn::newDevice(); // CPU or GPU if available
device.commit();

// Create buffers for input/output images accessible by both host (CPU) and device (CPU/GPU)
oidn::BufferRef colorBuf  = device.newBuffer(width * height * 3 * sizeof(float));
// oidn::BufferRef albedoBuf = 

// Create a filter for denoising a beauty (color) image using optional auxiliary images too
// This can be an expensive operation, so try no to create a new filter for every image!
oidn::FilterRef filter = device.newFilter("RT"); // generic ray tracing filter
filter.setImage("color",  colorBuf,  oidn::Format::Float3, width, height); // beauty
filter.setImage("albedo", albedoBuf, oidn::Format::Float3, width, height); // auxiliary
filter.setImage("normal", normalBuf, oidn::Format::Float3, width, height); // auxiliary
filter.setImage("output", colorBuf,  oidn::Format::Float3, width, height); // denoised beauty
filter.set("hdr", true); // beauty image is HDR
filter.commit();

// Fill the input image buffers
float* colorPtr = (float*)colorBuf.getData();
...

// Filter the beauty image
filter.execute();

// Check for errors
const char* errorMessage;
if (device.getError(errorMessage) != oidn::Error::None)
  std::cout << "Error: " << errorMessage << std::endl;

};
}
// Create an Open Image Denoise device
REGISTER_DENOISE(Denoise, "denoise")
