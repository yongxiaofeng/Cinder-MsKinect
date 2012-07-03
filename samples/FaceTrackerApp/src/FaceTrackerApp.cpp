/*
* 
* Copyright (c) 2012, Ban the Rewind
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Ban the Rewind nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

// Includes
#include "cinder/app/AppBasic.h"
#include "cinder/Camera.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"
#include "FaceTracker.h"
#include "Kinect.h"

/* 
* This application demonstrates how to the Face Tracker SDK with the 
* Kinect SDK to track faces with a Kinect.
*/ 
class FaceTrackerApp : public ci::app::AppBasic 
{

public:

	// Cinder callbacks
	void draw();
	void keyDown( ci::app::KeyEvent event );
	void prepareSettings( ci::app::AppBasic::Settings *settings );
	void setup();
	void shutdown();
	void update();

private:

	// Kinect
	KinectSdk::KinectRef				mKinect;
	std::vector<KinectSdk::Skeleton>	mSkeletons;

	// Face tracker
	FaceTrackerRef						mFaceTracker;

	// Camera
	ci::CameraPersp						mCamera;

	// Save screenshot
	void								screenShot();

};

// Imports
using namespace ci;
using namespace ci::app;
using namespace KinectSdk;
using namespace std;

// Render
void FaceTrackerApp::draw()
{

	// Clear window
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf::gray( 0.1f ) );

	// We're capturing
	if ( mKinect->isCapturing() ) {

		// Set up camera for 3D
		gl::setMatrices( mCamera );

		// Iterate through skeletons
		uint32_t i = 0;
		for ( vector<Skeleton>::const_iterator skeletonIt = mSkeletons.cbegin(); skeletonIt != mSkeletons.cend(); ++skeletonIt, i++ ) {

			// Valid skeletons have all joints
			if ( skeletonIt->size() == JointName::NUI_SKELETON_POSITION_COUNT ) {

				// Set color
				Colorf color = mKinect->getUserColor( i );

				// Iterate through joints
				for ( Skeleton::const_iterator boneIt = skeletonIt->cbegin(); boneIt != skeletonIt->cend(); ++boneIt ) {
					
					// Get position and rotation
					Vec3f position		= boneIt->second.getPosition();
					Matrix44f transform	= boneIt->second.getAbsoluteRotationMatrix();
					Vec3f direction		= transform.transformPoint( position ).normalized();
					direction			*= 0.05f;

					// Draw bone
					glLineWidth( 2.0f );
					glBegin( GL_LINES );
					Vec3f destination = skeletonIt->at( boneIt->second.getStartJoint() ).getPosition();
					gl::vertex( position );
					gl::vertex( destination );
					glEnd();

					// Draw joint
					gl::color( color );
					gl::drawSphere( position, 0.025f, 16 );
					
					// Draw joint orientation
					glLineWidth( 0.5f );
					gl::color( ColorAf::white() );
					gl::drawVector( position, position + direction, 0.05f, 0.01f );

				}

			}

		}

	}

}

// Handles key press
void FaceTrackerApp::keyDown( KeyEvent event )
{

	// Quit, toggle fullscreen
	switch ( event.getCode() ) {
	case KeyEvent::KEY_ESCAPE:
		quit();
		break;
	case KeyEvent::KEY_f:
		setFullScreen( !isFullScreen() );
		break;
	case KeyEvent::KEY_SPACE:
		screenShot();
		break;
	}

}

// Prepare window
void FaceTrackerApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 800, 600 );
	settings->setFrameRate( 60.0f );
}

// Take screen shot
void FaceTrackerApp::screenShot()
{
	writeImage( getAppPath() / fs::path( "frame" + toString( getElapsedFrames() ) + ".png" ), copyWindowSurface() );
}

// Set up
void FaceTrackerApp::setup()
{

	// Start Kinect
	mKinect = Kinect::create();
	DeviceOptions deviceOptions;
	mKinect->start( deviceOptions );
	mKinect->removeBackground();
	mKinect->setFlipped( true );

	// Set up face tracker
	mFaceTracker = FaceTracker::create();
	mFaceTracker->setup( deviceOptions );

	// Set the skeleton smoothing to remove jitters. Better smoothing means
	// less jitters, but a slower response time.nclude
	mKinect->setTransform( Kinect::TRANSFORM_SMOOTH );

	// Set up camera
	mCamera.lookAt( Vec3f( 0.0f, 0.0f, 2.0f ), Vec3f::zero() );
	mCamera.setPerspective( 45.0f, getWindowAspectRatio(), 0.01f, 1000.0f );

}

// Called on exit
void FaceTrackerApp::shutdown()
{

	// Stop input
	mKinect->stop();

}

// Runs update logic
void FaceTrackerApp::update()
{

	// Kinect is capturing
	if ( mKinect->isCapturing() ) {
	
		// Acquire skeletons
		if ( mKinect->checkNewSkeletons() ) {
			mSkeletons = mKinect->getSkeletons();
		}

	} else {

		// If Kinect initialization failed, try again every 90 frames
		if ( getElapsedFrames() % 90 == 0 ) {
			mKinect->start();
		}

	}

}

// Run application
CINDER_APP_BASIC( FaceTrackerApp, RendererGl )