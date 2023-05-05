//
//  anchorCube.cpp
//  helloAgain
//
//  Created by Huiying Shen on 10/2/18.
//  Copyright © 2018 Huiying Shen. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>
#include "util.h"
#include "markerArray.h"
#include "anchorBase.h"

using namespace std;
using namespace cv;



ostream& operator<<(ostream& os, const Marker3d& m)
{
    os << m.id_<<", "<<m.length<<endl;
    for (int i=0; i<m.vP3f.size(); i++)
        os<<m.vP3f[i]<<endl;
    
    return os;
}

void RectBase::setMarker3ds(){
    vMarker3d.clear();
    //lower, left
    Marker3d m(id1st, markerLength);
    push_back(m);
    //lower, right
    m.init(id1st + 1, markerLength);
    m.translate(Point3f(dx, 0, 0));
    push_back(m);
    //upper,right
    m.init(id1st + 2, markerLength);
    m.translate(Point3f(dx, dy, 0));
    push_back(m);
    //upper,left
    m.init(id1st + 3, markerLength);
    m.translate(Point3f(0, dy, 0));
    push_back(m);
    
    //lower
    m.init(id1st + 4, markerLength);
    m.translate(Point3f(dx/2, 0, 0));
    push_back(m);
    //right
    m.init(id1st + 5, markerLength);
    m.translate(Point3f(dx/2, dy/2, 0));
    push_back(m);
    //upper
    m.init(id1st + 6, markerLength);
    m.translate(Point3f(dx/2, dy, 0));
    push_back(m);
    //left
    m.init(id1st + 7, markerLength);
    m.translate(Point3f(0, dy/2, 0));
    push_back(m);
}

void SkiBoard::setMarker3ds(){
    vMarker3d.clear();
    for (int iy=0; iy<ny; iy++){
        float y = iy*(markerLength + gap);
        for (int ix=0; ix<nx; ix++){
            float x = ix*(markerLength + gap);
            Marker3d m(anchorIds[iy*nx + ix], markerLength);
            m.translate(Point3f(x, y, 0));
            push_back(m);
        }
    }
}
    
bool CamCalib::tryAddFrame(const Mat &gray){
    markerArray.detect(gray);
    vector<Point2f> imagePoints;
    vector<Point3f> objectPoints;
    board.fillterMarkers(markerArray, imagePoints, objectPoints);
     
    if (imagePoints.size()<16) return false;
    vImagePoints.push_back(imagePoints);
    vObjectPoints.push_back(objectPoints);
    size = gray.size();
    return true;
}

string CamCalib::calib(){
    int flags = CALIB_FIX_PRINCIPAL_POINT|CALIB_FIX_ASPECT_RATIO|CALIB_ZERO_TANGENT_DIST;
    //TermCriteria criteria = TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 30, DBL_EPSILON);
    
    calibrateCamera(vObjectPoints, vImagePoints, size, camMatrix, distCoeffs, rvecs, tvecs,flags);
    int nCorners = 0;
    float errSum = 0, errMax = 0;
    for (int i=0; i<vImagePoints.size(); i++){
        getReprojErrorOneFrame(vImagePoints[i],vObjectPoints[i],rvecs[i],tvecs[i],errSum,errMax);
        nCorners += vObjectPoints[i].size();
    }
    
    cout<<"errMean, errMax = "<<errSum/nCorners<<", "<<errMax<<endl;
    stringstream ss;
    ss<<"errMean, errMax = "<<errSum/nCorners<<", "<<errMax<<endl;
    ss<<"imgSize = "<<size<<endl;
    ss<<"camMatrix = "<<camMatrix<<endl;
    ss<<"distCoeffs ="<<distCoeffs<<endl;
    return ss.str();
}

