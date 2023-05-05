/*
 * camIoObj.cpp
 *
 *  Created on: Mar 13, 2018
 *      Author: huiying
 */
#include <string>
#include "camIoObj.h"
#include "util.h"

//Point2f P3fProjector::project(Mat &bgr, const Mat &rvec, const Mat &tvec, const Mat &camMatrix, const Mat &distCoeffs) {
//	vector<Point3f> vP3f;
//	vector<Point2f> vP2f;
//	vP3f.push_back(p3f);
//	cv::projectPoints(vP3f, rvec, tvec, camMatrix, distCoeffs, vP2f);
//	return vP2f[0];
//}

bool Region::trySetP3fXyz(const vector<string> &vs, Point3f &p) {
		if (vs.size() != 3) return false;
	int cnt = 0;
	for (string s : vs) {
		if (trySetOneFloat("x", p.x, s)) cnt++;
		if (trySetOneFloat("y", p.y, s)) cnt++;
		if (trySetOneFloat("z", p.z, s)) cnt++;
	}
	return cnt == 3;
}

vector<string> Region::parseVar(string s) {
	vector<string> out;
	int pos1 = 0;
	while (true) {
		int pos2 = s.find_first_of(',', pos1);
		if (pos2 == -1) {
			out.push_back(s.substr(pos1));
			break;
		}
		out.push_back(s.substr(pos1, pos2 - pos1));
		pos1 = pos2 + 1;
	}
	return out;
}

bool Region::trySetOneString(const string &name, string &val, string &sIn) {
	int pos1 = sIn.find('\"' + name + '\"');
	if (pos1 == -1) return false;
	int pos2 = sIn.find_first_of(':', pos1 + 1);
	if (pos2 == -1) return false;
	int pos3 = sIn.find_first_of('\"', pos2 + 1);
	if (pos3 == -1) return false;
	int pos4 = sIn.find_first_of('\"', pos3 + 1);
	if (pos4 == -1) return false;
	val = "" + sIn.substr(pos3 + 1, pos4 - pos3 - 1);
	sIn = sIn.substr(pos2, pos3 - pos2 + 1) + sIn.substr(pos4);
	return true;
}

string Region::getOneVecPoint3f(string &s) {
	int pos1 = s.find_first_of('[');
	if (pos1 == -1) return "";
	int pos2 = s.find_first_of(']', pos1);
	if (pos2 == -1) return "";
	string out = s.substr(pos1 + 1, pos2 - pos1 - 1);
	s = s.substr(0, pos1) + s.substr(pos2);
	return out;
}

string Region::getOnePoint3f(string &s) {
	int pos1 = s.find_first_of('{');
	if (pos1 == -1) return "";
	int pos2 = s.find_first_of('}', pos1);
	if (pos2 == -1) return "";
	string sout = s.substr(pos1 + 1, pos2 - pos1 - 1);
	s = s.substr(pos2 + 1);
	return sout;
}

int Region::getAllPoint3f(string &s) {
	string vP3fStr = getOneVecPoint3f(s);
	Point3f p;
	while (true) {
		string s1 = getOnePoint3f(vP3fStr);
		if (s1.size() == 0)
			break;
		if (tryInitPoint3f(s1, p))
			vP3f.push_back(p);
	}

	return vP3f.size();
}

float Region::dist(const Point3f & p1, const Point3f &p2, Mat rMat, float zScale){
    Matx31f out = Matx33f(rMat)*Matx31f(Vec3f(p1.x-p2.x,p1.y-p2.y,p1.z-p2.z));
    return sqrt(out(0,0)*out(0,0) + out(1,0)*out(1,0) + out(2,0)*out(2,0)*zScale*zScale);
}

void Region::setNearest(const Point3f &p3f, const Mat &rMat, float zScale) {
	float dMin = 9999.0;
    int iNearest = -1;
    for (int i=0; i<vP3f.size(); i++){
        float d = Region::dist(p3f, vP3f[i],rMat,zScale);
        //float d = sqrt((p3f.x-vP3f[i].x)*(p3f.x-vP3f[i].x) + (p3f.y-vP3f[i].y)*(p3f.y-vP3f[i].y));
        if (dMin > d){
            dMin = d;
            iNearest = i;
//            nearestErrP3f = p3f - vP3f[i];
        }
    }
    nearest = {iNearest,dMin};
}



