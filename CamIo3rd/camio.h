//
//  camio.hpp
//  CamIO4iOS
//
//  Created by Huiying Shen on 1/29/18.
//  Copyright © 2018 Huiying Shen. All rights reserved.
//

#ifndef camio_hpp
#define camio_hpp

#include <iostream>
#include <fstream>
#include <string>
#include <opencv2/opencv.hpp>

#include "camIoObj.h"
#include "util.h"
#include "anchorBase.h"
#include "dowelStylus.h"

//struct StylusStateString{
//    String no_board;
//    String board_only;
//    String exploring;
//    String tts_speaking;
//    String pre_recording;
//    String startRecording;
//};

class CamIO {
    enum Action { NewRegion, Add2Region, SelectRegion, DeleteRegion, Exploring };

    cv::Ptr<aruco::Dictionary> dictionary;
    cv::Ptr<aruco::DetectorParameters> detectionParams;
    MarkerArray markers;
    SmoothedDetection det;

    
	Obj obj;
    //Ptr<RectBase> pRectBase;
//    cv::Ptr<SkiBoard> pSkiBoard;
    cv::Ptr<BasePlayground> pBase;
	cv::Ptr<DowelStylus> pStylus;
//    MarkerBase4 base4;
//    vector<int> stylus_ids;
    RingBufP3f bufP3f;
    RingBufInt bufInt;
    RingBufP2f bufP2f;
    
    Action action = Action::Exploring;

    bool _DEBUG = false;
    bool hasBase = false;
    bool isIntrinscSet = false;
    bool bStylus = false;
    bool isCaptured = false;
    bool isStandingBy = false;
    int nStandby = 0;
    float tolClose = 0.02;
    
    //StylusStateString ssString;

    chrono::time_point<chrono::system_clock> time_point;
    chrono::time_point<chrono::system_clock> lastStylusSeen;
    long dtPoseTol = 5000*1000;

    string stateString = "???";
    string stateStringSav = "???";
    string currentTime = "???";
public:
//    vector<Point2f> vFeatPnt;
//    RingBufP2f tipData;
//    bool isSavingTip = true;
    FeatureBase featBase;
public:
    int iState;
    int iStateSav = 1;
    bool isCalib = false;
    bool isDark = false;
    bool isBase4Active = true;
    bool invertColor = true;
public:
    static Mat_<double> camMatrix, distCoeffs;
    static Mat_<double> camRotMat;
public:

	CamIO(aruco::PREDEFINED_DICTIONARY_NAME dictName = aruco::DICT_4X4_250) :markers(dictName),
    pStylus(DowelStylus::create(dictName)),bufP3f(5),
        time_point(chrono::system_clock::now()),lastStylusSeen(chrono::system_clock::now()){
//        testDist2();
        
        camMatrix = Mat_<double>(3,3);
        distCoeffs = Mat_<double>(1,5);
        distCoeffs = 0;
        
//        pStylus->oneCube(199,0.75,-1.333);
        pStylus->halfCube();
        
//        pBase = BasePlayground::create(dictName,2*0.7*0.0254);
//        pBase->addAllMarkers();
        pBase = BasePlayground::create(dictName,2*0.833*0.0254);
        pBase->addAllMarkers2();
//        base4.init(249);
	}
    
    bool invert_color(){ invertColor = !invertColor; return invertColor;}
    
    void scaleModel(float scale){
        obj.scale(scale);
    }
    void resetFeatureBase() {
        hasBase = false;
        featBase.reset();
    }
    bool setFeatureBaseImagePoints(string dat){
        featBase.reset();
        vector<string> vStr = split(dat, '\n');
        if (vStr.size() != featBase.objectPoints.size())
            return false;
        for (int i=0; i<vStr.size(); i++){
            stringstream s(vStr[i]);
            Point2f p;
            s>>p.x>>p.y;
            featBase.tryAddImagePnt(p);
        }
        if (featBase.tryGetPose(camMatrix, distCoeffs) ){
            featBase.rvec.copyTo(pBase->rvec);
            featBase.tvec.copyTo(pBase->tvec);
//            pBase->rvec = featBase.rvec.clone();
//            pBase->tvec = featBase.tvec.clone();
        }
        return true;
    }

//    void updateRect(bool bBoard, Mat gray, int padding = 20);
    pair<int,Point3f>  clusteringStylus(const RingBufP3f &bufP3f);
    void setStylusCube(int iStylus){
        switch (iStylus){
            case 0:  // 2 in
                pStylus->halfCube();
                
                break;
            case 1:  // 3 in
//                pStylus->oneCube(199,0.5,-2.0);
                pStylus->oneCube(199,0.75,-1.333);
                break;
            default:
                break;
        }
    }
    void setFreezePose(bool freezePose){
        dtPoseTol = 5000*1000;
        if (freezePose) dtPoseTol = 1000*1000*1200;  
    }
    void clear() {obj.clear();}
    bool isCameraCovered() {return isDark;}
    bool isImageBlack(const Mat& gray, int thrsh = 20, float tol = 0.05){
        float nHigh = 0;
        unsigned char *input = (unsigned char*)(gray.data);
        for (int i = 0;i < gray.cols;i++){
            for (int j = 0;j < gray.rows;j++)
                if (input[gray.cols * j + i ] > thrsh)
                    nHigh += 1;
        }
        return nHigh/gray.cols/gray.rows < tol;
    }
    
