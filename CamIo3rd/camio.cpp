//
//  camio.cpp
//  CamIO4iOS
//
//  Created by Huiying Shen on 1/29/18.
//  Copyright © 2018 Huiying Shen. All rights reserved.
//

#include <math.h>
#include <chrono>
#include <thread>

#include "util.h"
#include "camio.h"

Mat_<double> CamIO::camMatrix;
Mat_<double> CamIO::distCoeffs;
Mat_<double> CamIO::camRotMat;


string getTimeString(){
    auto t0 = std::time(nullptr);
    auto tm = *std::localtime(&t0);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%m/%d/%Y %H:%M:%S");
    return oss.str();
}

void CamIO::do1timeStep(Mat& bgr){
    if (invertColor) cv::bitwise_not(bgr, bgr);
    // yield some cpu time
    long tSleep = 5;
    if (isStandingBy) tSleep = 200;
    std::this_thread::sleep_for(std::chrono::milliseconds(tSleep));
    
    Mat gray;
    cvtColor(bgr, gray, CV_BGR2GRAY);
    
    isDark = isImageBlack(gray);
    markers.detect(gray);
    markers.draw(bgr);
    int fontFace=FONT_HERSHEY_DUPLEX;
    float fontScale=1.0;
    Scalar color=Scalar(100,100,0);
    
    Point3f p3f_cur(-10.1,0,0);  // default: no board
    stateString = "";
 
    bStylus = pStylus->detect(markers, camMatrix, distCoeffs);
    //cout<<"stylus_ids[i], markers.size() = "<< stylus_ids[i]<<", "<<markers.size()<<endl;
    float zAxisAngle = 90;
    if (bStylus) {
        pStylus->drawDetectErrors(bgr, Point(200,60),fontFace,fontScale,color);
        pStylus->draw_tvec(bgr, Point(650,60),fontFace,fontScale,Scalar(250,0,0));
        pStylus->drawAxis(bgr, camMatrix, distCoeffs,0.05);
        markers.draw(bgr);
        pStylus->drawProjected(bgr);
        zAxisAngle = pStylus->getAxisAngle();
//        cout<<"zAxisAngle = "<<zAxisAngle<<endl;
        lastStylusSeen = chrono::system_clock::now();
        if (bufP2f.milli_sec_2_newest_update() > 1000){
            Point2f p2f = pStylus->tip2Image(camMatrix, distCoeffs);
            bufP2f.add(p2f);
        }
    }
    
    featBase.draw(bgr);
    if (featBase.hasPose) {
        featBase.drawAxis(bgr, camMatrix, distCoeffs,0.05);
        if (_DEBUG) obj.draw(bgr, featBase.rvec, featBase.tvec, camMatrix, distCoeffs);
        cv::Rodrigues(featBase.rvec,CamIO::camRotMat);
//        featBase.rvec.copyTo(pBase->rvec)
//        featBase.tvec.copyTo(pBase->tvec)
    }
    
    if (featBase.hasPose && bStylus ){
        p3f_cur = pStylus->transformStylusTip2local(featBase.rvec,featBase.tvec);
    }
    auto t =  chrono::system_clock::now();
    long dt = chrono::duration_cast<chrono::milliseconds>(t - time_point).count();
    time_point = t;
    std::stringstream s, s1;
    s << "FPS = " << std::setprecision(3) << 1000/dt;
    cv::putText(bgr, s.str(), Point(200,90), FONT_HERSHEY_DUPLEX, 1.0, Scalar(100,100,0));
    
    bufP3f.add(p3f_cur);
    pair<int,Point3f> out = clusteringStylus(bufP3f);

    string msg = checkForStandingBy(10);
    if (msg.length() > 1){
        stateString = msg;
        iState = 4;
    } else{
        pair<int,std::string> sPair = setStatePair(out,bgr,zAxisAngle);
        iState = sPair.first;
        stateString = sPair.second;
    }
    long dt0 = chrono::duration_cast<chrono::milliseconds>(time_point - lastStylusSeen).count();
    if (dt0 > 5*1000){
        iState = 1;
        stateString = "";
    }

    stringstream ss;
    ss<<"Obj = "<<obj.current;
    cv::putText(bgr, ss.str(), Point(0,120), FONT_HERSHEY_DUPLEX, 1.0, Scalar(0,250,0));
    if (out.first == 4){
        drawTip(bgr,out.second);
    }

    int n = 50;
    if (stateStringSav.length() < 50) n = stateStringSav.length();
    cv::putText(bgr, stateStringSav.substr(0,n), Point(0,bgr.rows-25), FONT_HERSHEY_DUPLEX, 1.0, Scalar(0,0,250));
    cv::putText(bgr, currentTime, Point(0,bgr.rows-50), FONT_HERSHEY_DUPLEX, 1.0, Scalar(0,0,250));
}

