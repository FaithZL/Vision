////
//// Created by Zero on 2023/7/3.
////
//
//#include "baker.h"
//
//namespace vision {
//
//void Baker::allocate(ocarina::uint buffer_size, ocarina::Device &device) noexcept {
//    _positions = device.create_buffer<float4>(buffer_size);
//    _normals = device.create_buffer<float4>(buffer_size);
//    _radiance = device.create_buffer<float4>(buffer_size);
//}
//
//CommandList Baker::clear() noexcept {
//    CommandList ret;
//    ret << _positions.clear();
//    ret << _normals.clear();
//    ret << _radiance.clear();
//    ret << [&] { _pixel_num.clear(); };
//    return ret;
//}
//
//uint Baker::pixel_num() const noexcept {
//    return std::accumulate(_pixel_num.begin(), _pixel_num.end(), 0u);
//}
//
//CommandList Baker::append_buffer(const Buffer<ocarina::float4> &normals,
//                                 const Buffer<ocarina::float4> &positions) noexcept {
//    CommandList ret;
//    ret << _positions.copy_from(positions, 0);
//    ret << _normals.copy_from(positions, 0);
//    ret << [size = normals.size(), this] { _pixel_num.push_back(size); };
//    return ret;
//}
//
//BufferDownloadCommand *Baker::download_radiance(void *ptr, ocarina::uint offset) const noexcept {
//    return _radiance.download(ptr, offset);
//}
//
//}// namespace vision