    int newRegion(const string &regionString)
    {
        bool switchYZ = true;
        return obj.newRegion(regionString,switchYZ);
    }
    
    int newZone(const string &regionString)
    {
        bool switchYZ = true;
        return obj.newZone(regionString,switchYZ);
    }
    bool clearingYouAreHereBoundary(){
        int idFrom1 = obj.getIdByName("Long Ramp");
        int idFrom2 = obj.getIdByName("Adobe Creek wall");
        int idFrom3 = obj.getIdByName("Sidewalk to Main Bridge");
        int idCloseTo = obj.getIdByName("You are Here at the Magic Map");
        if (idFrom1 == -1 ||idFrom2 == -1 || idCloseTo == -1) return false;
        float distTol = 0.03;
        int cnt1 = obj.clearingBoundary(idFrom1, idCloseTo, distTol);
        int cnt2 = obj.clearingBoundary(idFrom2, idCloseTo, distTol);
        int cnt3 = obj.clearingBoundary(idFrom3, idCloseTo, distTol);
        return true;
    }
    
    bool changeRegionName(const string &namePlus){
        return obj.changeRegionName(namePlus);
    }
    
    string getHighestP3f(string name){
        int iRegion = obj.findRegionByName(name);
        return obj.getHighestP3f(iRegion);
    }
    
    int getRegionByName(const string &name){
        return obj.findRegionByName(name);
    }
    
    void newRegionWithXyz(const string &xyz){
        obj.newRegionWithXyz(xyz);
    }
    
    
    string getRegionNames(){
        return obj.getRegionNames();
    }
    void setCurrent(int i){
         obj.setCurrent(i);
     }
    
    bool isNewRegion(){ return action == Action::NewRegion;}
    bool isExploring(){ return action == Action::Exploring;}
    bool locationCaptured(){ return isCaptured; }
    
    void setNewRegion(){action = Action::NewRegion;}
    void setAdd2Region(){action = Action::Add2Region;}
    void setSelectRegion(){action = Action::SelectRegion;}
    bool isActionDone() const { return action==Action::Exploring;}
    bool isStylusVisible() const {return bStylus;}
    int getCurrentRegion() const {return obj.current;}
    bool setCurrentNameDescription(const string &name, const string &description){
        return obj.setCurrentNameDescription(name,description);
    }
    
    string getCurrentNameDescription(){
        return obj.getCurrentNameDescription();
    }

    bool setIntrinsic(string calibStr) {
        return ::setCameraCalib(calibStr, camMatrix, distCoeffs);
    }
    
    bool setCamMat3val(string str3val) {
//        cout<<"setCamMat3val, before "<<camMatrix<<endl;
        camMatrix = 0;
        stringstream ss(str3val);
        ss>>camMatrix.at<double>(0,0);
        ss>>camMatrix.at<double>(0,2);
        ss>>camMatrix.at<double>(1,2);
        camMatrix.at<double>(1,1) = camMatrix.at<double>(0,0);
        camMatrix.at<double>(2,2) = 1;
//        cout<<"setCamMat3val, after "<<camMatrix<<endl;
        return true;
    }

    bool loadObj(string objStr){ return true;}

    string getObjJson(){ return obj.toJson(); }
    
    void loadModel(string objJson){
        obj.clear();
        obj.setAllRegions(objJson);
        obj.checkReginNames();
    }
    
	void deleteCurrentRegion() { obj.deleteCurrentRegion(); }
     
    string getState(){ return stateString; }
    int getCurrentObjId(){ return obj.current;}
    
    static void setMatValue(cv::Mat_<double> &mat, int col, int row, float val){
        mat.at<double>(col,row) = val;
    }
    void drawTip(Mat &bgr, const Point3f &p3f);
    void do1timeStep(Mat& bgr);
    pair<int,std::string> setStatePair(pair<int,Point3f> out,Mat& bgr,float zAxisAngle);
    string checkForStandingBy(int nStep4Robust = 15);
    int procReadingStylus(Mat bgr, std::pair<int,float> near);
};

#endif /* camio_hpp */