void Region::draw(Mat &bgr, const Vec3f &rvec, const Vec3f &tvec, const Mat &camMatrix, 
	const Mat &distCoeffs, Scalar color, int radius, int thickness) const {
    vector<Point2f> vP2f;
    cv::projectPoints(vP3f, rvec, tvec, camMatrix, distCoeffs, vP2f);
    for (int i = 0; i < vP2f.size(); i++) {
        cv::Point p(vP2f[i]);
            cv::circle(bgr, p, radius, color,thickness);
     }
}

void Zone::draw(Mat &bgr, const Vec3f &rvec, const Vec3f &tvec, const Mat &camMatrix,
    const Mat &distCoeffs, Scalar color, int radius, int thickness) const {
    vector<Point2f> vtmp;
    cv::projectPoints(vP3f, rvec, tvec, camMatrix, distCoeffs, vtmp);
    for (int i = 0; i < vtmp.size(); i++) {
        cv::Point p(vtmp[i]);
        cv::circle(bgr, p, radius, color,thickness);
    }
}


bool Obj::tryAdd2Region(const Point3f &p3f) {
	if (current == -1) {
		addNewRegion(p3f);
		return true;
	}
	return add2Region(p3f);
}

bool Obj::deleteRegion(const Point3f &p3f, float tol) {
    return false;
//	current = select(p3f, tol);
//	if (current == -1) return false;
//	vRegion.erase(vRegion.begin() + current);
//	return true;
}

void Obj::draw(Mat &bgr, const Mat &rvec, const Mat &tvec, const Mat &camMatrix, const Mat &distCoeffs) const {
	for (Region r : vRegion)
		r.draw(bgr, rvec, tvec, camMatrix, distCoeffs);
    if (current > -1 && current < vRegion.size()){
        vRegion[current].draw(bgr, rvec, tvec, camMatrix, distCoeffs, Scalar(0, 0, 255));
        if (isCurrentClose)
            vRegion[current].draw(bgr, rvec, tvec, camMatrix, distCoeffs, Scalar(0, 0, 200));
    }
    
    for (Zone z : vZone)
        z.draw(bgr, rvec, tvec, camMatrix, distCoeffs);
}

string Obj::get_vRegion(string &s) {
	long pos1 = s.find_first_of('[');
	long pos2 = s.find_last_of(']');
	if (pos1 == -1 || pos2 < pos1) return "";
	return s.substr(pos1 + 1, pos2 - pos1 - 1);
}

int Obj::setAllRegions(string &s) {
	string s1 = get_vRegion(s);
	while (true) {
		Region r;
		if (r.tryInit(s1))
			vRegion.push_back(r);
		else
			break;
	}
    current = vRegion.size() - 1;
	return (int) vRegion.size();
}
pair<int,float> Obj::getNearest(const Point3d &p3f, const Mat &rMat, float zScale){
    for (int i=0; i<vZone.size(); i++){
        if (vZone[i].isInside(p3f)){
            return {vRegion.size() + i, 0};
        }
    }
    pair<int,float> out = {-1, -1};
    if (vRegion.size() > 0){
        float dMin = 999.0;
        for (int i=0; i< vRegion.size(); i++){
            Region &r = vRegion[i];
            r.setNearest(p3f,rMat,zScale);
            if (dMin > r.nearest.second){
                dMin = r.nearest.second;
                out = {i,dMin};
            }
        }
    }
    return out;
}
pair<int,float> Obj::getNearest_sav(const Point3d &p3f, const Mat &rMat, float zScale){
    pair<int,float> out = {-1, -1};
    for (int i=0; i<vZone.size(); i++){
        if (vZone[i].isInside(p3f)){
//                out = {vRegion.size() + i, 0};
            zoneOrRegion.addNew(vRegion.size() + i);
            return {zoneOrRegion.getMostCommon(), 0};
        }
    }
    
    if (vRegion.size() > 0){
        float dMin = 999.0;
        for (int i=0; i< vRegion.size(); i++){
            Region &r = vRegion[i];
            r.setNearest(p3f,rMat,zScale);
            if (dMin > r.nearest.second){
                dMin = r.nearest.second;
                out = {i,dMin};
            }
        }
    }
    zoneOrRegion.addNew(out.first);
    out.first = zoneOrRegion.getMostCommon();
    return out;
}

