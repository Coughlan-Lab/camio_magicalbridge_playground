//
//  dowelStylus.cpp
//  helloAgain
//
//  Created by Huiying Shen on 10/10/18.
//  Copyright © 2018 Huiying Shen. All rights reserved.
//

#include "camIoObj.h"
#include "dowelStylus.h"
#include "camio.h"

//ostream& operator<<(ostream& os, const DowelStylus& a){
//    os<<"DowelStylus: "<<a.squareLength<<", "<<a.markerLength<<", "<<a.nRow<<endl;
//    for (Marker3d m: a.vMarker3d)
//        os<<m;
//    return os;
//}

void DowelStylus::setParam(float id0, int nRow, float squareLength, float markerLength){
    this->nRow = nRow;
    anchorIds.resize(nRow*4);
    for (int j=0; j<4; j++){
        for (int i=0; i<nRow; i++){
            int k = j*nRow + i;
            int k1 = j*10 + i;
            anchorIds[k] = id0 + k1;
        }
    }
    this->markerLength = markerLength;
    this->squareLength = squareLength;
}

void DowelStylus::setXYZ1col(int col){
    Marker3d m3d(anchorIds[col],markerLength);
    m3d.rotateX(90);
    m3d.translate(Point3f(0,-squareLength/2.0,-squareLength/2.0));
    
    for (int j=0; j<col; j++)
        m3d.rotateZ90();
    for (int i=0; i<nRow; i++){
        m3d.id_ = anchorIds[col*nRow+i];
        m3d.translate(Point3f(0,0,-squareLength));
        vMarker3d.push_back(m3d);
    }
}
//void DowelStylus::oneCube(int topId, float scale , float dz ){
//    vMarker3d.clear();
//    squareLength = 0.101*scale;
//    markerLength = 0.0845*scale;
//    // the top face
//    Marker3d mt(topId,markerLength);
//    mt.rotateX(180);
//    mt.translate(Point3f(0,0,-squareLength));
//    vMarker3d.push_back(mt);
//
//    // 1st face
//    Marker3d m3d(topId+1,markerLength);
//    m3d.rotateX(90);
//    m3d.translate(Point3f(0,-squareLength/2.0,-squareLength/2.0));
//    vMarker3d.push_back(m3d);
//    // the next 3 faces
//    for (int i=0; i<3; i++){
//        m3d.id_++;
//        m3d.rotateZ90();
//        vMarker3d.push_back(m3d);
//    }
//
//    // move the cude back a little
//    moveMarkers(dz);
//}
void DowelStylus::addOneCube(int topId , const Point3f &p3f ){
    // the top face
    Marker3d mt(topId,markerLength);
    mt.rotateX(180);
    mt.translate(Point3f(0,0,-squareLength));
    vMarker3d.push_back(mt);

    // 1st face
    Marker3d m3d(topId+1,markerLength);
    m3d.rotateX(90);
    m3d.translate(Point3f(0,-squareLength/2.0,-squareLength/2.0));
    vMarker3d.push_back(m3d);
    // the next 3 faces
    for (int i=0; i<3; i++){
        m3d.id_++;
        m3d.rotateZ90();
        vMarker3d.push_back(m3d);
    }

    for (int i=0; i< 5; i++){
        int k = vMarker3d.size() - i - 1;
        vMarker3d[k].translate(p3f);
    }
}


Point3f DowelStylus::transformStylusTip2local(const Mat &rvec,const Mat &tvec) {
    Mat tip2Base = this->tvec - tvec;
    Mat rMat;
    Rodrigues(rvec, rMat);
    rMat = rMat.t();
    Matx31f out = Matx33f(rMat)*Matx31f(tip2Base);
    return Point3f(out(0, 0), out(1, 0), out(2, 0));
}

void DowelStylus::drawProjected(Mat &bgr, float mult){
    for (int i=0; i<projectedPoints.size(); i+=4){
        for (int k=0; k<4; k++){
            Point p1(projectedPoints[i+k]*mult);
            Point p2(projectedPoints[i+(k+1)%4]*mult);
            cv::line(bgr, p1, p2, Scalar(128, 0, 255),2,4);
        }
    }
}
