/*
 * camIoObj.h
 *
 *  Created on: Mar 13, 2018
 *      Author: huiying
 */

#ifndef CAMIOOBJ_H_
#define CAMIOOBJ_H_

#include <chrono>
#include <thread>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "util.h"
#include "markerArray.h"
#include "dowelStylus.h"
#include "anchorBase.h"

using namespace std;
using namespace cv;



//struct P3fProjector {
//	Point3f p3f;
//	P3fProjector(const Point3f &p3f) :p3f(p3f) {}
//	Point2f project(Mat &bgr, const Mat &rvec, const Mat &tvec, const Mat &camMatrix, const Mat &distCoeffs);
//};
struct P3fGroup{
    string name = "name";
    string description = "";
    vector<cv::Point3f> vP3f;
    void addPoint(const Point3f &p3f) { vP3f.push_back(p3f); }
};

struct Region: P3fGroup{
    
    std::pair<int,float> nearest;
    float dist2Stylus;
    
    Region():dist2Stylus(999),nearest({-1,999.0}){}
	Region(const Point3f &p3f):dist2Stylus(999),nearest({-1,999.0}){vP3f.push_back(p3f);}
    
    void scale(float scale){
        for (int i=0; i<vP3f.size(); i++)
            vP3f[i] *= scale;
    }
    int getHighestIndex(){
        int indx = -1;
        float zMax = -999.0;
        for (int i=0; i<vP3f.size(); i++){
            if (vP3f[i].z > zMax){
                indx = i;
                zMax = vP3f[i].z;
            }
        }
        return indx;
    }
    
    Point3f getHighestRobust(float tol = 0.001){
        int indx = getHighestIndex();
        if (indx == -1) return Point3f(-999,-999,-999);
        Point3f p = vP3f[indx];
        Point3f pMean(0,0,0);
        int cnt = 0;
        for (int i=0; i<vP3f.size(); i++){
            if (vP3f[i].z + tol >= p.z){
                pMean += vP3f[i];
                cnt++;
            }
        }
        cout<<"getHighestRobust, cnt = "<<cnt<<endl;
        pMean *= 1.0/cnt;
        return pMean;
    }

    static float dist2(const Point3f &p1, const Point3f &p2){
        Point3f p(p1);
        p -= p2;
        return p.dot(p);
    }
    
    float getDistSquare(const Point3f &p) const {
        float d2Min = 9999.0;
        for (int i=0; i<vP3f.size(); i++){
            float d2 = dist2(vP3f[i],p);
            if (d2Min > d2)
                d2Min = d2;
        }
        return d2Min;
    }
    
    
    void setNameDescription(const string &name, const string &description){
        this->name = "" + name;
        this->description = ""+description;
    }
    
    string getNameDescription(){
        return name + "------" + description;
    }

	static string f2s(float f, int precision = 3) {
		stringstream ss;
		ss << setprecision(precision) << f;
		return ss.str();
	}
	static string nameAndValToJson(const string &name, float val) {
		return '\"' + name + "\": " + f2s(val);
	}
	static string nameAndValToJson(const string &name, string val) {
		return '\"' + name + "\": " + '\"' + val+ '\"';
	}
	static string toJson(const Point3f &f) {
		return  nameAndValToJson("x", f.x) + ", "
			  + nameAndValToJson("y", f.y) + ", "
			  + nameAndValToJson("z", f.z);
	}
	 
	static string toJson(const vector<Point3f> &vP3f) {
		string o;
        for (int i=0; i<vP3f.size(); i++){
            const Point3f &p = vP3f[i];
            o += '{' + toJson(p) +'}';
            if (i < vP3f.size()-1)
                o += ',';
            o += '\n';
        }
		return o;
	}

	string toJson() {
		string o("{");
		o += nameAndValToJson("name", name) + ",\n";
		o += nameAndValToJson("description", description) + ",\n";
		o += "\"vPoint3f\": [\n" + toJson(vP3f) + "]\n}\n";
		return o;
	}

