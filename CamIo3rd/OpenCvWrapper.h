//
//  OpenCvWrapper.h
//  CamIo3rd
//
//  Created by Huiying Shen on 2/15/19.
//  Copyright © 2019 Huiying Shen. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface CamIoWrapper : NSObject
- (instancetype)init;

-(void) clear;
-(bool) invert_color;
-(void) resetFeatureBase;
-(void) setFeatureBaseImagePoints:(NSString*) dat;
-(void) scaleModel:(float) scale;
-(void) setStylusCube:(int) i;
-(void) setFreezePose:(bool) freezePose;

- (int)newRegion:(NSString*) regionString;
- (int)newZone:(NSString*) zoneString;
- (bool)clearingYouAreHereBoundary;

- (int) getRegionByName:(NSString*) name;
- (NSString*) getHighestP3f:(NSString*) name;
-(bool) changeRegionName:(NSString*) namePlus;
//- (void)setNoBoardStr: (NSString*) s;
//- (void)setBoardOnlyStr:(NSString*)s;
//- (void)setExoploringStr:(NSString*)s;
//- (void)setPreRecordingStr:(NSString*)s;
//- (void)setStartRecordingStr:(NSString*)s;

- (bool)setCameraCalib: (NSString*) calibStr;
- (bool)setCamMat3val: (NSString*) str3val;

-(bool) tryAdd4Calib: (UIImage *) image;
-(void) initCalib;
- (NSString*) doCalib;
- (bool)isCameraCovered;
-(UIImage *) procImage: (UIImage *) image;
-(NSString*) getState;
- (int) getStateIdx;
- (int) getCurrentObjId;

-(bool) isNewRegion;
-(bool) isExploring;
-(bool) locationCaptured;

-(void) setNewRegion;
-(void) setAdd2Region;
-(void) setSelectRegion;
-(void) deleteCurrentRegion;
-(bool) isActionDone;
-(bool) isStylusVisible;
-(int) getCurrentRegion;
-(void) setCurrent:(int) i;
-(bool) setCurrentNameDescription:(NSString*)name with:(NSString*)description;
- (NSString*) getCurrentNameDescription;
//-(void) setRectBase:(int) id1st markerLengthInMeter: (float)l rWidth2markerLength: (float)nx rHeight2markerLength: (float)ny ;
//-(NSString*) getRectBaseString;
-(void) loadModel:(NSString*)modelJson;
-(NSString*) getModelString;
-(NSString*) getRegionNames;
@end

NS_ASSUME_NONNULL_END
