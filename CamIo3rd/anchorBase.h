#ifndef __ANCHORCUBE__
#define __ANCHORCUBE__

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>
#include "util.h"
#include "markerArray.h"

using namespace std;
using namespace cv;


struct BaseObject : public Marker3dGroup{
    int id1st;
    float markerLength;
    
	MarkerArray markers;
	vector<int> anchorIds;
    
	BaseObject(aruco::PREDEFINED_DICTIONARY_NAME dictName = aruco::DICT_4X4_250) :Marker3dGroup(),markers(dictName) {
        errMeanTol = 10;
        errMaxTol = 30;
    }
	void setIds(int nIds) {
		anchorIds.resize(nIds);  
		for (int i = 0; i < nIds; i++)
			anchorIds[i] = id1st + i;
	}
    
    void setParams(int id1st, float markerLength) {
        this->id1st = id1st;
        this->markerLength = markerLength;
    }
    
    virtual void setMarker3ds() = 0;
    
    void draw(Mat &bgr, float mult=1.0,Scalar color= Scalar(0, 0, 255)) const {
        if (convexHull.convexHull.size()==0 || imagePoints.size()==0) return;
        vector<vector<Point2f> > vc;
        vc.resize(imagePoints.size()/4);
        for (int i=0; i<vc.size(); i++){
            vc[i].resize(4);
            for (int j=0; j<4; j++){
                //cout<<"i, j = "<<i<<", "<<j<<endl;
                vc[i][j] = imagePoints[i*4+j]*mult;
            }
        }
        aruco::drawDetectedMarkers(bgr, vc,noArray(),color);
//
//        vector<vector<Point2f> > vProjP;
//        vProjP.resize(projectedPoints.size()/4);
//        for (int i=0; i<vProjP.size(); i++){
//            vProjP[i].resize(4);
//            for (int j=0; j<4; j++){
//                //cout<<"i, j = "<<i<<", "<<j<<endl;
//                vProjP[i][j] = projectedPoints[i*4+j]*mult;
//            }
//        }
//        aruco::drawDetectedMarkers(bgr, vProjP,noArray(),Scalar(255, 0, 0));

        for (int k=0; k<convexHull.convexHull.size(); k++){
            std::stringstream s;
            s << convexHull.convexHull[k].first;
            int x = convexHull.convexHull[k].second.x, y = convexHull.convexHull[k].second.y;
            cv::putText(bgr, s.str(), cv::Point(x, y), FONT_HERSHEY_SIMPLEX, 0.5, color);
        }
    }
    
};

struct PoseDetection{
    chrono::time_point<chrono::system_clock> time_point = chrono::system_clock::now();
    vector<Marker2d3d> vMarker;
    bool isSet = false;
//    long milliSec4Good = 5000; //pose data good before going stale
//    bool isGood() {
//        if (!isSet) return false;
//        long milli = (chrono::system_clock::now() - time_point).count()/1000;
//        if (milli > milliSec4Good)
//            return false;
//        else
//            return true;
//    }
    float minDist;
    Mat rvec, tvec;
    PoseDetection(){}
    
    static bool moved(const Marker2d3d &mi, const Marker2d3d &mj, float pixTol){
        int n = 0;
        for (int k=0; k<4; k++)
            if (cv::norm(mi.corners2d[k]-mj.corners2d[k]) < pixTol )
                n++;
        return n<4;
    }
    
    int notMoved(const vector<Marker2d3d> &vMarker, float pixTol = 1.99){  //
        int cnt = 0;
        for (int i=0; i<vMarker.size(); i++){
            const Marker2d3d &mi = vMarker[i];
            for (int j=0; j<this->vMarker.size(); j++){
                const Marker2d3d &mj = this->vMarker[j];
                if (mi.id_ == mj.id_  && !moved(mi,mj,pixTol))
                    cnt++;
            }
        }
        return cnt;  //number of marker not moved
    }
//    void resetTime(){time_point = chrono::system_clock::now();}
    void saveData(const vector<Marker2d3d> &vMarker, const Mat &rvec, const Mat &tvec){
        time_point = chrono::system_clock::now();
        this->vMarker = vMarker;
        rvec.copyTo(this->rvec);
        tvec.copyTo(this->tvec);
        isSet = true;
    }
//    void invalidate() {isSet = false;}
};