	void setNearest(const Point3f &p3f, const Mat &rMat, float zScale);

    static string float2s(float x, int precision=3) {
        std::stringstream s;
        s << std::setprecision(precision) << x;
        return s.str();
    }
	static string p3fToString(const Point3f &p3f, int precision=3) {
		std::stringstream s;
		s << std::setprecision(precision) << p3f.x << ", ";
		s << std::setprecision(precision) << p3f.y << ", ";
		s << std::setprecision(precision) << p3f.z;
		return s.str();
	}

	void draw(Mat &bgr, const Vec3f &rvec, const Vec3f &tvec, const Mat &camMatrix, const Mat &distCoeffs,
		Scalar color = Scalar(255, 255, 0), int radius = 2, int thickness = 1) const;

    static float dist(const Point3f & p1, const Point3f &p2, Mat rMat = (Mat_<float>(3,3) << 1,0,0,0,1,0,0,0,1), float zScale = 1.0);

	bool tryInit(string &s) {
		return trySetOneString("name", name, s)
			&& trySetOneString("description", description, s)
			&& getAllPoint3f(s) > 0;
	}

	static bool tryInitPoint3f(const string &s, Point3f &p) {
		vector<string> vs = parseVar(s);
		return trySetP3fXyz(vs, p);
	}

	static bool trySetP3fXyz(const vector<string> &vs, Point3f &p);

	template<typename T>
	static bool trySetOneFloat(const string &name, T &val, const string &sIn) {
		long pos = sIn.find('\"' + name + '\"');
		if (pos == -1) return false;
		pos = sIn.find_first_of(':');
		if (pos == -1) return false;
		stringstream ss(sIn.substr(pos + 1));
		ss >> val;
		return true;
	}

	static vector<string> parseVar(string s);
	static bool trySetOneString(const string &name, string &val, string &sIn);
	static string getOneVecPoint3f(string &s);
	static string getOnePoint3f(string &s);
	int getAllPoint3f(string &s);
};

struct Zone: P3fGroup{
    vector<cv::Point2f> vP2f;
    float xMin,xMax,yMin,yMax,zMin,zMax;
    
    Zone(){}
    void setData2(){
        vP2f.clear();
        xMin = xMax = vP3f[0].x;
        yMin = yMax = vP3f[0].y;
        zMin = zMax = vP3f[0].z;
         
        for (int i=1; i<vP3f.size(); i++){
            const Point3f &p3f = vP3f[i];
            xMin = fmin(xMin,p3f.x); xMax = fmax(xMax,p3f.x);
            yMin = fmin(yMin,p3f.y); yMax = fmax(yMax,p3f.y);
            zMin = fmin(zMin,p3f.z); zMax = fmax(zMax,p3f.z);
        }
        float zMid = 0.5*(zMin + zMax);
        for (int i=0; i<vP3f.size(); i++){
            const Point3f &p3f = vP3f[i];
            if (p3f.z < zMid)
                vP2f.push_back(Point2f(p3f.x,p3f.y));
        }
        float dz = zMax - zMin;
        zMax += dz;         //make the ceiling a lot higher
        zMin += dz/2.0;     // lift the bottom a little to avoid collision with objects

//        Point2f p2f(0,0);
//        for (int i=0; i<vP2f.size(); i++)
//            p2f += vP2f[i];
//        p2f *= 1.0/vP2f.size();
//        bool b = isInside(Point3f(p2f.x,p2f.y,zMid));
//        cout<<"Zone::setData(), name = "<<name<<", vP3f.size() = "<<vP3f.size()<<endl;
    }
    
    
    static int isAbove(const Point2f &p, const Point2f &p1, const Point2f &p2){
        if (p.x < fmin(p2.x,p1.x) || p.x > fmax(p2.x,p1.x)) return 0;
        if (fabs(p2.x - p1.x) <0.000001) return 0;
        float dy = (p2.y - p1.y)/(p2.x - p1.x)*(p.x - p1.x);
        if (p.y < p1.y + dy) return 0;
        return 1;
    }
    
