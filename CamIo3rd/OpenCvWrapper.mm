//
//  OpenCvWrapper.m
//  CamIo3rd
//
//  Created by Huiying Shen on 2/15/19.
//  Copyright © 2019 Huiying Shen. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <opencv2/opencv.hpp>
#import <opencv2/imgcodecs/ios.h>
#import "OpenCvWrapper.h"
#import "markerArray.h"
#import "camio.h"

using namespace std;
using namespace cv;

@interface CamIoWrapper()
@property CamIO *camio;
@end


@implementation CamIoWrapper
- (id) init {
    if (self = [super init]) {
        aruco::PREDEFINED_DICTIONARY_NAME dictName = aruco::DICT_4X4_250;
        self.camio = new CamIO(dictName);
    } return self;
}

- (void) dealloc {
    delete self.camio;
}

- (void) clear {
    self.camio->clear();
}
- (bool) invert_color{
    return self.camio->invert_color();
}

-(void) resetFeatureBase{
    self.camio->resetFeatureBase();
}
-(void) setFeatureBaseImagePoints:(NSString*) dat
{
    self.camio->setFeatureBaseImagePoints(string([dat cStringUsingEncoding:NSUTF8StringEncoding]));
}
-(void) scaleModel:(float) scale
{
    self.camio->scaleModel(scale);
}

-(void) setStylusCube:(int) i
{
    self.camio->setStylusCube(i);
}

-(void) setFreezePose:(bool) freezePose
{
    self.camio->setFreezePose(freezePose);
}

- (int)newRegion:(NSString*) regionString
{
    return self.camio->newRegion(string([regionString cStringUsingEncoding:NSUTF8StringEncoding]));
}
- (int)newZone:(NSString*) zoneString
{
    return self.camio->newZone(string([zoneString cStringUsingEncoding:NSUTF8StringEncoding]));
}
- (bool)clearingYouAreHereBoundary {
    return self.camio->clearingYouAreHereBoundary();
}

-(int) getRegionByName:(NSString*) name{
    return self.camio->getRegionByName(string([name cStringUsingEncoding:NSUTF8StringEncoding]));
}

- (NSString*) getHighestP3f:(NSString*) name {
    string s = self.camio->getHighestP3f(string([name cStringUsingEncoding:NSUTF8StringEncoding]));
    return [NSString stringWithCString:s.c_str() encoding:NSUTF8StringEncoding];
}

-(bool) changeRegionName:(NSString*) namePlus
{
    return self.camio->changeRegionName(string([namePlus cStringUsingEncoding:NSUTF8StringEncoding]));
}

- (bool)setCameraCalib:(NSString*)calibStr
{
    return self.camio->setIntrinsic(string([calibStr cStringUsingEncoding:NSUTF8StringEncoding]));
}

- (bool)setCamMat3val:(NSString*)str3val
{
    return self.camio->setCamMat3val(string([str3val cStringUsingEncoding:NSUTF8StringEncoding]));
}


-(bool) setCurrentNameDescription:(NSString*)name with:(NSString*)description
{
    return self.camio->setCurrentNameDescription(string([name cStringUsingEncoding:NSUTF8StringEncoding]),
                                                 string([description cStringUsingEncoding:NSUTF8StringEncoding]));
}
-(void) setCurrent:(int) i
{
    self.camio->setCurrent(i);
}


- (void)loadModel:(NSString*)modelJson
{
    self.camio->loadModel(string([modelJson cStringUsingEncoding:NSUTF8StringEncoding]));
}


- (NSString*) getModelString
{
    return [NSString stringWithCString:self.camio->getObjJson().c_str() encoding:NSUTF8StringEncoding];
}

- (NSString*) getCurrentNameDescription
{
    return [NSString stringWithCString:self.camio->getCurrentNameDescription().c_str() encoding:NSUTF8StringEncoding];
}

- (bool)isCameraCovered{
    return self.camio->isCameraCovered();
}

-(UIImage *) procImage: (UIImage *) image {
    Mat rgba,bgr;
    UIImageToMat(image, rgba);
    cvtColor(rgba, bgr, CV_RGBA2BGR);
    
    self.camio->do1timeStep(bgr);
    
    cvtColor(bgr, rgba, CV_BGR2RGBA);
    return MatToUIImage(rgba);
}
- (NSString*) getState
{
    return [NSString stringWithCString:self.camio->getState().c_str() encoding:NSUTF8StringEncoding];
}

-(NSString*) getRegionNames
{
    return [NSString stringWithCString:self.camio->getRegionNames().c_str() encoding:NSUTF8StringEncoding];
}

- (int) getStateIdx
{
    return self.camio->iState;
}
- (int) getCurrentObjId
{
    return self.camio->getCurrentObjId();
}

-(bool) isNewRegion {
    return self.camio->isNewRegion();
}
-(bool) isExploring{
    return self.camio->isExploring();
}
-(bool) locationCaptured{
    return self.camio->locationCaptured();
}


-(void) setNewRegion {
    self.camio->setNewRegion();
}
-(void) setAdd2Region {
    self.camio->setAdd2Region();
}
-(void) setSelectRegion {
    self.camio->setSelectRegion();
}
-(void) deleteCurrentRegion{
    self.camio->deleteCurrentRegion();
}
-(bool) isActionDone{
    return self.camio->isActionDone();
}
-(bool) isStylusVisible{
    return self.camio->isStylusVisible();
}
-(int) getCurrentRegion{
    return self.camio->getCurrentRegion();
}
@end