struct BasePlayground: public BaseObject{
    vector<Point3f> rect3;
    vector<Point2f> rectProjected;
    
    vector<int> id_;
    vector<vector<Point2f> > corners;
    
    PoseDetection pose;
    bool notMoved = false;
    
    BasePlayground(aruco::PREDEFINED_DICTIONARY_NAME dictName, float markerLength):BaseObject(dictName){
        this->markerLength = markerLength;
        float w = 29.75*0.0254, h = 23.25*0.0254;
        rect3.push_back(Point3f(0,0,0));
        rect3.push_back(Point3f(w,0,0));
        rect3.push_back(Point3f(w,h,0));
        rect3.push_back(Point3f(0,h,0));
    }
    void setMarker3ds(){}
    void addMarker(int id_, float x, float y){
        Marker3d m(id_,markerLength);
        m.translate(Point3f(x,y,0));
        vMarker3d.push_back(m);
    }
    void addAllMarkers(){
        addMarker(130,0.051, 0.025);
        addMarker(131,0.103, 0.052);
        addMarker(111,0.731, 0.025);
        addMarker(110,0.731, 0.076);
        addMarker(120,0.731, 0.564);
        addMarker(121,0.731, 0.512);
        addMarker(100,0.124, 0.473);
        addMarker(101,0.035, 0.503);
    }
    void addAllMarkers2(){
        addMarker(200,0.063, 0.025);
        addMarker(201,0.745, 0.025);
        addMarker(202,0.745, 0.575);
        addMarker(203,0.033, 0.505);
        
//        addMarker(200,0.025, -0.025);
//        addMarker(201,0.745, -0.025);
//        addMarker(202,0.745, 0.625);
//        addMarker(203,0.025, 0.625);
    }
    bool hasGoodSeparation(const vector<Marker2d3d> &vMarker, float minDistTol= 0.25){
        if (vMarker.size()!=4) return false;
        pose.minDist = 9999;
        for (int i=0; i<4; i++){
            const Point3f &p0 = vMarker[i].corners3d[0];
            const Point3f &p1 = vMarker[(i+1)%4].corners3d[0];
            float dist = cv::norm(p0-p1);
            pose.minDist = fmin(dist,pose.minDist);
        }
        bool b = pose.minDist>minDistTol;
        if (b == false) {
            //cout<<"minDist = "<<pose.minDist<<endl;
        }
        return b;
    }
    bool detect(const MarkerArray &markers, const Mat &camMatrix, const Mat &distCoeffs) {
//
        vector<Marker2d3d> vMarker;
        Marker3dGroup::getConvexHullMarkers(markers,vMarker);
        bool b = false, b1 = hasGoodSeparation(vMarker);
        if (b1) {
            b = Marker3dGroup::detect(vMarker,camMatrix, distCoeffs);
        }
        if (b && b1 ){
            pose.saveData(vMarker, rvec, tvec);
            cv::projectPoints(rect3, pose.rvec, pose.tvec, camMatrix, distCoeffs, rectProjected);
            return true;
        }
        if (!b ){  // no current detection
            notMoved = pose.notMoved(vMarker) >= 2;
//            if (pose.notMoved(vMarker) >= 3) pose.resetTime();
//            else pose.invalidate();  // moved and no current detection
        }
//        if (pose.isGood()){
//            cv::projectPoints(rect, pose.rvec, pose.tvec, camMatrix, distCoeffs, rectProjected);
//            return true;
//        }
        return false;
    }
    
    cv::Rect getRect(){
        float xMin = 9999, yMin = 9999, xMax = 0, yMax = 0;
        for (int i=0; i<4; i++){
            Point2f &p = rectProjected[i];
            xMin = min(xMin,p.x);
            yMin = min(yMin,p.x);
            xMax = max(xMin,p.x);
            yMax = max(yMin,p.x);
        }
        return cv::Rect((int)xMin,(int)yMin,(int)(xMax-xMin),(int)(yMax-yMin));
    }
    
    void draw(Mat &bgr, float mult=1.0,Scalar color= Scalar(0, 0, 255)) const {
        BaseObject::draw(bgr,mult,color);
        if (rectProjected.size() != 4) return;
        for (int i=0; i<4; i++){
            int i1 = (i + 1)%4;
            cv::line(bgr,rectProjected[i],rectProjected[i1],color);
        }
    }
    
