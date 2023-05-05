//
//  dowelStylus.hpp
//  helloAgain
//
//  Created by Huiying Shen on 10/10/18.
//  Copyright © 2018 Huiying Shen. All rights reserved.
//

#ifndef dowelStylus_hpp
#define dowelStylus_hpp

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>
#include "util.h"
#include "markerArray.h"

using namespace std;
using namespace cv;


struct DowelStylus: public ArucoObj, public Marker3dGroup{
    MarkerArray markers;
    vector<int> anchorIds;
    
    
    int nRow;
    
//    RingBufMatx31f tipStore;

//	Point3f tip;
   
    DowelStylus(aruco::PREDEFINED_DICTIONARY_NAME dictName = aruco::DICT_4X4_250):ArucoObj(dictName),Marker3dGroup(){
        errMeanTol = 10;
        errMaxTol = 30;
        nIdMin = 2;
        imagePointsSave.resize(1); //started with non zero, so CamIO won't start with standing by.
    }
    void setParam(float id0, int nRow, float squareLength, float markerLength);
    void setXYZ1col(int col);
    void setXYZ() { for (int c=0; c<4; c++) setXYZ1col(c); }
//    void moveMarkers(float dz = -1.5){
//        for (int i=0; i< vMarker3d.size(); i++){
//            vMarker3d[i].translate(Point3f(0,0,dz*squareLength));
//        }
//    }
    void setTop(){
        Marker3d m3d(anchorIds[0]-1,markerLength);
        m3d.rotateX(180);
        m3d.translate(Point3f(0,0,-4.5*squareLength));
        vMarker3d.push_back(m3d);
    }
    void set4Cube(float scale = 0.5){
        nIdMin = 2;  //minimun markers detected
        vMarker3d.clear();
        squareLength = 0.101*scale;
        markerLength = 0.0845*scale;
        float l = squareLength;
        addOneCube(200, Point3f(0,    0,  -3*l));
        addOneCube(230, Point3f(0,    0,  -2*l));
        addOneCube(210, Point3f(0,    -l, -2*l));
        addOneCube(220, Point3f(-l,   0,  -2*l));
    }
    
    void oneCube(int topId = 199, float scale = 1, float dz = -1.5){
        nIdMin = 1; //minimun markers detected
        vMarker3d.clear();
        squareLength = 0.101*scale;
        markerLength = 0.0845*scale;
        addOneCube(topId, Point3f(0,0,dz*squareLength));
    }
    void halfCube(int id0=210, int id1=211, int id2=214){
        nIdMin = 2; //minimun markers detected
        vMarker3d.clear();
        squareLength = 0.0254*2;
        markerLength = 0.833*squareLength;
//        squareLength = 0.101*scale;
//        markerLength = 0.0845*scale;
        
        Marker3d m3d(id0,markerLength);
        m3d.rotateZ90();
        m3d.translate(Point3f(squareLength/2.0,squareLength/2.0,0));
        vMarker3d.push_back(m3d);
        
        
//        Marker3d m3d(mt);
        m3d.id_ = id1;
        m3d.translate(Point3f(-squareLength,0,0));
        m3d.rotateY(-90);
        vMarker3d.push_back(m3d);
        
        m3d.id_ = id2;
        m3d.translate(Point3f(0,-squareLength,0));
        m3d.rotateZ90();
        vMarker3d.push_back(m3d);

        
        for (int i=0; i< vMarker3d.size(); i++){
            vMarker3d[i].rotateX(-45);
        }
        float angle = atan2(1,1.414)/3.14159*180;
        for (int i=0; i< vMarker3d.size(); i++){
            vMarker3d[i].rotateY(angle);
        }

        for (int i=0; i< vMarker3d.size(); i++){
            vMarker3d[i].translate(Point3f(0,0,3.25*squareLength));
        }
    }
     

    void addOneCube(int topId, const Point3f &p3f);

	 
    Point3f transformStylusTip2local(const Mat &rvec,const Mat &tvec);
    Point3f transformStylusTip2local(const Marker3dGroup &base){
        return transformStylusTip2local(base.rvec,base.tvec);
    }
//    Point3f getTip(){
//        Matx31f out = Matx31f(tvec);
//        return Point3f(out(0, 0), out(1, 0), out(2, 0));
//    }
    
    Point2f tip2Image(const Mat &camMatrix, const Mat &distCoeffs){
        vector<Point3f> vP3f;
        vector<Point2f> vP2f;
        vP3f.push_back(Point3f(0,0,0));
        cv::projectPoints(vP3f, rvec, tvec, camMatrix, distCoeffs, vP2f);
        return vP2f[0];
    }

//    void drawTip(Mat &bgr, const Mat &camMatrix, const Mat &distCoeffs,float mult=1.0);
    void drawProjected(Mat &bgr, float mult=1.0);
    float getAxisAngle(){
        Mat rMat;
        Rodrigues(rvec, rMat);
        Mat z = (Mat_<double>(3,1) << 0,0,1);
        Matx31f o = Matx33f(rMat)*Matx31f(z);
//        cout<<"DowelStylus::getZAxis = "<<o;
        return acos(abs(o(2, 0)))/3.14159*180;
    }
    
    vector<Point2f> imagePointsSave;
    chrono::time_point<chrono::system_clock> time_point_imagePointsSave;
    
    static bool moved(const vector<Point2f> &vp1, const vector<Point2f> &vp2, float tol=3){
        if (vp1.size()!= vp2.size()) return true;
        for (int i=0; i<vp1.size(); i++)
            if (dist2(vp1[i],vp2[i]) > tol*tol)
                return true;
        return false;
    }
    
    long mSecLastMove(){
        if ( moved(imagePointsSave,imagePoints, 5.0) ){
            imagePointsSave = imagePoints;
            time_point_imagePointsSave = chrono::system_clock::now();
            return 0;
        }
        auto dt =  chrono::system_clock::now() - time_point_imagePointsSave;
        return chrono::duration_cast<chrono::milliseconds>(dt).count();
    }

    friend ostream& operator<<(ostream& os, const DowelStylus& anch);
    
    static cv::Ptr<DowelStylus> create(aruco::PREDEFINED_DICTIONARY_NAME dictName = aruco::DICT_5X5_250) {
        return cv::Ptr<DowelStylus>(new DowelStylus(dictName));
    }
};

#endif /* dowelStylus_hpp */