    bool isInside(const Point3f &p){
        if (p.x<xMin || p.x>xMax || p.y<yMin || p.y>yMax || p.z<zMin || p.z>zMax)
            return false;
        int cnt = 0;
        Point2f p2f(p.x,p.y);
        const Point2f &p1 = vP2f[vP2f.size() - 1];
        for (int i=0; i<vP2f.size(); i++){
            int i1 = i-1;
            if (i==0) i1 = vP2f.size() - 1;
            const Point2f &p1 = vP2f[i1];
            const Point2f &p2 = vP2f[i];
            cnt += isAbove(p2f,p1,p2);
        }
        return cnt%2 == 1;
    }
    
    void draw(Mat &bgr, const Vec3f &rvec, const Vec3f &tvec, const Mat &camMatrix, const Mat &distCoeffs,
        Scalar color = Scalar(255, 0, 255), int radius = 3, int thickness = 2) const;
    
};


struct SmoothedDetection{
    
    struct Detection{
        int val;
        chrono::time_point<chrono::system_clock> time_point = chrono::system_clock::now();
        Detection(int val=-1):val(val){
            time_point = chrono::system_clock::now();
        }
        long timeDiffMilli(const Detection &other){
            return chrono::duration_cast<chrono::milliseconds>(time_point - other.time_point).count();
        }
    };
    
    vector<Detection> vDet;
    int cur;
    int iDet = -1;
    SmoothedDetection(int sz=60):cur(-1){vDet.resize(sz);}
    void addNew(int val){
        cur = (cur+1)%vDet.size();
        vDet[cur] = Detection(val);
    }
    
    void update(int newVal){
        addNew(newVal);
        iDet = getMostCommon();
        
//        if (h.get_fMostComm() < 0.33)   // most common state is less than half of the total
//            iDet = -1;
    }
    
    HistoT<int> h;
    int getMostCommon(long dtMilli=300 /* millisec */){
        h.reset();
        for (unsigned int i=0; i<vDet.size(); i++){
//            long milli = vDet[cur].timeDiffMilli(vDet[i]);
            //cout<<"milli = "<<milli<<endl;
            if (vDet[cur].timeDiffMilli(vDet[i]) < dtMilli)
                h.add(vDet[i].val);
            //cout<<"millisec = "<<vDet[cur].timeDiffMilli(vDet[i])<<endl;
        }
        return h.getMostCommon();
    }
    int getCntMax(){return h.getCntMax();}
    float get_fMostComm(){return h.get_fMostComm();}
    
    static void test0(){
        SmoothedDetection smDet(12);
        int vv[20] = {1,2,2,4,5, 5,4,3,3,4, 4,3,1,3,5, 4,3,4};
        for (int i=0; i<15; i++){
            smDet.addNew(vv[i]);
            std::this_thread::sleep_for (std::chrono::milliseconds(9));
        }
        cout<<smDet.getMostCommon()<<endl;
        cout<<smDet.getCntMax()<<endl;
        cout<<smDet.get_fMostComm()<<endl;
    }
    
};

struct Landmark{
    string name;
    Point3f p3f;
    Landmark(string name="", Point3f p3f=Point3f(0,0,0)):name(name),p3f(p3f){}
};

struct Obj {
	string name;
	string description;
	vector<Region> vRegion;
    vector<Zone> vZone;
    SmoothedDetection zoneOrRegion;
    
    vector<Landmark> vLandmark;

//    int iNearest = -1;
	int current = -1;
    bool isCurrentClose = false;
    Point3f nearestErrP3f;

    Obj():name("name_"),description("des_"){
        vLandmark.push_back(Landmark("Kinder Bells", Point3f(0.0639,0.112,0.0324)));
        vLandmark.push_back(Landmark("You are here at the Magic Map", Point3f(0.711,0.219,0.0164)));
        vLandmark.push_back(Landmark("Umbrella Pole", Point3f(0.418,0.372,0.0322)));
        vLandmark.push_back(Landmark("Exercise Bike",Point3f(0.22,0.498,0.0162)));
    }
	void clear() {
        vRegion.clear();
        vZone.clear();
        vLandmark.clear();
        current = -1;
        
    }