string CamIO::checkForStandingBy(int nStep4Robust){
    /*
     standing by when stylus is invisible for a while
     */
    auto t =  chrono::system_clock::now();
    long dt = chrono::duration_cast<chrono::milliseconds>(t - lastStylusSeen).count();
    if (dt > 10*1000)
        isStandingBy = true;
    

    /*
     standing by when stylus is svisible, but not moved for a while
     */
    bool started_for_a_while = bufP2f.milli_sec_2_oldest_update() > 10*1000;
    bool recently_visible = bufP2f.milli_sec_2_newest_update() < 1000;
    bool not_moved_for_a_while = bufP2f.getMaxDist() < 10;
    if (started_for_a_while && recently_visible && not_moved_for_a_while) {
        isStandingBy = true;
//        cout<<"dtMax, dMax = "<<dtMax<<", "<<dMax<<endl;
    }

    
    // when stylus recently moved significantly, transition out of standing by
    bool bMoved = bufP2f.getMaxDist() > 100;
    bool bVisible = bufP2f.milli_sec_2_newest_update() < 2000;
    if (isStandingBy && bMoved && bVisible ){
        nStandby = 0;
        isStandingBy = false;
    }
    
    if (!isStandingBy) nStandby++;

    if (nStandby < nStep4Robust)  {// send over welcome msg several time for robustness
        string welcome = "Welcome to the Magic Map";
        stateStringSav = welcome;
        currentTime = getTimeString();
        return welcome;
    }
    else return "";
}

pair<int,std::string> CamIO::setStatePair(pair<int,Point3f> out,Mat& bgr,float zAxisAngle){
    pair<int,float> near = {-1, -1};
    if (out.first == 4){
        Mat rMat;
        Rodrigues(pBase->rvec, rMat);
        float tolAnIso = 0.33;
        near = obj.getNearest(out.second,rMat,tolAnIso);
        //drawTip(bgr,out.second);
    }
    obj.current = -1;
    pair<int,std::string> sPair;
    switch (out.first){
        case 0:
            sPair = {0,"0"};
            break;
        case 1:
            sPair = {1,"1"};
            break;
        case 4:
            if (near.second<tolClose && pStylus->nIdDetected>1){
                sPair = {4,"4"};
                obj.current = near.first;
                if (near.first >= obj.vRegion.size() )
                    sPair.second = obj.vZone[near.first - obj.vRegion.size()].name;
                else{
                    sPair.second = obj.vRegion[near.first].name + ",------,  " + obj.vRegion[near.first].description;
                    obj.vRegion[near.first].draw(bgr, pBase->rvec, pBase->tvec, camMatrix, distCoeffs, Scalar(0, 0, 255));
                    stateStringSav = sPair.second;
                    currentTime = getTimeString();
                 }
                cv::putText(bgr, stateString, Point(200,120), FONT_HERSHEY_DUPLEX, 1.0, Scalar(0,0,250));
            } else {
                sPair = {5,"5"};
                if (zAxisAngle > 30.0){
                    sPair.second = "Stylus Straight Upright";
//                    cout<<"sPair.second = Stylus up right, please!"<<endl;
//                    stateStringSav = sPair.second;
                }
            }
            break;
        default:
            cout<<"end of do1timeStep(), should not be here!!!"<<endl;
            sPair = {-1,"-1"};
    }
    return sPair;
}


pair<int,Point3f> CamIO::clusteringStylus(const RingBufP3f &bufP3f){
    int i1=0, i2=0;
    vector<Point3f> vP3f;
    for (int k=0; k<bufP3f.vDat.size(); k++){
        float x = bufP3f.vDat[k].x;
        if (x < -20.0) i2 += 1;
        else if (x < -10.0) i1 += 1;
        else vP3f.push_back(bufP3f.vDat[k]);
    }
    
    if (vP3f.size() >= i1+i2){
        Point3f p3f(0,0,0);
        for (int i=0; i<vP3f.size(); i++)
            p3f += vP3f[i];
        p3f *= 1.0/vP3f.size();
        return pair<int,Point3f>(4,p3f);                                // board and stylus
    } else if (i1> i2) return pair<int,Point3f>(0,Point3f());     // no board
    else  return pair<int,Point3f>(1,Point3f());           // no stylus
}

void CamIO::drawTip(Mat &bgr, const Point3f &tip){
    vector<Point3f> vP3f;
    vector<Point2f> vP2f;
    vP3f.push_back(tip);
    cv::projectPoints(vP3f, pBase->rvec, pBase->tvec, camMatrix, distCoeffs, vP2f);
    circle(bgr, vP2f[0], 5, Scalar(0, 255, 255),3.0); //    FONT_HERSHEY_SIMPLEX
    int x = vP2f[0].x + 50, y = vP2f[0].y;
    Scalar c = Scalar(0,255,0);
//    putText(bgr,Region::p3fToString(tip),Point(x,y),FONT_HERSHEY_DUPLEX,0.9,Scalar(0,0,255));
    putText(bgr,Region::float2s(tip.x),Point(x,y),FONT_HERSHEY_DUPLEX,0.9,c);
    putText(bgr,Region::float2s(tip.y),Point(x,y+20),FONT_HERSHEY_DUPLEX,0.9,c);
    putText(bgr,Region::float2s(tip.z),Point(x,y+40),FONT_HERSHEY_DUPLEX,0.9,c);
}