void CamCalib::getReprojErrorOneFrame(const vector<Point2f> &imagePoints, const vector<Point3f> &objectPoints, const Mat &rvec, const Mat &tvec, float &errSum, float &errMax){
    
    vector<Point2f> projectedPoints;
    cv::projectPoints(objectPoints, rvec, tvec, camMatrix, distCoeffs, projectedPoints);

    for (int i=0; i<imagePoints.size(); i++){
        float err = cv::norm(imagePoints[i] - projectedPoints[i]);
        errSum += err;
        errMax = std::fmax(err,errMax);
    }
}

void FeatureBase::setFeatureP3f(){
//    string dat = "\
//    Kinder Bells, 0.0639_0.112_0.0324\n\
//    Rocking Horse, 0.108_0.18_0.012\n\
//    You are here at the Magic Map, 0.711_0.219_0.0164\n\
//    Playhouse, 0.564_0.24_0.0718\n\
//    Umbrella Pole, 0.418_0.372_0.0322\n\
//    Exercise Bike, 0.22_0.498_0.0162\
//    ";
    string dat = "\
    upper left, 0.00_0.00_0.00\n\
    lower left, 0.760_0.00_0.00\n\
    lower right, 0.760_0.590_0.00\n\
    upper right, 0.00_0.590_0.00\n\
    You are Here at the Magic Map, 0.711_0.219_0.0164\n\
    Exercise Bike, 0.22_0.498_0.0162\
    ";
    vector<string> vStr = split(dat, '\n');
    float scale = 1.016;
    for (int i=0; i<vStr.size(); i++){
        vector<string> v2 = split(vStr[i], ',');
        replaceAll(v2[1],"_"," ");
        stringstream ss(v2[1]);
        Point3f p3f;
        ss>>p3f.x>>p3f.y>>p3f.z;
        p3f.x *= scale;
        p3f.y *= scale;
        objectPoints.push_back(p3f);
        names.push_back(v2[0]);
    }
}
bool FeatureBase::tryAddImagePnt(){
    if (imagePoints.size() == objectPoints.size())
        return false;
    bool b1 = tipData.isFresh();
    bool b2 = tipData.isTight();
    cout<<"b1, b2 = "<<b1<<", "<<b2<<endl;
    if (b1 && b2)
        return tryAddImagePnt(tipData.centroid);
    
    return false;
}

bool FeatureBase::tryAddImagePnt(const Point2f &p2f, float tol){
    for (int i=0; i<imagePoints.size(); i++){
        if (dist2<Point2f>(imagePoints[i],p2f) < tol*tol)
            return false;
    }
    imagePoints.push_back(p2f);
    cout<<"FeatureBase::tryAddImagePnt(), added, p2f = "<<p2f<<endl;
    return true;
}

void FeatureBase::draw(Mat &bgr, Scalar color
                       ){
    for (int i=0; i<imagePoints.size(); i++){
        cv::Point p = imagePoints[i];
        p += cv::Point(10,5);
        stringstream ss;
        ss<<'_'<<i;
        cv::putText(bgr,ss.str(), p, FONT_HERSHEY_SIMPLEX, 1.0, color);
        cv::circle(bgr, imagePoints[i],5,color,3);
    }
}
bool FeatureBase::tryGetPose(const Mat &camMatrix, const Mat &distCoeffs){
    if (hasPose) return false;
    if (imagePoints.size() != objectPoints.size())
        return false;
    solve(camMatrix, distCoeffs);

    cout<<"vFeatureP3f = \n"<<objectPoints<<endl;
    cout<<"vImageP2f = \n"<<imagePoints<<endl;
    cout<<"camMatrix = \n"<<camMatrix<<endl;
    cout<<"distCoeffs = \n"<<distCoeffs<<endl;
    cout<<"rvec = \n"<<rvec<<endl;
    cout<<"tvec = \n"<<tvec<<endl;

    hasPose = errMean<tolMean && errMax<tolMax;
    cout<<"FeatureBase::tryGetPose(): hasPose, err = "<<hasPose<<", "<<errMean<<", "<<errMax<<endl;
    return hasPose;
}
