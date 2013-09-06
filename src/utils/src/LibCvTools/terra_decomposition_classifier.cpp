#include "terra_decomposition_classifier.h"

TerraDecomClassifier::TerraDecomClassifier(const float _threshold) :
    threshold(_threshold),
    use_max_prob(false),
    color_ext(false)
{
}

TerraDecomClassifier::~TerraDecomClassifier()
{
}

void TerraDecomClassifier::setClassifier(RandomForest::Ptr _classifier)
{
    classifier = _classifier;
}

void TerraDecomClassifier::setExtractor(cv_extraction::Extractor::Ptr _extractor)
{
    extractor  = _extractor;
}

void TerraDecomClassifier::setUseMaxProb(bool value)
{
    use_max_prob = value;
}

void TerraDecomClassifier::setUseColorExt(bool value)
{
    color_ext = value;
}

void TerraDecomClassifier::classify(const std::vector<Rect> rois, std::vector<bool> classification)
{
    std::vector<cv::Mat> descriptors;
    extractor->extract(image, rois, descriptors);

    for(int i = 0 ; i < rois.size() ; ++i) {
        cv::Mat &d = descriptors[i];

        if(d.rows > 1) {
            if(use_max_prob)
                classifier->predictClassProbMultiSampleMax(d, last_id, last_prob);
            else
                classifier->predictClassProbMultiSample(d, last_id, last_prob);
        } else {
            classifier->predictClassProb(d, last_id, last_prob);
        }

        classification.push_back(last_prob < threshold);
    }
}

bool TerraDecomClassifier::classify(const Rect &roi)
{
    /// EXTRACT
    cv::Mat descriptors;
    extractor->extract(image, roi, descriptors);

    /// PREDICT
    if(descriptors.empty()) {
        return false;
    }

    if(color_ext) {
        cv::Mat     img_roi(image, roi);
        cv::Vec2b   mean = cv_extraction::Extractor::extractMeanColorRGBYUV(img_roi);
        cv_extraction::Extractor::addColorExtension(descriptors, mean);
    }


    if(descriptors.type() != CV_32FC1) {
        descriptors.convertTo(descriptors, CV_32FC1);
    }

    if(descriptors.rows > 1) {
        if(use_max_prob)
            classifier->predictClassProbMultiSampleMax(descriptors, last_id, last_prob);
        else
            classifier->predictClassProbMultiSample(descriptors, last_id, last_prob);
    } else {
        classifier->predictClassProb(descriptors, last_id, last_prob);
    }

    return last_prob < threshold;

}

float TerraDecomClassifier::get_prob()
{
    return last_prob;
}

int TerraDecomClassifier::get_id()
{
    return last_id;
}

void TerraDecomClassifier::set_image(const Mat &_image)
{
    image = _image;
}