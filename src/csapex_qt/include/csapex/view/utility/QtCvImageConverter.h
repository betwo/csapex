#ifndef IMAGE_CONVERTER_H
#define IMAGE_CONVERTER_H

/// SYSTEM
#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <stdexcept>
#include <QImage>

namespace QtCvImageConverter
{
/**
 * @brief The QTQTRGBConverter struct is the default rgb mapper
 */
struct QTRGBConverter
{
    static inline unsigned int rgb(int r, int g, int b)  // set RGB value
    {
        return (0xffu << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
    }

    static inline unsigned int rgba(int r, int g, int b, int a)  // set RGBA value
    {
        return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
    }
};

static inline int getRed(unsigned rgb)
{
    return ((rgb >> 16) & 0xff);
}

static inline int getGreen(unsigned rgb)
{
    return ((rgb >> 8) & 0xff);
}

static inline int getBlue(unsigned rgb)
{
    return (rgb & 0xff);
}

static inline int getAlpha(unsigned rgb)
{
    return rgb >> 24;
}

/**
 * @brief The Converter class is a helper class template for converting QImages to cv images
 * @note It is a template so that Qt doesn't have to be linked to this library
 */
class Converter
{
private:
    /**
     * @brief Tools
     */
    Converter();

public:
    /**
     * @brief mat2QImage converts an OpenCV image to a Qt image
     * @param mat OpenCV image
     * @return Qt image
     */
    static cv::Mat QImage2Mat(const QImage& img)
    {
        int h = img.height();
        int w = img.width();
        cv::Mat mat(h, w, CV_8UC3);

        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                unsigned source = img.pixel(x, y);
                cv::Vec3b& target = mat.at<cv::Vec3b>(y, x);
                target[0] = getBlue(source);
                target[1] = getGreen(source);
                target[2] = getRed(source);
            }
        }
        return mat;
    }

    /**
     * @brief mat2QImage converts an OpenCV image to a Qt image
     * @param mat OpenCV image
     * @return Qt image
     */
    static QImage mat2QImage(const cv::Mat& mat, bool bgr = true)
    {
        switch (mat.depth()) {
            case CV_8U:
                return mat2QImageImpl<unsigned char>(mat, bgr);
            case (int)CV_8S:
                return mat2QImageImpl<char>(mat, bgr);
            case CV_16U:
                return mat2QImageImpl<unsigned short>(mat, bgr);
            case (int)CV_16S:
                return mat2QImageImpl<short>(mat, bgr);
            case (int)CV_32S:
                return mat2QImageImpl<int>(mat, bgr);
            case CV_32F:
                return mat2QImageImpl<float>(mat, bgr);
            case CV_64F:
                return mat2QImageImpl<double>(mat, bgr);
            default:
                throw std::runtime_error("cannot convert image, unknown type");
        }
    }

private:
    template <typename T>
    static QImage mat2QImageImpl(const cv::Mat& mat, bool bgr)
    {
        const int bytes_per_pixel = sizeof(T);
        if (bytes_per_pixel == 1) {
            return mat2QImageImplNoCompression<T>(mat, bgr);
        } else {
            return mat2QImageImplCompression<T>(mat, bgr);
        }
    }

    template <typename T>
    static QImage mat2QImageImplNoCompression(const cv::Mat& mat, bool bgr)
    {
        int h = mat.rows;
        int w = mat.cols;
        int channels = mat.channels();
        QImage qimg(w, h, QImage::Format_ARGB32);
        T* data = (T*)(mat.data);

        assert(w > 0);
        assert(h > 0);
        assert(data != NULL);

        for (int y = 0; y < h; y++, data += mat.step1()) {
            for (int x = 0; x < w; x++) {
                char r = 0, g = 0, b = 0, a = 0;
                if (channels == 1) {
                    r = data[x * channels];
                    g = r;
                    b = r;
                } else if (channels == 3 || channels == 4) {
                    r = data[x * channels + 2];
                    g = data[x * channels + 1];
                    b = data[x * channels];
                }

                if (channels == 4) {
                    a = data[x * channels + 3];
                    qimg.setPixel(x, y, QTRGBConverter::rgba(r, g, b, a));
                } else {
                    qimg.setPixel(x, y, QTRGBConverter::rgb(r, g, b));
                }
            }
        }
        return qimg;
    }

    template <typename T>
    static QImage mat2QImageImplCompression(const cv::Mat& mat, bool bgr)
    {
        int h = mat.rows;
        int w = mat.cols;
        int channels = mat.channels();

        QImage qimg(w, h, QImage::Format_ARGB32);
        const T* data = reinterpret_cast<const T*>(mat.data);

        const double factor = 1.0 / std::pow(2, (sizeof(T) - 1) * 8 - 2);

        assert(w > 0);
        assert(h > 0);
        assert(data != NULL);

        int offset_r = 2;
        int offset_g = 1;
        int offset_b = 0;

        if (!bgr) {
            offset_r = 0;
            offset_b = 2;
        }

        for (int y = 0; y < h; y++, data += mat.step1()) {
            for (int x = 0; x < w; x++) {
                char r = 0, g = 0, b = 0, a = 0;
                if (channels == 1) {
                    const auto raw = data[x * channels];
                    r = raw * factor;
                    g = r;
                    b = r;
                } else if (channels == 3 || channels == 4) {
                    r = data[x * channels + offset_r] * factor;
                    g = data[x * channels + offset_g] * factor;
                    b = data[x * channels + offset_b] * factor;
                }

                if (channels == 4) {
                    a = data[x * channels + 3];
                    qimg.setPixel(x, y, QTRGBConverter::rgba(r, g, b, a));
                } else {
                    qimg.setPixel(x, y, QTRGBConverter::rgb(r, g, b));
                }
            }
        }
        return qimg;
    }
};

}  // namespace QtCvImageConverter

#endif  // IMAGE_CONVERTER_H