    void scale(float scale){
        for (int i=0; i<vRegion.size(); i++){
            vRegion[i].scale(scale);
        }
    }
	bool add2Region(const Point3f &p3f) {
		if (current == -1) return false;
		vRegion[current].addPoint(p3f);
		return true;
	}
    
    Region &addEmptyRegion(){
        vRegion.push_back(Region());
        return *vRegion.rbegin();
    }
    
    string getRegionNames(){
        cout<<"getRegionNames() \n";
        string out = "";
        for (int i=0; i<vRegion.size(); i++){
            string nm = vRegion[i].name;
            out += nm + '\n';
        }
        for (int i=0; i<vZone.size(); i++){
            string nm = vZone[i].name;
            out += nm + '\n';
        }
        return out;
    }
    
    void checkReginNames(){
        for (int i=0; i<vRegion.size(); i++){
            string name = vRegion[i].name;
            int pos = name.find('?');
            if (pos==-1){
                stringstream ss;
                ss<<name<<"?hotspot"<<i;
                vRegion[i].name = ss.str();
            }else{
                int pos2 = name.find("?hotspot");
                if (pos2 != -1){
                    stringstream ss;
                    ss<<name.substr(0,pos2)<<"?hotspot"<<i;
                    vRegion[i].name = ss.str();
                }
            }
        }
    }
    
    void setCurrent(int i){
        if (i<0 || i>=vRegion.size()) return;
        current = i;
    }

	bool deleteRegion(const Point3f &p3f, float tol = 10.0);
    
    bool changeRegionName(const string &namePlus) {
        std::vector<std::string> lines = split(namePlus, '\n');
        if (lines.size() == 1) return false;
        int i = findRegionByName(lines[0]);
        if (i == -1) return false;
        vRegion[i].name = lines[1];
        if (lines.size() >= 3)
            vRegion[i].description = lines[2];
        return true;
    }
    
    bool myMatch(const string &line, const  string &name) const {
        std::vector<std::string> lines = split(line, '-');
        int k=-1;
        for (string l:lines){
            k = name.find(l,k+1);
            if (k==-1) return false;
        }
        return true;
    }
    
    int findRegionByName(const string &name)  const {
        for (int i=0; i<vRegion.size(); i++)
        if (name == vRegion[i].name) return i;
//            if (myMatch(name,vRegion[i].name)) return i;
        return -1;
    }
    
    string getHighestP3f(int iRegion){
        if (iRegion<0 || iRegion>vRegion.size()) return "";
        Point3d p3f = vRegion[iRegion].getHighestRobust();
        return p3fToString(p3f);
    }
    
    int clearingBoundary(int idFrom, int idCloseTo, float distTol){
        float d2Tol = distTol*distTol;
        Region &rFrom = vRegion[idFrom];
        Region &rCloseTo = vRegion[idCloseTo];
        int sz0 =  rFrom.vP3f.size();
        for (int i=0; i<rFrom.vP3f.size(); ){
            if (rCloseTo.getDistSquare(rFrom.vP3f[i]) < d2Tol)
                rFrom.vP3f.erase(rFrom.vP3f.begin() + i);
            else
                i++;
        }
        return sz0 - rFrom.vP3f.size();
    }
    
    static void getP3f(Point3f &p3f, const string &line,bool switchYZ = false){
        std::vector<std::string> items = split(line, ' ');
//        for (int i=0; i<items.size(); i++)
//            cout<<i<<", "<<items[i]<<endl;
        
        stringstream ss1(items[1]);
        stringstream ss2(items[2]);
        stringstream ss3(items[3]);
        ss1 >> p3f.x;
        if (switchYZ){
            ss2 >> p3f.z;
            ss3 >> p3f.y;
            p3f.y = -p3f.y;
        } else {
            ss2 >> p3f.y;
            ss3 >> p3f.z;
        }
        p3f *= 0.001;
    }
    
