#include "terra_batch_trainer.h"
#include <computation/cmp_extraction.hpp>
#include <computation/yaml.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>

TerraBatchTrainer::TerraBatchTrainer(const std::string &path) :
    EXTRACTION_PATH("extraction.yaml"),
    CLASSIFIER_PATH("classifier.yaml"),
    FOREST_PATH("forest.yaml"),
    roi_lib_path_(path)
{
}

void TerraBatchTrainer::run()
{
    std::ifstream in(roi_lib_path_.c_str());
    if(!in.is_open()) {
        std::cerr << "Couldn't open file !" << std::endl;
        return;
    }

    std::cout << "============= LOAD  =============" << std::endl;
    read(in);
    in.close();
    std::cout << "============= EXTR. =============" << std::endl;
    extract();
    std::cout << "============= TRAIN =============" << std::endl;
    train();
    std::cout << "============= WRITE =============" << std::endl;
    std::ofstream out(CLASSIFIER_PATH.c_str());
    if(!out.is_open()) {
        std::cerr << "Couldn't write file !" << std::endl;
        return;
    }

    write(out);
    out.close();

}

void TerraBatchTrainer::read(std::ifstream &in)
{
    cv_extraction::FeatureExtractor::Ptr feat(new cv_extraction::FeatureExtractor);
    cv_extraction::PatternExtractor::Ptr patt(new cv_extraction::PatternExtractor);
    try {
        YAML::Parser parser(in);
        YAML::Node   document;
        parser.GetNextDocument(document);

        Extractor::read(document, extractor_, extracto_params_, keypoint_params_);

        /// FOREST PARAMS
        forest_params_.read(document);

        /// PATHS TO THE FILES
        const YAML::Node &paths = document["ROI_FILES"];
        CMPYAML::readSequence<std::string>(paths.begin(), paths.end() , buf_roi_paths_);

        /// BUFFER CLASSES
        const YAML::Node &classes = document["CLASSES"];
        readClasses(classes.begin(), classes.end());

        /// BUFFER COLORS
        const YAML::Node &colors = document["CLASSES_PALETTE"];
        CMPYAML::readSequence<int>(colors.begin(), colors.end(), buf_colors_);

        std::cout << "Loaded settings and roi file pathes!" << std::endl;
    } catch (YAML::Exception e) {
        std::cerr << "Problems reading preferences : '" << e.what() <<"' !" << std::endl;
    }
}

void TerraBatchTrainer::write(std::ofstream &out)
{
    std::ifstream in(FOREST_PATH.c_str());
    if(!in.is_open()) {
        std::cerr << "Resulting forest couldn't be opened!" << std::endl;
        return;
    }

    YAML::Emitter emitter;
    emitter << YAML::BeginMap;
    emitter << YAML::Key << "CLASSES" << YAML::Value;
    writeClasses(emitter);
    emitter << YAML::Key << "CLASSES_PALETTE" << YAML::Value;
    CMPYAML::writeSequence<int>(emitter, buf_colors_);
    emitter << YAML::Key << "CLASSIFIER" << YAML::Value;
    emitter << YAML::BeginMap;
    keypoint_params_.write(emitter);
    extracto_params_->write(emitter);
    emitter << YAML::Key << "DATA" << YAML::Value;
    std::stringstream buf;
    buf << in.rdbuf();
    emitter << buf.str();
    emitter << YAML::EndMap;
    out << emitter.c_str();
    in.close();
}

void TerraBatchTrainer::extract()
{
    std::ofstream out(EXTRACTION_PATH.c_str());
    if(!out.is_open()) {
        std::cerr << "Couldn't not write extraction file!" << std::endl;
        return;
    }

    YAML::Emitter emitter;
    emitter << YAML::BeginMap;
    emitter << YAML::Key << "data" << YAML::Value;
    emitter << YAML::BeginSeq;
    for(VecStr::iterator it = buf_roi_paths_.begin() ; it != buf_roi_paths_.end() ; it++) {
        extractROIS(*it, emitter);
    }
    emitter << YAML::EndSeq;
    emitter << YAML::EndMap;
    out << emitter.c_str();
    out.close();
}

void TerraBatchTrainer::extractROIS(const std::string &path, YAML::Emitter &emitter)
{
    std::ifstream in(path.c_str());
    if(!in.is_open()) {
        std::cerr << "Couldn't open file '" << path << "' !" << std::endl;
        return;
    }

    std::vector<cv_roi::TerraROI> terra_rois;
    cv::Mat image;

    try {
        YAML::Parser p(in);
        YAML::Node   doc;
        p.GetNextDocument(doc);
        std::string image_path;
        doc["IMAGE"] >> image_path;
        image = cv::imread(image_path);

        const YAML::Node &rois = doc["ROIS"];
        for(YAML::Iterator it = rois.begin() ; it != rois.end() ; it++) {
            cv_roi::TerraROI roi;
            (*it)["class"] >> roi.id.id;
            (*it)["x"]     >> roi.roi.rect.x;
            (*it)["y"]     >> roi.roi.rect.y;
            (*it)["w"]     >> roi.roi.rect.width;
            (*it)["h"]     >> roi.roi.rect.height;
            terra_rois.push_back(roi);
        }

    } catch(YAML::Exception e) {
        std::cerr << "Error reading document '" << e.what() << "' !" << std::endl;
        return;
    }

    if(image.empty()) {
        std::cerr << "Image couldn't be opened!" << std::endl;
        return;
    }


    CMPExtraction::extractToYAML(emitter,image, extractor_, terra_rois);
}

void TerraBatchTrainer::train()
{
    CMPRandomForestExt::Ptr random_forest(new CMPRandomForestExt);
    random_forest->setParams(forest_params_);

    bool trained = random_forest->trainFromData(EXTRACTION_PATH);
    if(trained) {
        try {
            random_forest->save(FOREST_PATH);
        } catch (cv::Exception e) {
            std::cerr << "ERROR : " << e.what() << std::endl;
        }

    } else {
        std::cerr << "Couldn't train classifier!" << std::endl;
    }
}

void TerraBatchTrainer::readClasses(const YAML::Iterator &begin, const YAML::Iterator &end)
{

    for(YAML::Iterator it = begin ; it != end ; it++) {
        int class_id;
        int color;
        std::string info;

        (*it)["id"] >> class_id;
        (*it)["color"] >> color;
        (*it)["info"]  >> info;

        buf_classes_.push_back(class_id);
        buf_classes_colors_.push_back(color);
        buf_classes_infos_.push_back(info);
    }
}

void TerraBatchTrainer::writeClasses(YAML::Emitter &emitter)
{
    emitter << YAML::BeginSeq;
    for(int i = 0 ; i < buf_classes_.size() ; i++) {
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "id"   << YAML::Value << buf_classes_[i];
        emitter << YAML::Key << "color"<< YAML::Value << buf_classes_colors_[i];
        emitter << YAML::Key << "info" << YAML::Value << buf_classes_infos_[i];
        emitter << YAML::EndMap;
    }
    emitter << YAML::EndSeq;
}

int main(int argc, char *argv[])
{
    if(argc != 2) {
        std::cerr << "Wrong number of arguments - execute 'terraBatchtrainer <file>.yaml" << std::endl;
        return 1;
    }

    TerraBatchTrainer batch(argv[1]);
    batch.run();
    return 0;
}