    static cv::Ptr<BasePlayground> create(aruco::PREDEFINED_DICTIONARY_NAME dictName, float markerLength) {
        return cv::Ptr<BasePlayground>(new BasePlayground(dictName,markerLength));
    }
};

struct RectBase : public BaseObject {
    float dx, dy;
	RectBase(aruco::PREDEFINED_DICTIONARY_NAME dictName = aruco::DICT_4X4_250) :BaseObject(dictName) {}

    void setParams(int id1st, float markerLength, float dx, float dy){
        BaseObject::setParams(id1st,markerLength);
        this->dx = dx;
        this->dy = dy;
    }
    
    void setMarker3ds();
	void setMarker3ds(int id1st, float markerLength, float dx, float dy){
        setParams(id1st,markerLength,dx,dy);
        setIds(8);
        setMarker3ds();
    }

	static cv::Ptr<RectBase> create(aruco::PREDEFINED_DICTIONARY_NAME dictName = aruco::DICT_4X4_250) {
		return cv::Ptr<RectBase>(new RectBase(dictName));
	}
};

struct SkiBoard: public BaseObject {
    float gap;
    int nx, ny;
    
    
    SkiBoard(aruco::PREDEFINED_DICTIONARY_NAME dictName = aruco::DICT_5X5_250) :BaseObject(dictName) {}
    void setParams(int id1st, float markerLength, float gap, int nx, int ny){
        BaseObject::setParams(id1st,markerLength);
        this->gap = gap;
        this->nx = nx;
        this->ny = ny;
    }
    void setMarker3ds();
    void setMarker3ds(int id1st, float markerLength, float gap2markerLength=0.2, int nx=8, int ny=12){
        setParams(id1st,markerLength,markerLength*gap2markerLength, nx, ny);
        setIds(nx*ny);
        setMarker3ds();
    }
    
    static cv::Ptr<SkiBoard> create(aruco::PREDEFINED_DICTIONARY_NAME dictName = aruco::DICT_5X5_250) {
        return cv::Ptr<SkiBoard>(new SkiBoard(dictName));
    }
};
 
struct CamCalib{
    SkiBoard board;
    MarkerArray markerArray;
    
    vector<vector<Point2f> > vImagePoints;
    vector<vector<Point3f> > vObjectPoints;
    vector<Mat> rvecs;
    vector<Mat> tvecs;
    
    Mat camMatrix;
    Mat distCoeffs;
    cv::Size size;
    
    CamCalib(aruco::PREDEFINED_DICTIONARY_NAME dictName = aruco::DICT_5X5_250):board(dictName),markerArray(dictName){
        board.setMarker3ds(0,0.0215);
    }
    
    void clear(){vImagePoints.clear(); vObjectPoints.clear();}
    
    bool tryAddFrame(const Mat &gray);
    
    string calib();
    
    void getReprojErrorOneFrame(const vector<Point2f> &imagePoints, const vector<Point3f> &objectPoints, const Mat &rvec, const Mat &tvec, float &errSum, float &errMax);
};

struct FeatureBase: SolvePnp{
    vector<string> names;
    RingBufP2f tipData;
    
    bool hasPose = false;
    float tolMean = 15.0, tolMax = 25.0; // reprojection error tol, in pixels
    FeatureBase():tipData(30){
        setFeatureP3f();
    }
    void setFeatureP3f();
    void reset(){
        imagePoints.clear();
        hasPose = false;
    }
    string name(int i){
        if (i<0 || i>= names.size()) {
            cout<<"FeatureBase::name(), i = "<<i<<endl;
            return "???";
        }
        return names[i];
    }
    int nextLandmark(){
        if (imagePoints.size() == objectPoints.size())
            return -1;
        return imagePoints.size();
    }
    
    void addTip(const Point2f &tipProj){
        tipData.add(tipProj);
    }
    bool tryAddImagePnt();
    bool tryAddImagePnt(const Point2f &p2f, float tol = 50);
    void draw(Mat &bgr, Scalar color=Scalar(0,0,200));
    void drawAxis(Mat &bgr, const Mat &camMatrix, const Mat &distCoeffs, float l = 0.01){
        aruco::drawAxis(bgr, camMatrix, distCoeffs, rvec, tvec, l);
    }
    bool tryGetPose(const Mat &camMatrix, const Mat &distCoeffs);
};

#endif