    static void setP3fGroupData(P3fGroup &r,const string &regionString,bool switchYZ){
        std::vector<std::string> lines = split(regionString, '\n');
        if(lines[0].find('\t') != std::string::npos){
            std::vector<std::string> l2 = split(lines[0], '\t');
            r.name = l2[0];
            r.description = l2[1];
        } else
            r.name = lines[0];
        int pos = r.name.find_last_of(".");
        if (pos>0){
            r.name = r.name.substr(0,pos);
        }
        if (r.name.substr(0,5) == "ZONE_"){
            cout<<r.name<<endl;
            r.name = "ZONE_" + r.name.substr(8);
        }

        for (int i=1; i<lines.size(); i++){
            Point3f p3f;
            string line = ltrim(lines[i]);
            if (line.size()<20) continue;
            getP3f(p3f,line,switchYZ);
//            cout<<line<<endl;
//            cout<<p3f<<endl;
            r.addPoint(p3f);
        }
    }
    
    int getIdByName(const string &name){
        for (int i=0; i<vRegion.size(); i++)
            if (name==vRegion[i].name) return i;
        return -1;
    }
    
    int newRegion(const string &regionString, bool switchYZ = false){
        Region &r = addEmptyRegion();
        setP3fGroupData(r,regionString,switchYZ);
        return r.vP3f.size();
    }
    
    int newZone(const string &regionString, bool switchYZ = false){
        Zone z;
        setP3fGroupData(z,regionString,switchYZ);
        z.setData2();
        vZone.push_back(z);
        return z.vP2f.size();
    }

    void newRegionWithXyz(const string &xyz){
        string tmp(xyz);
        replaceAll(tmp,"_"," ");
        Point3f p3f;
        stringstream ss(tmp);
        ss>>p3f.x;
        ss>>p3f.y;
        ss>>p3f.z;
        Region &r = addNewRegion(p3f);
        
        stringstream ss1;
        ss1<<xyz<<"?hotspot"<<current;
        string name = ss1.str();
        r.name = ss1.str();
        r.description = "recording";
    }
    
	Region &addNewRegion(const Point3f &p3f) {
		vRegion.push_back(p3f);
		current = (int)vRegion.size() - 1;
        return vRegion[current];
	}

	bool tryAdd2Region(const Point3f &p3f);
    
    bool setCurrentNameDescription(const string &name, const string &description){
        if (current==-1) return false;
        vRegion[current].setNameDescription(name,description);
        return true;
    }
    
    string getCurrentNameDescription(){
        if (current==-1) return "none";
        return vRegion[current].getNameDescription();
    }
    
    void deleteCurrentRegion(){
        if (current==-1) return;
        vRegion.erase(vRegion.begin()+current);
        current = vRegion.size()-1;
    }

	void draw(Mat &bgr, const Mat &rvec, const Mat &tvec, const Mat &camMatrix, const Mat &distCoeffs) const;
    

    
    pair<int,float> getNearest(const Point3d &p3f, const Mat &rMat, float zScale);
    
    pair<int,float> getNearest_sav(const Point3d &p3f, const Mat &rMat, float zScale);

    float getDistMin(){
        if (current == -1) return 999;
        return vRegion[current].nearest.second;
    }
	bool tryInit(string &s) {
        clear();
		return Region::trySetOneString("name", name, s)
			&& Region::trySetOneString("description", description, s)
			&& setAllRegions(s) > 0;
	}

	static string get_vRegion(string &s);
	int setAllRegions(string &s);

	string toJson() {
		string o = "{\n";
		o += Region::nameAndValToJson("name", name) + ",\n";
		o += Region::nameAndValToJson("description", description) + ",\n";
		o += "\"vRegion\": [\n";
        for (int i=0; i<vRegion.size(); i++){
            Region &r = vRegion[i];
            o += r.toJson();
            if (i<vRegion.size()-1)
                o += ',';
            o += '\n';
        }
		return o + "]\n}";
	}
};

#endif /* CAMIOOBJ_H_ */
